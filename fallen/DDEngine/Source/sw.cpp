//
// Sofware renderer hacked in!
//

#include "ddlib.h"
#include "sw.h"
#include "poly.h"
#include "texture.h"
#include "elev.h"
#include "game.h"


#ifndef TARGET_DC


#define THIS_IS_INCLUDED_FROM_SW


//
// Tom's swizzling!
//

static char swizzle_table[16] = {
	0x00, 0x01, 0x04, 0x05,
	0x10, 0x11, 0x14, 0x15,
	0x40, 0x41, 0x44, 0x45,
	0x50, 0x51, 0x54, 0x55 };


//
// In texture.cpp...
//

extern D3DTexture TEXTURE_texture[];


//
// Our back-buffer.
//

ULONG *SW_buffer;
#ifdef TARGET_DC
// Spoof that sucker!
ULONG  SW_buffer_memory[1];
#else
ULONG  SW_buffer_memory[SW_MAX_WIDTH * SW_MAX_HEIGHT + 2];	// + 2 to ensure quadword alignment.
#endif
SLONG  SW_buffer_width;
SLONG  SW_buffer_height;
SLONG  SW_buffer_pitch;	// In ULONGs

#define SW_PIXEL(x,y) (SW_buffer + (x) + (y) * SW_buffer_pitch)


//
// The lines we are rendering to for widescreen.
//

SLONG SW_render_top;
SLONG SW_render_bot;


//
// Our textures.
//

typedef struct
{
	CBYTE      name[256];
	SLONG      size;
	TGA_Pixel *data;
	UBYTE      blurred;
	UBYTE      halved;

} SW_Texture;

#ifdef TARGET_DC
#define SW_MAX_TEXTURES 1
#else
#define SW_MAX_TEXTURES (22 * 64 + 110)
#endif

SW_Texture SW_texture[SW_MAX_TEXTURES];


//
// How we draw each texture page.
//

UBYTE SW_page[SW_MAX_TEXTURES];


//
// For alpha pages, this is the z-sort they are forced to have.
//

UWORD SW_bucket[SW_MAX_TEXTURES];


//
// The 'masked' triangles.
//

typedef struct sw_masked
{
	SLONG x1, y1, z1, r1, g1, b1, u1, v1;
	SLONG x2, y2, z2, r2, g2, b2, u2, v2;
	SLONG x3, y3, z3, r3, g3, b3, u3, v3;

	TGA_Pixel *tga;
	SLONG      tga_size;

	struct sw_masked *next;

} SW_Masked;

#ifdef TARGET_DC
#define SW_MAX_MASKED 1
#else
#define SW_MAX_MASKED 4096
#endif

SW_Masked SW_masked[SW_MAX_MASKED];
SLONG     SW_masked_upto;


//
// The masked triangle buckets...
//

#ifdef TARGET_DC
#define SW_MAX_BUCKETS 1
#else
#define SW_MAX_BUCKETS 256
#endif

SW_Masked *SW_masked_bucket[SW_MAX_BUCKETS];






//
// The alpha sprites.
//

typedef struct sw_alpha
{
	SLONG x1, y1, u1, v1;
	SLONG x2, y2, u2, v2;
	SLONG x3, y3, u3, v3;
	
	SLONG wz, wa, wr, wg, wb;

	TGA_Pixel *tga;
	UWORD      tga_size;
	UWORD      mode;

	sw_alpha *next;

} SW_Alpha;

#ifdef TARGET_DC
#define SW_MAX_ALPHAS 1
#else
#define SW_MAX_ALPHAS 2048
#endif

SW_Alpha SW_alpha[SW_MAX_ALPHAS];
SLONG    SW_alpha_upto;

//
// The sprite buckets.
//

SW_Alpha  *SW_alpha_bucket    [SW_MAX_BUCKETS];
SW_Alpha **SW_alpha_bucket_end[SW_MAX_BUCKETS]; // The address of the last pointer of the linked list- so we can add sprites at the end!








void SW_set_page(SLONG page, SLONG type)
{
	if (!WITHIN(page, 0, SW_MAX_TEXTURES - 1))
	{
		return;
	}

	SW_page[page] = type;
}





//
// The spans.
//

typedef struct sw_span
{
	UWORD x1;	// 0-bit fixed point
	UWORD x2;	// 0-bit fixed point

	SLONG z;	// 16-bit fixed point and between 0.0F and 1.0F
	SLONG dz;	// 16-bit fixed point and between 0.0F and 1.0F

	SLONG r;	// 16-bit fixed point.
	SLONG dr;	// 16-bit fixed point.

	SLONG g;	// 16-bit fixed point.
	SLONG dg;	// 16-bit fixed point.

	SLONG b;	// 16-bit fixed point.
	SLONG db;	// 16-bit fixed point.

	SLONG u;	// 16-bit fixed point.
	SLONG du;	// 16-bit fixed point.

	SLONG v;	// 16-bit fixed point.
	SLONG dv;	// 16-bit fixed point.

	TGA_Pixel *tga;
	UWORD      tga_size;
	UWORD      a;	// a is for alpha

	struct sw_span *next;

} SW_Span;


#ifdef TARGET_DC

// I'm not wasting 8Mb for something we'll never need!
#define SW_MAX_SPANS 1

#else

#define SW_MAX_SPANS 0x20000

#endif

SW_Span SW_span[SW_MAX_SPANS];
SLONG   SW_span_upto;




//
// The spans for each line of the screen.
//

// Defined in the header
//#define SW_MAX_HEIGHT 480

SW_Span *SW_line[SW_MAX_HEIGHT];





#ifndef TARGET_DC


// ========================================================
//
// Assembler fixed point thingies from T.A.C.
//
// ========================================================

static inline SLONG DIV16(SLONG a, SLONG b)
{
	SLONG v;
	_asm
	{
		mov		edx,a
		mov		ebx,b
		mov		eax,edx
		shl		eax,16
		sar		edx,16
		idiv	ebx
		mov		v,eax
	}
	return v;
}

#pragma warning( disable : 4035 )	
// stop warning of no return value : eax is valid
static inline SLONG MUL16(SLONG a, SLONG b)
// MSVC++ version, params:ecx,edx, return:eax
// this is as fast on 486/Pentium as the old version
// and 2.5* faster on Pentium Pro
{
	__asm
	{
		mov		eax, a
		imul	b				// result in edx:eax
		shl		edx,16
		shr		eax,16
		or		eax,edx
		// return in eax
	}
}
#pragma warning( default : 4035 )




#else //#ifndef TARGET_DC


// Dreamcast versions. Might be inlined later, just C for now.
// Also very approximate - DC compiler doesn't like long long, so trash the accuracy.
static inline SLONG DIV16(SLONG a, SLONG b)
{
	SLONG ta, tb;
	int iShift;

	// return ( ( a << 16 )  / b );


	iShift = 0;

	if ( a < 0x10000 )
	{
		return ( ( a << 16 ) / b );
	}
	else if ( a < 0x1000000 )
	{
		return ( ( a << 8 ) / ( b >> 8 ) );
	}
	else
	{
		return ( a / ( b >> 16 ) );
	}

}

static inline SLONG MUL16(SLONG a, SLONG b)
{
	// return ( ( a * b ) >> 16 );

	if ( a < 0x10000 )
	{
		// a is small, b might be big.
		return ( ( a * ( b >> 16 ) ) );
	}
	else if ( a < 0x1000000 )
	{
		// both are about the same size.
		return ( ( a >> 8 ) * ( b >> 8 ) );
	}
	else
	{
		// a is big, b should be small.
		return ( ( a >> 16 ) * b );
	}
}


#endif //#else //#ifndef TARGET_DC



// ========================================================
//
// RECIPROCAL CODE
//
// ========================================================


//
// Reciprocals are useful... so we store loads of them!  The reciprocals are
// in 16bit fixed point.  1 / 0 = 1 ... apparently!
//

// These reciprocals are such that recip[x] = DIV16(0x10000, x << 16).

// There is no need ever to access recipminus yourself, just do recip[-5] for
// example, and it will work... honest!
// 

// DONT ACCESS THESE DIRECTLY- GO THROUGH THE #defines!

#define ALWAYS_HOW_MANY_RECIPS_MINUS 1024
#define ALWAYS_HOW_MANY_RECIPS       32768

extern SLONG ALWAYS_recipminus[ALWAYS_HOW_MANY_RECIPS_MINUS];
extern SLONG ALWAYS_recip     [ALWAYS_HOW_MANY_RECIPS];

//
// These reciprocals are such that recippt[x] = DIV16(0x10000, x), as such
// some values cannot be held in an int so recippt[0] and recippt[1] and
// recippt[2] are all set to 0x10000.
//

#ifdef TARGET_DC
#define ALWAYS_HOW_MANY_RECIPPT 1024
#else
#define ALWAYS_HOW_MANY_RECIPPT 65536
#endif

extern SLONG ALWAYS_recippt[ALWAYS_HOW_MANY_RECIPPT];


#ifndef NDEBUG

SLONG ALWAYS_recip_slow  (SLONG x, CBYTE *file, SLONG line);
SLONG ALWAYS_recippt_slow(SLONG x, CBYTE *file, SLONG line);

#define RECIPPT(x)	(ALWAYS_recippt_slow(x, __FILE__, __LINE__))
#define RECIP(x)	(ALWAYS_recip_slow  (x, __FILE__, __LINE__))

#else

#define RECIP(x)	(ALWAYS_recip[x])
#define RECIPPT(x)	(ALWAYS_recippt[x])

#endif



// These two should be kept next to eachother compiler!

SLONG ALWAYS_recipminus[ALWAYS_HOW_MANY_RECIPS_MINUS];
SLONG ALWAYS_recip     [ALWAYS_HOW_MANY_RECIPS];
SLONG ALWAYS_recippt   [ALWAYS_HOW_MANY_RECIPPT];



void ALWAYS_init()
{
	SLONG i;


	// Calculate the reciprocals...

	for (i = 1; i < ALWAYS_HOW_MANY_RECIPS; i++)
	{
		ALWAYS_recip[i] = DIV16(1 << 16, i << 16);
	}

	for (i = 1; i < ALWAYS_HOW_MANY_RECIPS_MINUS; i++)
	{
		ALWAYS_recipminus[ALWAYS_HOW_MANY_RECIPS_MINUS - i] = DIV16(-1 << 16, i << 16);
	}

	for (i = 3; i < ALWAYS_HOW_MANY_RECIPPT; i++)
	{
		ALWAYS_recippt[i] = DIV16(1 << 16, i);
	}

	// 1 / 0 = 1 ... apparently!

	ALWAYS_recip[0] = 0x10000;

	ALWAYS_recippt[0] = 0x10000;
	ALWAYS_recippt[1] = 0x10000;
	ALWAYS_recippt[2] = 0x10000;
}

#ifndef NDEBUG

SLONG ALWAYS_recip_slow(SLONG x, CBYTE *file, SLONG line)
{
	if (x <= -ALWAYS_HOW_MANY_RECIPS_MINUS ||
		x >=  ALWAYS_HOW_MANY_RECIPS)
	{
		ASSERT(0);

		return (0x10000);
	}

	return ALWAYS_recip[x];
}

SLONG ALWAYS_recippt_slow(SLONG x, CBYTE *file, SLONG line)
{
	if (x <  0 ||
		x >= ALWAYS_HOW_MANY_RECIPPT)
	{
		ASSERT(0);

		return (0x10000);
	}

	return ALWAYS_recippt[x];
}


#endif

















void SW_init(
		SLONG width,
		SLONG height)
{
	SLONG i;

	ASSERT(WITHIN(width,  16, SW_MAX_WIDTH ));
	ASSERT(WITHIN(height, 16, SW_MAX_HEIGHT));

	SW_buffer_width  = width;
	SW_buffer_height = height;
	SW_buffer_pitch  = width;
	SW_buffer        = SW_buffer_memory;

	//
	// Initialise the spans.
	//

	memset(SW_line, 0, sizeof(SW_line));

	SW_span_upto = 0;

	//
	// Clear the masked triangle and sprites.
	//

	SW_alpha_upto  = 0;
	SW_masked_upto = 0;

	memset(SW_alpha_bucket,  0, sizeof(SW_alpha_bucket ));
	memset(SW_masked_bucket, 0, sizeof(SW_masked_bucket));

	for (i = 0; i < SW_MAX_BUCKETS; i++)
	{
		SW_alpha_bucket_end[i] = &SW_alpha_bucket[i];
	}

	//
	// Make sure the reciprocals are initialised
	//

	{
		static int recips_initialised = 0;

		if (!recips_initialised)
		{
			recips_initialised = TRUE;

			ALWAYS_init();
		}
	}
}





//
// Inserts a new span. WARNING! This function will allocate
// new spans from the arrays.
//

void SW_insert_span(SW_Span *ss, SLONG line)
{
	SLONG pixels;

	SW_Span  *ss_next;
	SW_Span **ss_prev;

	//
	// Valid span?
	//

	ASSERT(WITHIN(line, 0, SW_buffer_height - 1));

	ASSERT(WITHIN(ss->x1, 0, SW_buffer_width));
	ASSERT(WITHIN(ss->x2, 0, SW_buffer_width));

	ss_prev = &SW_line[line];
	ss_next =  SW_line[line];

	while(1)
	{
		if (ss_next == NULL || ss->x2 <= ss_next->x1)
		{
			//
			// This is where to insert the span.
			//

		   *ss_prev  = ss;
			ss->next = ss_next;

			return;
		}

		//
		// Does the new span overlap with the existing span?
		//

		if (ss->x1 < ss_next->x2)
		{
			//
			// The spans do overlap! Only one can survive!
			//

			//
			// Some very rough z-sort values!
			//

			SLONG ss_sort      = ss->z;
			SLONG ss_next_sort = ss_next->z;

			if (ss_sort < ss_next_sort)
			{
				//
				// The new span is in front of the old one. So we must
				// clip the old span to the new one.
				//

				if (ss->x1 > ss_next->x1)
				{
					//
					// The left bit of the old span survives.
					//

					if (ss->x2 >= ss_next->x2)
					{
						//
						// OOOOOOO			(Old span)
						//	 NNNNNNNNNNN	(New span)
						//
						// We only need to make the old span end sooner!
						//

						ss_next->x2 = ss->x1;

						//
						// Continue to check the next span...
						//

						ss_prev = &ss_next->next;
						ss_next =  ss_next->next;
					}
					else
					{
						//
						// OOOOOOOOOOOOOOO
						//   NNNNNNNNN
						//
						// We need to split the old span into two and put the
						// new one in the middle.
						//

						//
						// Create a new span (the second bit of the old one)
						//

						ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

						SW_Span *ss_extra;
						
						ss_extra = &SW_span[SW_span_upto++];
						
						pixels = ss->x2 - ss_next->x1;

						ss_extra->x1       = ss->x2;
						ss_extra->x2       = ss_next->x2;
						ss_extra->du       = ss_next->du;
						ss_extra->dv       = ss_next->dv;
						ss_extra->dr       = ss_next->dr;
						ss_extra->dg       = ss_next->dg;
						ss_extra->db       = ss_next->db;
						ss_extra->dz       = ss_next->dz;
						ss_extra->u        = ss_next->u + ss_next->du * pixels;
						ss_extra->v        = ss_next->v + ss_next->dv * pixels;
						ss_extra->r        = ss_next->r + ss_next->dr * pixels;
						ss_extra->g        = ss_next->g + ss_next->dg * pixels;
						ss_extra->b        = ss_next->b + ss_next->db * pixels;
						ss_extra->z        = ss_next->z + ss_next->dz * pixels;
						ss_extra->tga      = ss_next->tga;
						ss_extra->tga_size = ss_next->tga_size;

						//
						// Shorten the old one.
						//

						ss_next->x2 = ss->x1;

						//
						// Put the three spans in the correct order.
						//

						ss_extra->next = ss_next->next;
						ss_next->next  = ss;
						ss->next       = ss_extra;
						
						//
						// They should follow on from one-another.
						//

						ASSERT(ss_next->x2 == ss_next->next->x1 && ss_next->next->x2 == ss_next->next->next->x1);

						return;
					}
				}
				else
				{
					//
					// The left hand edge of the old span doesn't survive.
					//

					if (ss->x2 >= ss_next->x2)
					{
						//
						//     OOOOOOOOOO
						//	 NNNNNNNNNNNNNNNNNNN
						//
						// The right hand edge of the old span doesn't survive either.
						// The old span is completely engulfed by the new one.
						//

					   *ss_prev  = ss_next->next;
						ss_next  = ss_next->next;
					}
					else
					{
						//
						//     OOOOOOOOOOO
						//  NNNNNNNNN
						//
						// Make the old span start later.
						//

						pixels = ss->x2 - ss_next->x1;

						ss_next->x1 = ss->x2;
						ss_next->u += ss_next->du * pixels;
						ss_next->v += ss_next->dv * pixels;
						ss_next->r += ss_next->dr * pixels;
						ss_next->g += ss_next->dg * pixels;
						ss_next->b += ss_next->db * pixels;
						ss_next->z += ss_next->dz * pixels;
						
						//
						// Put the new span in before this one.
						//

					   *ss_prev  = ss;
					    ss->next = ss_next;	

						return;
					}
				}
			}
			else
			{
				//
				// The old span is in front of the new one.
				//

				if (ss_next->x1 > ss->x1)
				{
					//
					// The left hand bit of the new span survives.
					//

					if (ss_next->x2 >= ss->x2)
					{
						//
						// NNNNNNNNNN
						//   OOOOOOOOOOOOOOOO
						//
						// Just make the new span end sooner.
						//

						ss->x2 = ss_next->x1;

						//
						// Insert it before the old span.
						//
						
					   *ss_prev  = ss;
						ss->next = ss_next;

						return;
					}
					else
					{
						//
						//  NNNNNNNNNNNNNNN
						//    OOOOOOOOO
						//
						// Spilt the new span into two.
						//

						//
						// The second bit of the new span.
						//

						ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

						SW_Span *ss_extra;
						
						ss_extra = &SW_span[SW_span_upto++];
						
						pixels = ss_next->x2 - ss->x1;

						ss_extra->x1       = ss_next->x2;
						ss_extra->x2       = ss->x2;
						ss_extra->du       = ss->du;
						ss_extra->dv       = ss->dv;
						ss_extra->dr       = ss->dr;
						ss_extra->dg       = ss->dg;
						ss_extra->db       = ss->db;
						ss_extra->dz       = ss->dz;
						ss_extra->u        = ss->u + ss->du * pixels;
						ss_extra->v        = ss->v + ss->dv * pixels;
						ss_extra->r        = ss->r + ss->dr * pixels;
						ss_extra->g        = ss->g + ss->dg * pixels;
						ss_extra->b        = ss->b + ss->db * pixels;
						ss_extra->z        = ss->z + ss->dz * pixels;
						ss_extra->tga      = ss->tga;
						ss_extra->tga_size = ss->tga_size;

						//
						// Shorten the old bit.
						//

						ss->x2 = ss_next->x1;

						//
						// Insert the shortened bit.
						//

					   *ss_prev  = ss;
						ss->next = ss_next;

						//
						// Continue to insert the extra bit...
						//

						ss = ss_extra;

						ss_prev = &ss_next->next;
						ss_next =  ss_next->next;
					}
				}
				else
				{
					//
					// The left hand edge of the new span doesn't survive.
					//

					if (ss_next->x2 >= ss->x2)
					{
						//
						//     NNNNNNNN
						//  OOOOOOOOOOOOO
						//
						// The old span completely engulfs the new one.
						//

						return;
					}
					else
					{
						//
						//     NNNNNNNNNNNN
						//  OOOOOOOOO
						//
						// Just make the new span start later.
						//

						pixels = ss_next->x2 - ss->x1;

						ss->x1 = ss_next->x2;
						ss->u += ss->du * pixels;
						ss->v += ss->dv * pixels;
						ss->r += ss->dr * pixels;
						ss->g += ss->dg * pixels;
						ss->b += ss->db * pixels;
						ss->z += ss->dz * pixels;

						//
						// Continue clipping against the other spans...
						//

						ss_prev = &ss_next->next;
						ss_next =  ss_next->next;
					}
				}

			}
		}
		else
		{
			ss_prev = &ss_next->next;
			ss_next =  ss_next->next;
		}
	}
}





//
// Toms mmx code.
//





//
// Draws an alpha span with the given mode.
//

#define SW_MODE_MASKED		 0
#define SW_MODE_ALPHA		 1
#define SW_MODE_ADDITIVE	 2
#define SW_MODE_ALPHA_NOZ	 3
#define SW_MODE_ADDITIVE_NOZ 4

void SW_draw_span_reference(SW_Span *ss, SLONG line, SLONG mode)
{
	SLONG i;
	SLONG x;

	ULONG *dest;
	ULONG  pixel;
	SLONG  a;
	SLONG  r;
	SLONG  g;
	SLONG  b;
	SLONG  R;
	SLONG  G;
	SLONG  B;
	SLONG  pr;
	SLONG  pg;
	SLONG  pb;
	SLONG  pa;
	SLONG  u;
	SLONG  v;
	SLONG  U;
	SLONG  V;
	SLONG  oa;
	SLONG  or;
	SLONG  og;
	SLONG  ob;
	SLONG  br;
	SLONG  bg;
	SLONG  bb;

	switch(mode)
	{
		case SW_MODE_MASKED:

			r = ss->r;
			g = ss->g;
			b = ss->b;
			u = ss->u;
			v = ss->v;

			dest = SW_buffer + ss->x1 + line * SW_buffer_pitch;

			for (x = ss->x1; x < ss->x2; x++)
			{
				R  = r >> 16;
				G  = g >> 16;
				B  = b >> 16;
				U  = u >> 16;
				V  = v >> 16;
				U &= ss->tga_size - 1;
				V &= ss->tga_size - 1;

				pa = ss->tga[U + V * ss->tga_size].alpha;
				pr = ss->tga[U + V * ss->tga_size].red;
				pg = ss->tga[U + V * ss->tga_size].green;
				pb = ss->tga[U + V * ss->tga_size].blue;

				if (pa <= 128)
				{
					dest++;
				}
				else
				{
					ASSERT(WITHIN(R, 0, 255));
					ASSERT(WITHIN(G, 0, 255));
					ASSERT(WITHIN(B, 0, 255));

					pr = pr * R >> 7;
					pg = pg * G >> 7;
					pb = pb * B >> 7;

					if (pr > 255) {pr = 255;}
					if (pg > 255) {pg = 255;}
					if (pb > 255) {pb = 255;}

				   *dest++ = (pr << 16) | (pg << 8) | pb;
				}

				r += ss->dr;
				g += ss->dg;
				b += ss->db;
				u += ss->du;
				v += ss->dv;
			}

			break;

		case SW_MODE_ALPHA:

			//
			// Constant rgba...
			//

			a = ss->a;
			r = ss->r;
			g = ss->g;
			b = ss->b;
			u = ss->u;
			v = ss->v;

			dest = SW_buffer + ss->x1 + line * SW_buffer_pitch;

			for (x = ss->x1; x < ss->x2; x++)
			{
				U  = u >> 16;
				V  = v >> 16;
				U &= ss->tga_size - 1;
				V &= ss->tga_size - 1;

				pa = ss->tga[U + V * ss->tga_size].alpha;
				pr = ss->tga[U + V * ss->tga_size].red;
				pg = ss->tga[U + V * ss->tga_size].green;
				pb = ss->tga[U + V * ss->tga_size].blue;

				if (pa == 0)
				{
					//
					// Optimisation? Maybe not!
					//

					dest++;
				}
				else
				{
					//
					// Source values.
					//

					pa = pa * a >> 8;
					pr = pr * r >> 8;
					pg = pg * g >> 8;
					pb = pb * b >> 8;

					if (pr > 255) {pr = 255;}
					if (pg > 255) {pg = 255;}
					if (pb > 255) {pb = 255;}

					if (pa == 255)
					{
					   *dest++ = (pr << 16) | (pg << 8) | pb;
					}
					else
					{
						//
						// The rgb of the pixel now.
						//

						or = (*dest >> 16) & 0xff;
						og = (*dest >>  8) & 0xff;
						ob = (*dest >>  0) & 0xff;
						
						//
						// Blend!
						//

						br = or * (255 - pa) + pr * pa >> 8;
						bg = og * (255 - pa) + pg * pa >> 8;
						bb = ob * (255 - pa) + pb * pa >> 8;

						ASSERT(WITHIN(br, 0, 255));
						ASSERT(WITHIN(bg, 0, 255));
						ASSERT(WITHIN(bb, 0, 255));

						//
						// Plot!
						//

					   *dest++ = (br << 16) | (bg << 8) | bb;
					}
				}

				u += ss->du;
				v += ss->dv;
			}

			break;

		case SW_MODE_ADDITIVE:

			//
			// Constant rgba...
			//

			r = ss->r;
			g = ss->g;
			b = ss->b;
			u = ss->u;
			v = ss->v;

			dest = SW_buffer + ss->x1 + line * SW_buffer_pitch;

			for (x = ss->x1; x < ss->x2; x++)
			{
				U  = u >> 16;
				V  = v >> 16;
				U &= ss->tga_size - 1;
				V &= ss->tga_size - 1;

				pr = ss->tga[U + V * ss->tga_size].red;
				pg = ss->tga[U + V * ss->tga_size].green;
				pb = ss->tga[U + V * ss->tga_size].blue;

				pr = pr * r >> 7;
				pg = pg * g >> 7;
				pb = pb * b >> 7;

				//
				// The rgb of the pixel now.
				//

				or = (*dest >> 16) & 0xff;
				og = (*dest >>  8) & 0xff;
				ob = (*dest >>  0) & 0xff;

				pr += or;
				pg += og;
				pb += ob;

				if (pr > 255) {pr = 255;}
				if (pg > 255) {pg = 255;}
				if (pb > 255) {pb = 255;}

			   *dest++ = (pr << 16) | (pg << 8) | pb;

				u += ss->du;
				v += ss->dv;
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}




#ifndef TARGET_DC




#define SWIZZLE 1

#define ALPHA_NONE 0
#define ALPHA_TEST 1
#define ALPHA_ADD 2
#define ALPHA_BLEND 3


inline void SW_draw_span_masked(SW_Span *ss, SLONG line, SLONG mode)
{
	#undef  ALPHA_MODE
	#define ALPHA_MODE ALPHA_TEST

	#include "tom.cpp"
}

inline void SW_draw_span_alpha(SW_Span *ss, SLONG line, SLONG mode)
{
	#undef  ALPHA_MODE
	#define ALPHA_MODE ALPHA_BLEND

	#include "tom.cpp"
}


inline void SW_draw_span_additive(SW_Span *ss, SLONG line, SLONG mode)
{
	#undef  ALPHA_MODE
	#define ALPHA_MODE ALPHA_ADD

	#include "tom.cpp"
}

void SW_draw_span(SW_Span *ss, SLONG line, SLONG mode)
{
	switch(mode)
	{
		case SW_MODE_MASKED:
			SW_draw_span_masked(ss, line, mode);
			break;
		
		case SW_MODE_ALPHA:

			ss->r <<= 16;
			ss->g <<= 16;
			ss->b <<= 16;

			SW_draw_span_alpha(ss, line, mode);

			break;
		
		case SW_MODE_ADDITIVE:

			ss->r <<= 16;
			ss->g <<= 16;
			ss->b <<= 16;

			SW_draw_span_additive(ss, line, mode);

			break;
		
		default:
			ASSERT(0);
			break; 
	}
}


#else //#ifndef TARGET_DC


// Cheezily wrapped for now.
void SW_draw_span(SW_Span *ss, SLONG line, SLONG mode)
{
	SW_draw_span_reference(ss, line, mode);
}


#endif //#else //#ifndef TARGET_DC





void SW_add_masked_triangle(
		SLONG x1, SLONG y1, SLONG z1, SLONG r1, SLONG g1, SLONG b1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG z2, SLONG r2, SLONG g2, SLONG b2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG z3, SLONG r3, SLONG g3, SLONG b3, SLONG u3, SLONG v3,
		TGA_Pixel *tga,
		SLONG      tga_size)
{
	//
	// The zsort value of the triangle.
	//

	z1 -= 4;
	z2 -= 4;
	z3 -= 4;

	SLONG zsort = z1 + z2 + z2 + z3 >> 4;

	SATURATE(zsort, 0, SW_MAX_BUCKETS - 1);

	//
	// Create a masked triangle structure.
	//

	ASSERT(WITHIN(SW_masked_upto, 0, SW_MAX_MASKED - 1));

	SW_Masked *sm = &SW_masked[SW_masked_upto++];

	sm->x1 = x1; sm->y1 = y1; sm->z1 = z1; sm->r1 = r1; sm->g1 = g1; sm->b1 = b1; sm->u1 = u1; sm->v1 = v1;
	sm->x2 = x2; sm->y2 = y2; sm->z2 = z2; sm->r2 = r2; sm->g2 = g2; sm->b2 = b2; sm->u2 = u2; sm->v2 = v2;
	sm->x3 = x3; sm->y3 = y3; sm->z3 = z3; sm->r3 = r3; sm->g3 = g3; sm->b3 = b3; sm->u3 = u3; sm->v3 = v3;
	sm->tga      = tga;
	sm->tga_size = tga_size;

	//
	// Add it to the bucket list.
	//

	sm->next                = SW_masked_bucket[zsort];
	SW_masked_bucket[zsort] = sm;
}

void SW_add_alpha_sprite(
		SLONG x1, SLONG y1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG u3, SLONG v3,
		SLONG wz, SLONG wa, SLONG wr, SLONG wg, SLONG wb,	// For the whole triangle.
		TGA_Pixel *tga,
		SLONG      tga_size,
		SLONG      mode)
{
	//
	// The zsort value of the triangle.
	//

	SLONG zsort = wz >> 2;

	SATURATE(zsort, 0, SW_MAX_BUCKETS - 1);

	//
	// Create a masked triangle structure.
	//

	ASSERT(WITHIN(SW_alpha_upto, 0, SW_MAX_ALPHAS - 1));

	SW_Alpha *sa = &SW_alpha[SW_alpha_upto++];

	sa->x1 = x1; sa->y1 = y1; sa->u1 = u1; sa->v1 = v1;
	sa->x2 = x2; sa->y2 = y2; sa->u2 = u2; sa->v2 = v2;
	sa->x3 = x3; sa->y3 = y3; sa->u3 = u3; sa->v3 = v3;
	sa->tga      = tga;
	sa->tga_size = tga_size;
	sa->wz       = wz;
	sa->wa       = wa;
	sa->wr       = wr;
	sa->wg       = wg;
	sa->wb       = wb;
	sa->mode     = mode;

	//
	// Add it to the end of the bucket list- so the polys are drawn in the
	// order they are submitted!
	//

	sa->next                     = NULL;
   *(SW_alpha_bucket_end[zsort]) = sa;
	SW_alpha_bucket_end[zsort]   = &sa->next;
}





//
// Clips the span and then draws the bits that survive.
//

void SW_clip_and_draw_span(SW_Span *ss, SLONG line, SLONG mode)
{	
	SLONG ss_sort;
	SLONG ss_next_sort;
	SLONG pixels;

	SW_Span *ss_next;

	//
	// Valid span?
	//

	ASSERT(WITHIN(line, 0, SW_buffer_height - 1));

	ASSERT(WITHIN(ss->x1, 0, SW_buffer_width));
	ASSERT(WITHIN(ss->x2, 0, SW_buffer_width));

	ss_next = SW_line[line];

	//
	// The sort value of this span.
	//
	
	ss_sort = ss->z;

	while(1)
	{
		if (ss_next == NULL || ss->x2 <= ss_next->x1)
		{
			//
			// Draw the span.
			//

			SW_draw_span(ss, line, mode);

			return;
		}

		//
		// Does the alpha span overlap with the existing span?
		//

		if (ss->x1 < ss_next->x2)
		{
			//
			// Yes! What is the zsort of the two spans?
			//

			ss_sort      = ss->z;
			ss_next_sort = ss_next->z;

			if (ss_sort > ss_next_sort)
			{
				//
				// The alpha span is behind- so we must clip it.
				//

				if (ss->x1 < ss_next->x1)
				{
					//
					// The left hand edge of the alpha span survives.
					//

					if (ss->x2 > ss_next->x2)
					{
						//
						// AAAAAAAAAAAAAAAAA
						//    OOOOOOOOOO
						//
						// There is also another bit to the right, that
						// survives as well.
						//

						//
						// OPTIMISATION REQUIRED? THERE IS PROBABLY ANOTHER SPAN
						// AFTER THIS ONE SO WE COULD NOT BOTHER CREATING THE
						// EXTRA SPAN...
						//

						//
						// Create the new span to the right.
						//

						ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

						SW_Span *ss_extra = &SW_span[SW_span_upto++];

						pixels = ss_next->x2 - ss->x1;

						ss_extra->x1       = ss_next->x2;
						ss_extra->x2       = ss->x2;
						ss_extra->u        = ss->u + ss->du * pixels;
						ss_extra->v        = ss->v + ss->dv * pixels;
						ss_extra->du       = ss->du;
						ss_extra->dv       = ss->dv;
						ss_extra->tga      = ss->tga;
						ss_extra->tga_size = ss->tga_size;

						if (mode == SW_MODE_MASKED)
						{
							ss_extra->z  = ss->z + ss->dz * pixels;
							ss_extra->r  = ss->r + ss->dr * pixels;
							ss_extra->g  = ss->g + ss->dg * pixels;
							ss_extra->b  = ss->b + ss->db * pixels;
							ss_extra->dz = ss->dz;
							ss_extra->dr = ss->dr;
							ss_extra->dg = ss->dg;
							ss_extra->db = ss->db;
						}
						else
						{
							ss_extra->z = ss->z;
							ss_extra->r = ss->r;
							ss_extra->g = ss->g;
							ss_extra->b = ss->b;
							ss_extra->a = ss->a;
						}

						//
						// Shorten the left-hand span and draw it.
						//

						ss->x2 = ss_next->x1;

						SW_draw_span(ss, line, mode);

						//
						// Continue checking the next span.
						//

						ss = ss_extra;
					}
					else
					{
						//
						// AAAAAAAAA
						//   OOOOOOOOOO
						//
						// Easy just shorten the length of the alpha span and draw it.
						//

						ss->x2 = ss_next->x1;

						SW_draw_span(ss, line, mode);

						return;
					}
				}
				else
				{
					//
					// The left hand edge of the alpha span doesn't survive.
					//

					if (ss->x2 < ss_next->x2)
					{
						//
						// The right hand edge doesn't survive either. It it
						// completely z-buffered out.
						//

						return;
					}
					else
					{
						//
						//   AAAAAAAAAAAAA
						// OOOOOOOOOO
						//
						// Only the right hand edge of the alpha span survives.
						//

						pixels = ss_next->x2 - ss->x1;

						ss->x1 = ss_next->x2;
						ss->u += ss->du * pixels;
						ss->v += ss->dv * pixels;

						if (mode == SW_MODE_MASKED)
						{
							ss->z += ss->dz * pixels;
							ss->r += ss->dr * pixels;
							ss->g += ss->dg * pixels;
							ss->b += ss->db * pixels;
						}

						//
						// Carry on checking the other spans...
						//
					}
				}
			}
		}

		ss_next = ss_next->next;
	}
}



//
// Swizzles a texture.
//

void SW_swizzle(TGA_Pixel *tga, SLONG size)
{
	SLONG i;

	SLONG x;
	SLONG y;
	
	SLONG index_normal;
	SLONG index_swizzle;

	TGA_Pixel *buffer;

	//
	// Use the back buffer memory as a temporary swizzle buffer.
	//

	ASSERT(size <= 256);

	buffer = (TGA_Pixel *) SW_buffer_memory;

	memcpy(buffer, tga, size * size * sizeof(TGA_Pixel));

	//
	// Swizzle the texture.
	//

	for (x = 0; x < size; x++)
	for (y = 0; y < size; y++)
	{
		//
		// The index of the unswizzled pixel.
		//

		index_normal = x + y * size;

		//
		// The index of the swizzled pixel.
		//

		index_swizzle = 0;

		for (i = 0; i < 10; i++)
		{
			if (x & (1 << i))
			{
				index_swizzle |= (1 << (i * 2));
			}

			if (y & (1 << i))
			{
				index_swizzle |= (1 << (i * 2 + 1));
			}
		}

		tga[index_swizzle] = buffer[index_normal];
	}
}




void SW_blur(TGA_Pixel *tga, SLONG size)
{
	SLONG i;

	SLONG x;
	SLONG y;

	SLONG dx;
	SLONG dy;

	SLONG px;
	SLONG py;

	SLONG r;
	SLONG g;
	SLONG b;
	SLONG a;
	SLONG dist;
	SLONG samples;
	SLONG index;
	
	TGA_Pixel *buffer;

	//
	// Use the back buffer memory as a temporary blur buffer.
	//

	ASSERT(size <= 256);

	buffer = (TGA_Pixel *) SW_buffer_memory;

	memcpy(buffer, tga, size * size * sizeof(TGA_Pixel));

	//
	// blur the texture.
	//

	for (x = 0; x < size; x++)
	for (y = 0; y < size; y++)
	{
		r = 0;
		g = 0;
		b = 0;
		a = 0;

		samples = 0;

		for (dx = -1; dx <= +1; dx++)
		for (dy = -1; dy <= +1; dy++)
		{
			px = x + dx;
			py = y + dy;

			if (WITHIN(px, 0, size - 1) &&
				WITHIN(py, 0, size - 1))
			{
				static SLONG mul[3] =
				{
					664,
					260,
					150,
				};

				dist  = abs(dx) + abs(dy);
				index = px + py * size;

				r += buffer[index].red   * mul[dist] >> 8;
				g += buffer[index].green * mul[dist] >> 8;
				b += buffer[index].blue  * mul[dist] >> 8;
				a += buffer[index].alpha * mul[dist] >> 8;

				samples += 1;
			}
		}

		r /= samples;
		g /= samples;
		b /= samples;
		a /= samples;

		if (r > 128) {r += 64;}
		if (g > 128) {g += 64;}
		if (b > 128) {b += 64;}

		SATURATE(r, 0, 255);
		SATURATE(g, 0, 255);
		SATURATE(b, 0, 255);
		SATURATE(a, 0, 255);

		index = x + y * size;

		tga[index].red    = r;
		tga[index].green  = g;
		tga[index].blue   = b;
		tga[index].alpha  = a;
	}
}


void SW_halfsize(TGA_Pixel *tga, SLONG size)
{
	SLONG i;

	SLONG x;
	SLONG y;

	SLONG dx;
	SLONG dy;

	SLONG px;
	SLONG py;

	SLONG r;
	SLONG g;
	SLONG b;
	SLONG a;
	SLONG dist;
	SLONG samples;
	SLONG index;
	SLONG index1;
	SLONG index2;
	
	TGA_Pixel *buffer;

	ASSERT(size >= 32);

	//
	// Use the back buffer memory as a temporary blur buffer.
	//

	ASSERT(size <= 256);

	buffer = (TGA_Pixel *) SW_buffer_memory;

	memcpy(buffer, tga, size * size * sizeof(TGA_Pixel));

	//
	// Half the size of the texture.
	//

	index1 = 0;

	for (x = 0; x < size; x += 2)
	for (y = 0; y < size; y += 2)
	{
		index = x + y * size;

		r = buffer[index].red;
		g = buffer[index].green;
		b = buffer[index].blue;
		a = buffer[index].alpha;

		r += buffer[index + 1].red;
		g += buffer[index + 1].green;
		b += buffer[index + 1].blue;
		a += buffer[index + 1].alpha;

		r += buffer[index + size].red;
		g += buffer[index + size].green;
		b += buffer[index + size].blue;
		a += buffer[index + size].alpha;

		r += buffer[index + size + 1].red;
		g += buffer[index + size + 1].green;
		b += buffer[index + size + 1].blue;
		a += buffer[index + size + 1].alpha;

		r >>= 2;
		g >>= 2;
		b >>= 2;
		a >>= 2;

		index = (x >> 1) + (y >> 1) * (size >> 1);

		tga[index].red   = r;
		tga[index].green = g;
		tga[index].blue  = b;
		tga[index].alpha = a;
	}
}




SLONG mulshift;


void SW_reload_textures()
{
	SLONG    i;
	SLONG    j;
	SLONG    flat_page_hack;
	SLONG    tt_index;
	TGA_Info ti;

	SW_Texture *st;

	//
	// Is this the last level?
	//

	extern SLONG playing_level(const CBYTE *name);

	if (playing_level("Finale1.ucm"))
	{
		mulshift = 1;
	}
	else
	{
		mulshift = 2;
	}

	//
	// Load from clumps where required.
	//

	extern void TEXTURE_initialise_clumping(CBYTE *fname_level);

	TEXTURE_initialise_clumping(ELEV_fname_level);

	for (i = 0; i < SW_MAX_TEXTURES; i++)
	{
		st = &SW_texture[i];

		//
		// Which TEXTURE_texture do we use for this SW_texture?
		//

		flat_page_hack = FALSE;

		if (i < 22 * 64)
		{
			tt_index = i;
		}
		else
		{
			switch(i)
			{
				case POLY_PAGE_SMOKECLOUD2:
					tt_index = TEXTURE_page_smokecloud;
					break;

				case POLY_PAGE_BIGBANG:
					tt_index = TEXTURE_page_bigbang;
					break;

				case POLY_PAGE_EXPLODE1:
					tt_index = TEXTURE_page_explode1;
					break;

				case POLY_PAGE_EXPLODE1_ADDITIVE:
					tt_index = TEXTURE_page_explode1;
					break;

				case POLY_PAGE_EXPLODE2_ADDITIVE:
					tt_index = TEXTURE_page_explode2;
					break;

				case POLY_PAGE_DUSTWAVE:
					tt_index= TEXTURE_page_dustwave;
					break;

				case POLY_PAGE_FACE1:
					tt_index     = TEXTURE_page_face1;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_FACE2:
					tt_index     = TEXTURE_page_face2;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_NEWFONT_INVERSE:
					tt_index = TEXTURE_page_lcdfont;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_COLOUR:
					flat_page_hack = TRUE;
					break;

				case POLY_PAGE_LADDER:
					tt_index = TEXTURE_page_ladder;
					break;

				case POLY_PAGE_LEAF:
					tt_index = TEXTURE_page_leaf;
					break;

				case POLY_PAGE_RUBBISH:
					tt_index = TEXTURE_page_rubbish;
					break;

				/*

				case POLY_PAGE_SHADOW_OVAL:
					tt_index = TEXTURE_page_shadowoval;
					break;

				*/
				
				case POLY_PAGE_FONT2D:
					tt_index = TEXTURE_page_font2d;
					SW_bucket[i] = 1;	// Force to have a zsort of 1.
					break;

				case POLY_PAGE_LASTPANEL_ALPHA:
					tt_index     = TEXTURE_page_lastpanel;
					SW_bucket[i] = 5;
					break;

				case POLY_PAGE_LASTPANEL_ADDALPHA:
					tt_index     = TEXTURE_page_lastpanel;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_LASTPANEL_ADD:
					tt_index     = TEXTURE_page_lastpanel;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_LASTPANEL2_ADD:
					tt_index     = TEXTURE_page_lastpanel2;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_LASTPANEL2_ADDALPHA:
					tt_index     = TEXTURE_page_lastpanel2;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_LASTPANEL2_ALPHA:
					tt_index     = TEXTURE_page_lastpanel2;
					SW_bucket[i] = 1;
					break;

				case POLY_PAGE_FLAMES2:
					tt_index = TEXTURE_page_flame2;
					break;

				case POLY_PAGE_PCFLAMER:
					tt_index = TEXTURE_page_pcflamer;
					break;
				
				case POLY_PAGE_BLOOM1:
					tt_index = TEXTURE_page_bloom1;
					break;

				case POLY_PAGE_BLOODSPLAT:
					tt_index = TEXTURE_page_bloodsplat;
					break;

				case POLY_PAGE_COLOUR_ALPHA:
					flat_page_hack = TRUE;
					SW_bucket[i] = 9;
					break;

#ifdef TARGET_DC
				case POLY_PAGE_BACKGROUND_IMAGE:
					tt_index = TEXTURE_page_background_use_instead;
					break;
				case POLY_PAGE_BACKGROUND_IMAGE2:
					tt_index = TEXTURE_page_background_use_instead2;
					break;
#endif

				default:
					
					//
					// Unsupported texture.
					//
					ASSERT ( FALSE );

					continue;
			}
		}

		if (flat_page_hack)
		{
			if (st->data)
			{
				//
				// We already have data here...
				//
			}
			else
			{
				st->data = (TGA_Pixel *) MemAlloc(32 * 32 * sizeof(TGA_Pixel));
				st->size = 32;

				memset(st->data, -1, 32 * 32 * sizeof(TGA_Pixel));
			}
		}
		else
		if (TEXTURE_texture[tt_index].Type == D3DTEXTURE_TYPE_UNUSED)
		{
			//
			// We should unload our SW_texture if it is used.
			//

			if (st->data)
			{
				TGA_Pixel *no_longer_valid = st->data;

				MemFree(st->data);

				if (i >= 64 * 22)
				{
					//
					// This texture might be shared...
					//

					for (j = 64 * 22; j < SW_MAX_TEXTURES; j++)
					{
						if (SW_texture[j].data == no_longer_valid)
						{
							memset(&SW_texture[j], 0, sizeof(SW_Texture));
						}
					}
				}

				memset(st, 0, sizeof(SW_Texture));
			}
		}
		else
		{
			//
			// Do we already have this texture loaded?
			//

			if (strcmp(TEXTURE_texture[tt_index].texture_name, st->name) == 0)
			{
				//
				// Yes! Do nothing.
				//
			}
			else
			{
				if (st->data)
				{
					//
					// We have the wrong texture loaded.
					//

					MemFree(st->data);

					memset(st, 0, sizeof(SW_Texture));
				}

				if (i >= 64 * 22)
				{
					//
					// Do we share this texture with another page?
					//

					for (j = 64 * 22; j < SW_MAX_TEXTURES; j++)
					{
						if (strcmp(TEXTURE_texture[tt_index].texture_name, SW_texture[j].name) == 0)
						{
							//
							// Just use this tga.
							//

							ASSERT(SW_texture[j].data);

						   *st = SW_texture[j];	
							
							continue;
						}
					}
				}

				//
				// No. We should load this texture.
				//

				st->size = TEXTURE_texture[tt_index].size;

				if (st->size == 0)
				{
					st->size = 128;
				}

				st->data    = (TGA_Pixel *) MemAlloc(st->size * st->size * sizeof(TGA_Pixel));
				st->blurred = FALSE;
				
				strcpy(st->name, TEXTURE_texture[tt_index].texture_name);

				ti = TGA_load(st->name, st->size, st->size, st->data, TEXTURE_texture[tt_index].ID);

				if (!ti.valid)
				{
					//
					// Error loading texture TGA.
					//

					MemFree(st->data);

					memset(st, 0, sizeof(SW_Texture));
				}
				else
				{
					if (ti.width < st->size)
					{
						st->size = ti.width;

						st->data = (TGA_Pixel *) realloc(st->data, st->size * st->size * sizeof(TGA_Pixel));
					}

					if (i < 64 * 22)
					{
						if (RealDisplayWidth  < 640 ||
							RealDisplayHeight < 480)
						{
							//
							// Half the size of the texture!
							//

							ASSERT(!st->halved);

							SW_halfsize(st->data, st->size);
							
							st->size >>= 1;
							st->halved = TRUE;
							st->data   = (TGA_Pixel *) realloc(st->data, st->size * st->size * sizeof(TGA_Pixel));
						}
					}

					if (tt_index == TEXTURE_page_font2d || tt_index == TEXTURE_page_lastpanel || tt_index == TEXTURE_page_lastpanel2)
					{
						extern SLONG RealDisplayWidth;
						extern SLONG RealDisplayHeight;

						if (RealDisplayWidth  < 640 ||
							RealDisplayHeight < 480)
						{
							//
							// Blur the texture!
							//

							if (!st->blurred)
							{
								SW_blur(st->data, st->size);

								st->blurred = TRUE;
							}
						}
					}

					//
					// Swizzle the texture.
					//

					SW_swizzle(st->data, st->size);
				}
			}
		}
	}

	CloseTGAClump();
}



#if !defined (TARGET_DC)


//
// Thanks, Tom.
//

void SW_render_spans_tom(void)
{
	SLONG i;
	SLONG x;

	ULONG *dest, addr, *tex;
	ULONG  pixel;
	SLONG  r, rd;
	SLONG  g, gd;
	SLONG  b, bd;
	SLONG  R;
	SLONG  G;
	SLONG  B;
	SLONG  pr;
	SLONG  pg;
	SLONG  pb;
	SLONG  u, ud, tempu;
	SLONG  v, vd, tempv;
	SLONG  U;
	SLONG  V;
	ULONG	wrap, wrap1, wrap2;
	int tempx1,tempx2;

	SW_Span *ss;


	ULONG *last_dest;
	_int64 mmt1, mmt2, mmt3, mmt4, umask, vmask, wrapmask, uinc, vinc, notumask, notvmask, alpha_test_value, alpha_mask;
	ULONG utemp, vtemp;

	umask = 0x5555ffff5555ffff;
	vmask = 0xaaaaffffaaaaffff;
	notumask = ~0x5555ffff5555ffff;
	notvmask = ~0xaaaaffffaaaaffff;
	alpha_test_value = 0xff000000ff000000;
	alpha_mask = 0x00007fff00000000;

#define SWIZZLE 1


#define ALPHA_BLEND_NOT_DOUBLE_LIGHTING 1

#undef  ALPHA_MODE
#define ALPHA_MODE ALPHA_NONE

	for (i = SW_render_top; i < SW_render_bot; i++)
	{
		for (ss = SW_line[i]; ss; ss = ss->next)
		{
			//r = ss->r;
			//g = ss->g;
			//b = ss->b;
			//u = ss->u;
			//v = ss->v;
			//ud = ss->du;
			//vd = ss->dv;

			tempx1 = ss->x1;
			tempx2 = ss->x2;

			if ( tempx2 > tempx1 )
			{

				dest = SW_buffer + i * SW_buffer_pitch;
				last_dest = dest + tempx2;
				dest += tempx1;
				if ( ( tempx2 & 0x1 ) != 0 ) 
				{
					// Last pixel is odd - go one less so the loop ends properly.
					// Remember that we are stepping two pixels at a time, and
					// ending when we hit this one, then maybe drawing the last pixel.

					// (e.g. x2 is 7, so last pixel to draw is 6, so terminate when we get to 6 - then do one more)
					// (e.g. x2 is 8, so last pixel to draw is 7, so terminate when we get to 8)

					last_dest--;
				}

				r  = (ss->r )>>1;
				g  = (ss->g )>>1;
				b  = (ss->b )>>1;
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				rd = (ss->dr)>>1;
				gd = (ss->dg)>>1;
				bd = (ss->db)>>1;
#endif
				// Swizzle interpolators
				u = ss->u;
				v = ss->v;
				ud = ss->du;
				vd = ss->dv;
				tex = (ULONG *)(ss->tga);

				// Remember this is for DWORDS - assembler doesn't automagically scale.
#if SWIZZLE
				wrap = ((ss->tga_size * ss->tga_size) - 1)<<2;
#else
				wrap = ss->tga_size;
#endif

				__asm {

				push esi
				push edi
				push eax
				push ebx
				push ecx
				push edx

#if SWIZZLE
				movd	mm0,[wrap]
				movq	mm1,mm0
				psllq	mm0,32
				por		mm0,mm1
				movq	[wrapmask],mm0
#else
				mov		eax,[wrap]
				mov		ebx,0x200
				mov		ecx,9
wrap_loop:
				dec		ecx
				shr		ebx,1
				cmp		ebx,eax
				jne		wrap_loop

				mov		eax,[v]
				mov		ebx,[vd]
				shl		eax,cl
				shl		ebx,cl
				mov		[v],eax
				mov		[vd],ebx

				mov		eax,[wrap]
				dec		eax
				movd	mm0,eax
				psllq	mm0,16
				movq	mm1,mm0
				psllq	mm0,32
				por		mm0,mm1
				movq	[umask],mm0
				movd	mm1,ecx
				psllq	mm0,mm1
				movq	[vmask],mm0
#endif

				mov		eax,[r]
				mov		ebx,[g]
				mov		ecx,[b]
				and		eax,0x00ffff00
				and		ebx,0x00ffff00
				and		ecx,0x00ffff00
				movd	mm0,ebx
				movd	mm1,ecx
				shr		eax,8
				psllq	mm0,8
				psrlq	mm1,8
				por		mm0,mm1

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				or		eax,0x7fff0000
#else
				or		eax,0x40000000
#endif
				movd	mm1,eax
				psllq	mm1,32
				por		mm0,mm1
				movq	[mmt1],mm0


#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				mov		eax,[rd]
				mov		ebx,[gd]
				mov		ecx,[bd]
				and		eax,0x00ffff00
				and		ebx,0x00ffff00
				and		ecx,0x00ffff00
				movd	mm0,ebx
				movd	mm1,ecx
				shr		eax,8
				psllq	mm0,8
				psrlq	mm1,8
				por		mm0,mm1
				movd	mm1,eax
				psllq	mm1,32
				por		mm0,mm1
				movq	[mmt2],mm0
#endif

#if SWIZZLE
				mov		eax,[u]
				mov		ebx,[v]
				lea		esi,[swizzle_table]
				mov		ecx,eax
				mov		edx,ebx
				shr		ecx,16
				shr		edx,16
				and		ecx,0xf
				and		edx,0xf
				shr		eax,20
				shr		ebx,20
				and		eax,0xf
				and		ebx,0xf
				mov		ah,[esi+eax]
				mov		bh,[esi+ebx]
				mov		al,[esi+ecx]
				mov		bl,[esi+edx]
				shl		ebx,1
				mov		WORD PTR[u+2],ax
				mov		WORD PTR[v+2],bx

				mov		eax,[ud]
				mov		ebx,[vd]
				mov		ecx,eax
				mov		edx,ebx
				shr		ecx,16
				shr		edx,16
				and		ecx,0xf
				and		edx,0xf
				shr		eax,20
				shr		ebx,20
				and		eax,0xf
				and		ebx,0xf
				mov		ah,[esi+eax]
				mov		bh,[esi+ebx]
				mov		al,[esi+ecx]
				mov		bl,[esi+edx]
				shl		ebx,1
				or		eax,0xaaaa
				or		ebx,0x5555
				mov		WORD PTR[ud+2],ax
				mov		WORD PTR[vd+2],bx
#endif



				movq	mm0, [mmt1]
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				movq	mm2, [mmt2]
#endif
				mov		edi, [dest]
				mov		esi, [tex]


				// Set up the integer texel fetcher
				mov			eax,[u]
				mov			ebx,[v]



				;Is the first pixel odd?
				mov		ecx,[tempx1]
				test	ecx,0x1
				jnz		first_pixel_odd



#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				// Calculate r1g1b1, and double up the increments
				movq	mm1,mm0
				paddsw	mm1,mm2
				psllw	mm2,1
#endif




#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff

				mov		edx,eax
				add		eax,DWORD PTR [ud]
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
				or		ecx,edx
				shr		ecx,16-2
				push	ecx

				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
				or		edx,ecx
				shr		edx,16-2
				pop		ecx
#endif
				movd	mm7,[esi+ecx]
				movd	mm6,[esi+edx]


				jmp		setup_main_loop


first_pixel_odd:


#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
#else
				mov		ecx,eax
				mov		edx,ebx
				add		eax,DWORD PTR [ud]
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
				or		ecx,edx
				shr		ecx,16-2
#endif
				movd	mm7,[esi+ecx]




#if ALPHA_MODE==ALPHA_BLEND
		

				punpcklbw mm7,mm7			;unpack
	#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
	#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
	#endif

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				psrlw	mm7,8-1
#else
				psrlw	mm7,8-2
#endif
	#if SWIZZLE
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				mov		edx,ebx
				add		ebx,DWORD PTR [vd]
	#endif
				pmulhw	mm7,mm0				;colour modulate
				movq	mm6,mm7

	#if SWIZZLE
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
	#endif
				psrlq	mm6,8+1				;half required value (creates a sign bit)
				pand	mm6,[alpha_mask]
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		ecx,edx
				shr		ecx,16-2
	#endif
				movq	mm5,mm6
	#if SWIZZLE
				mov		edx,eax
				add		eax,DWORD PTR [ud]
	#else
				push	ecx
				mov		ecx,eax
	#endif

				psrlq	mm5,16
	#if SWIZZLE
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
	#endif

				por		mm5,mm6
	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
	#endif
				psrlq	mm5,16
	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				and		edx,DWORD PTR [vmask]
				or		edx,ecx
	#endif
				por		mm5,mm6				;alpha channel replicated to r,g,b
	#if SWIZZLE
	#else
				pop		ecx
				shr		edx,16-2
	#endif
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format
				// Remember that mm2 is spare - no gouard shading on alpha-blenders.

				movd	mm6,[edi]

				punpcklbw mm6,mm6			;unpack dest
				psrlw	mm6,8
				
				psubsw	mm7,mm6				;src-dest
				pmulhw	mm7,mm5				;alpha*(src-dest)
				
				paddsw	mm7,mm7				;*2
				paddsw	mm7,mm6				;dest+alpha*(src-dest)
				
				packuswb mm7,mm7			;re-pack


				movd	[edi],mm7			;store pixel

				add		edi,4



#else //#if ALPHA_MODE==ALPHA_BLEND

				punpcklbw mm7,mm7			;unpack
	#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				or		ecx,ebx
	#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
	#endif
				psrlw	mm7,8-2
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
	#endif
				pmulhw	mm7,mm0				;colour modulate
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		ecx,edx
				shr		ecx,16-2
				push	ecx
	#endif
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				paddsw	mm0,mm2				;inc rgb
#endif
	#if SWIZZLE
				mov		edx,eax
				add		eax,DWORD PTR [ud]
	#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
	#endif
				packuswb mm7,mm7			;re-pack
	#if SWIZZLE
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
	#endif

#if ALPHA_MODE==ALPHA_ADD
				movd	mm6,[edi]
				paddusb	mm7,mm6
#elif ALPHA_MODE==ALPHA_TEST
				movq	mm5,mm7
				movq	mm6,[edi]
				pcmpgtb	mm7,[alpha_test_value]	;SIGNED compare!
				psrad	mm7,24				;propogate compare result down.
				pand	mm6,mm7
				pandn	mm7,mm5				;mask (mm7 = (mm5 AND ~mm7))
				por		mm7,mm6				;...and merge
#endif

	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				and		edx,DWORD PTR [vmask]
				or		edx,ecx
	#endif

				movd	[edi],mm7			;store pixel


// KLUDGE!
//				mov		DWORD PTR [edi],0x7f7f7f7f


	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				pop		ecx
				shr		edx,16-2
	#endif

				add		edi,4


#endif //#else //#if ALPHA_MODE==ALPHA_BLEND




#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				// Calculate r1g1b1, and double up the increments
				movq	mm1,mm0
				paddsw	mm1,mm2
				psllw	mm2,1
#endif





#if SWIZZLE
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				or		ecx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		ecx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		ecx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
//
//				mov		edx,eax
//				add		eax,DWORD PTR [ud]
//				or		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		edx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		edx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
#else
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				mov		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				and		ecx,DWORD PTR [umask]
//				and		edx,DWORD PTR [vmask]
//				or		ecx,edx
//				shr		ecx,16-2
//				push	ecx
//
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				mov		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				and		ecx,DWORD PTR [umask]
//				and		edx,DWORD PTR [vmask]
//				or		edx,ecx
//				pop		ecx
//				shr		edx,16-2
#endif
				
				movd	mm7,[esi+ecx]
				movd	mm6,[esi+edx]


setup_main_loop:

				cmp		edi,[last_dest]
				je		main_loop_done





hor_loop:
				// Map:
				// MM0: (all 8.8): xx,r0,g0,b0
				// MM1: (all 8.8): xx,r1,g1,b1		(not for blend+add)
				// MM2: (all 8.8): xx,r,g,b deltas	(not for blend+add)
				// eax: (16.16): u
				// ebx: (16.16): v
				// esi: texture base addr
				// edi: dest addr




#if ALPHA_MODE==ALPHA_BLEND
		
				punpcklbw mm7,mm7			;unpack
	#if SWIZZLE
				mov		ecx,eax
	#else
				mov		ecx,eax
	#endif
				punpcklbw mm6,mm6			;unpack
	#if SWIZZLE
				add		eax,DWORD PTR [ud]
	#else
				mov		edx,ebx
	#endif

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				psrlw	mm7,8-1
	#if SWIZZLE
				or		ecx,ebx
	#else
				and		ecx,DWORD PTR [umask]
	#endif
				psrlw	mm6,8-1
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
	#else
				and		edx,DWORD PTR [vmask]
	#endif
#else
				psrlw	mm7,8-2
	#if SWIZZLE
				or		ecx,ebx
	#else
				and		ecx,DWORD PTR [umask]
	#endif
				psrlw	mm6,8-2
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
	#else
				and		edx,DWORD PTR [vmask]
	#endif
#endif
				pmulhw	mm7,mm0				;colour modulate
	#if SWIZZLE
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
	#endif
				pmulhw	mm6,mm0				;colour modulate
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		ecx,edx
				add		eax,DWORD PTR [ud]
	#endif

				// Extract and replicate the alpha parts
				// Texel 0
				movq	mm2,mm7
	#if SWIZZLE
				mov		edx,eax
	#else
				shr		ecx,16-2
				add		ebx,DWORD PTR [vd]
	#endif
				psrlq	mm2,8+1				;half required value (creates a sign bit)
	#if SWIZZLE
				add		eax,DWORD PTR [ud]
	#else
				push	ecx
				mov		edx,ebx
	#endif
				pand	mm2,[alpha_mask]
	#if SWIZZLE
				or		edx,ebx
	#else
				mov		ecx,eax
				and		edx,DWORD PTR [vmask]
	#endif
				movq	mm5,mm2
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
	#else
				and		ecx,DWORD PTR [umask]
				add		eax,DWORD PTR [ud]
	#endif
				psrlq	mm5,16
	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				or		edx,ecx
				add		ebx,DWORD PTR [vd]
	#endif
				por		mm5,mm2
				psrlq	mm5,16
	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				shr		edx,16-2
				pop		ecx
	#endif
				por		mm5,mm2				;alpha channel replicated to r,g,b

				// Texel 0
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format
				
				movq	mm2,[edi]
				movq	mm1,mm2
				punpcklbw mm2,mm2			;unpack dest
				punpckhbw mm1,mm1			;unpack dest

				psrlw	mm2,8
				
				psubsw	mm7,mm2				;src-dest
				pmulhw	mm7,mm5				;alpha*(src-dest)



				paddsw	mm7,mm7				;*2
				paddsw	mm7,mm2				;dest0+alpha0*(src0-dest0)

				// Texel 1
				movq	mm2,mm6
				psrlq	mm2,8+1				;half required value (creates a sign bit)
				pand	mm2,[alpha_mask]
				movq	mm5,mm2
				psrlq	mm5,16
				por		mm5,mm2
				psrlq	mm5,16
				por		mm5,mm2				;alpha channel replicated to r,g,b

				// Texel 1
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format

				// Already unpacked above.
				psrlw	mm1,8
				
				psubsw	mm6,mm1				;src-dest
				pmulhw	mm6,mm5				;alpha*(src-dest)


				paddsw	mm6,mm6				;*2
				paddsw	mm6,mm1				;dest1+alpha1*(src1-dest1)

				// Pack.
				packuswb mm7,mm6			;re-pack

#else //#if ALPHA_MODE==ALPHA_BLEND

				punpcklbw mm7,mm7			;texel0
	#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
	#else
				mov		ecx,eax
				mov		edx,ebx
	#endif
				punpcklbw mm6,mm6			;texel1
	#if SWIZZLE
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
	#endif
				psrlw	mm7,8-2
	#if SWIZZLE
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				or		ecx,edx
				add		eax,DWORD PTR [ud]
	#endif
				psrlw	mm6,8-2
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				shr		ecx,16-2
				add		ebx,DWORD PTR [vd]
	#endif
				pmulhw	mm7,mm0
	#if SWIZZLE
				mov		edx,eax
				add		eax,DWORD PTR [ud]
	#else
				push	ecx
				mov		edx,ebx
	#endif
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				pmulhw	mm6,mm1
#else
				pmulhw	mm6,mm0
#endif
	#if SWIZZLE
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				mov		ecx,eax
				and		edx,DWORD PTR [vmask]
	#endif

				packuswb mm7,mm6
	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				and		ecx,DWORD PTR [umask]
				add		eax,DWORD PTR [ud]
	#endif

#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				paddsw	mm0,mm2
#endif
#if ALPHA_MODE==ALPHA_ADD
				paddusb	mm7,[edi]
#elif ALPHA_MODE==ALPHA_BLEND
#elif ALPHA_MODE==ALPHA_TEST
				movq	mm5,mm7
				movq	mm6,[edi]
				pcmpgtb	mm7,[alpha_test_value]	;SIGNED compare!
				psrad	mm7,24				;propogate compare result down.
				pand	mm6,mm7
				pandn	mm7,mm5				;mask (mm7 = (mm5 AND ~mm7))
				por		mm7,mm6				;...and merge
#endif
	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		edx,ecx
				add		ebx,DWORD PTR [vd]
	#endif
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				paddsw	mm1,mm2
#endif


	#if SWIZZLE
	#else
				shr		edx,16-2
				pop		ecx
	#endif



#endif //#else //#if ALPHA_MODE==ALPHA_BLEND

				movq	[edi],mm7

#if SWIZZLE
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				or		ecx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		ecx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		ecx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
//
//				mov		edx,eax
//				add		eax,DWORD PTR [ud]
//				or		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		edx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		edx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
#else
//				mov		ecx,eax
//				mov		edx,ebx
//				and		ecx,DWORD PTR [umask]
//				and		edx,DWORD PTR [vmask]
//				or		ecx,edx
//				add		eax,DWORD PTR [ud]
//				shr		ecx,16-2
//				add		ebx,DWORD PTR [vd]
//				push	ecx
//
//				mov		edx,ebx
//				mov		ecx,eax
//				and		edx,DWORD PTR [vmask]
//				and		ecx,DWORD PTR [umask]
//				add		eax,DWORD PTR [ud]
//				or		edx,ecx
//				add		ebx,DWORD PTR [vd]
//				shr		edx,16-2
//				pop		ecx
#endif
				movd	mm7,[esi+ecx]
				movd	mm6,[esi+edx]

				add edi,8

				cmp edi,[last_dest]
				jne	hor_loop

main_loop_done:

				// Do we need to do the last pixel?
				;Is the last pixel odd?
				mov		ecx,[tempx2]
				test	ecx,0x1
				jz		finished
				

				// Do the last pixel

#if ALPHA_MODE==ALPHA_BLEND
		
				punpcklbw mm7,mm7			;unpack

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				psrlw	mm7,8-1
#else
				psrlw	mm7,8-2
#endif
				pmulhw	mm7,mm0				;colour modulate

				movq	mm6,mm7

				psrlq	mm6,8+1				;half required value (creates a sign bit)
				pand	mm6,[alpha_mask]
				movq	mm5,mm6
				psrlq	mm5,16
				por		mm5,mm6
				psrlq	mm5,16
				por		mm5,mm6				;alpha channel replicated to r,g,b
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format
				// Remember that mm2 is spare - no gouard shading on alpha-blenders.
				
				movd	mm6,[edi]
				//mov		eax,0x10806050
				//movd	mm6,eax

				punpcklbw mm6,mm6			;unpack dest
				psrlw	mm6,8
				
				psubsw	mm7,mm6				;src-dest
				pmulhw	mm7,mm5				;alpha*(src-dest)
				paddsw	mm7,mm7				;*2
				paddsw	mm7,mm6				;dest+alpha*(src-dest)
				
				packuswb mm7,mm7			;re-pack


				//paddd	mm3,[uinc]			;inc u
				//paddd	mm4,[vinc]			;inc v
				//pand	mm3,[umask]			;fix swizzle
				//pand	mm4,[vmask]			;fix swizzle


#else //#if ALPHA_MODE==ALPHA_BLEND

				punpcklbw mm7,mm7
				psrlw	mm7,8-2
				pmulhw	mm7,mm0
				packuswb mm7,mm7
#if ALPHA_MODE==ALPHA_ADD
				movd	mm6,[edi]
				paddusb	mm7,mm6
#elif ALPHA_MODE==ALPHA_BLEND
#elif ALPHA_MODE==ALPHA_TEST
				movq	mm5,mm7
				movq	mm6,[edi]
				pcmpgtb	mm7,[alpha_test_value]	;SIGNED compare!
				psrad	mm7,24				;propogate compare result down.
				pand	mm6,mm7
				pandn	mm7,mm5				;mask (mm7 = (mm5 AND ~mm7))
				por		mm7,mm6				;...and merge
#endif

#endif //#else //#if ALPHA_MODE==ALPHA_BLEND

				movd	[edi],mm7

finished:

				pop edx
				pop ecx
				pop ebx
				pop eax
				pop edi
				pop esi

				// Clean up the MMX state
				emms

				}
			}
		}
	}
}





void SW_render_spans_eddie(void)
{
	SLONG i;
	SLONG x;

	ULONG  pixel;
	SLONG  pr;
	SLONG  pg;
	SLONG  pb;

	SW_Span *ss;

	for (i = SW_render_top; i < SW_render_bot; i++)
	{
		for (ss = SW_line[i]; ss; ss = ss->next)
		{
			ULONG	count = ss->x2 - ss->x1;

			if (!count)	continue;

			ULONG	gr = ((ss->g >> 8) << 16) | ((ss->r >> 8) & 0xFFFF);
			ULONG	zb = (ss->b >> 8) & 0xFFFF;

			ULONG	dgdr = ((ss->dg >> 8) << 16) | ((ss->dr >> 8) & 0xFFFF);
			ULONG	dzdb = (ss->db >> 8) & 0xFFFF;

			ULONG*	dest = SW_buffer/*_screen*/ + ss->x1 + i * SW_buffer_pitch;
			ULONG*	src = (ULONG*)ss->tga;

			if (ss->tga_size == 64)
			{
#define SHIFT	6
#define USH		(30 - SHIFT)
#define VSH		(30 - 2*SHIFT)
#define UMSK	((0xFFFFFFFFu >> (32 - SHIFT)) << 2)
#define VMSK	(UMSK << SHIFT)

				ULONG	u = ss->u << (16 - SHIFT);
				ULONG	v = ss->v << (16 - SHIFT);
				ULONG	du = ss->du << (16 - SHIFT);
				ULONG	dv = ss->dv << (16 - SHIFT);

				__asm
				{
					mov			eax,gr
					mov			ebx,zb
					movd		mm7,eax			// mm7 = 00|00|GG|RR
					movd		mm0,ebx			// mm0 = 00|00|00|BB

					mov			eax,dgdr
					mov			ebx,dzdb
					movd		mm6,eax			// mm6 = 00|00|dG|dR
					movd		mm1,ebx			// mm1 = 00|00|00|dB

					xor			eax,eax
					movd		mm5,eax			// mm5 = 00000000

					punpckldq	mm7,mm0			// mm7 = 00|BB|GG|RR
					punpckldq	mm6,mm1			// mm6 = 00|dB|dG|dR

					mov			esi,dest		// esi = destination pointer
					mov			eax,u			// eax = U
					mov			ebx,v			// ebx = V
					mov			ecx,count

myloop64:
					// get U + V * 64
					mov			edi,ebx			// edi = V (6:26)
					mov			edx,eax			// edx = U (6:26)

					shr			edi,VSH			// edi = VVVVVVxxxxxxxx
					shr			edx,USH			// edx = UUUUUUxx

					and			edi,VMSK		// edi = VVVVVV00000000
					and			edx,UMSK		// edx = UUUUUU00

					add			edi,src

					add			eax,[du]
					add			ebx,[dv]

					// load texel
					mov			edx,DWORD PTR [edx + edi]

					// get shade
					movq		mm1,mm7			// mm1 = 00|BB|GG|RR
					psrlw		mm1,8			// mm1 = 00|0B|0G|0R

					// expand 
					movd		mm0,edx
					punpcklbw	mm0,mm5			// mm0 = 00|0B|0G|0R

					// multiply
					pmullw		mm1,mm0			// mm1 = 00|BB|GG|RR shaded colour

					// increment colour
					paddw		mm7,mm6			// increment R,G,B

					// double with saturation
					paddusw		mm1,mm1

					// shift down
					psrlw		mm1,8			// mm1 = 00|0B|0G|0R

					// pack back to 00000GBR
					packuswb	mm1,mm5			// mm1 = 00000BGR

					// mov back to integer unit
					movd		edx,mm1

					mov			[esi],edx
					add			esi,4

					dec			ecx
					jnz			myloop64
				}
			}
			else if (ss->tga_size == 32)
			{
#undef SHIFT
#undef USH
#undef VSH
#undef UMSK
#undef VMSK
#define SHIFT	5
#define USH		(30 - SHIFT)
#define VSH		(30 - 2*SHIFT)
#define UMSK	((0xFFFFFFFFu >> (32 - SHIFT)) << 2)
#define VMSK	(UMSK << SHIFT)

				ULONG	u = ss->u << (16 - SHIFT);
				ULONG	v = ss->v << (16 - SHIFT);
				ULONG	du = ss->du << (16 - SHIFT);
				ULONG	dv = ss->dv << (16 - SHIFT);

				// the code below is identical to the code above

				__asm
				{
					mov			eax,gr
					mov			ebx,zb
					movd		mm7,eax			// mm7 = 00|00|GG|RR
					movd		mm0,ebx			// mm0 = 00|00|00|BB

					mov			eax,dgdr
					mov			ebx,dzdb
					movd		mm6,eax			// mm6 = 00|00|dG|dR
					movd		mm1,ebx			// mm1 = 00|00|00|dB

					xor			eax,eax
					movd		mm5,eax			// mm5 = 00000000

					punpckldq	mm7,mm0			// mm7 = 00|BB|GG|RR
					punpckldq	mm6,mm1			// mm6 = 00|dB|dG|dR

					mov			esi,dest		// esi = destination pointer
					mov			eax,u			// eax = U
					mov			ebx,v			// ebx = V
					mov			ecx,count

myloop32:
					// get U + V * 32
					mov			edi,ebx			// edi = V (6:26)
					mov			edx,eax			// edx = U (6:26)

					shr			edi,VSH			// edi = VVVVVVxxxxxxxx
					shr			edx,USH			// edx = UUUUUUxx

					and			edi,VMSK		// edi = VVVVVV00000000
					and			edx,UMSK		// edx = UUUUUU00

					add			edi,src

					add			eax,[du]
					add			ebx,[dv]

					// load texel
					mov			edx,DWORD PTR [edx + edi]

					// get shade
					movq		mm1,mm7			// mm1 = 00|BB|GG|RR
					psrlw		mm1,8			// mm1 = 00|0B|0G|0R

					// expand 
					movd		mm0,edx
					punpcklbw	mm0,mm5			// mm0 = 00|0B|0G|0R

					// multiply
					pmullw		mm1,mm0			// mm1 = 00|BB|GG|RR shaded colour

					// increment colour
					paddw		mm7,mm6			// increment R,G,B

					// double with saturation
					paddusw		mm1,mm1

					// shift down
					psrlw		mm1,8			// mm1 = 00|0B|0G|0R

					// pack back to 00000GBR
					packuswb	mm1,mm5			// mm1 = 00000BGR

					// mov back to integer unit
					movd		edx,mm1

					mov			[esi],edx
					add			esi,4

					dec			ecx
					jnz			myloop32
				}
			}
			else
			{
				ULONG	width = 0;
				ULONG	tgawidth = ss->tga_size;
				while (tgawidth > 1)
				{
					width++;
					tgawidth >>= 1;
				}

				ULONG	shift1 = 30 - width;
				ULONG	shift2 = 30 - 2 * width;
				ULONG	mask1 = (0xFFFFFFFFu >> (32 - width)) << 2;
				ULONG	mask2 = mask1 << width;

				ULONG	u = ss->u << (16 - width);
				ULONG	v = ss->v << (16 - width);
				ULONG	du = ss->du << (16 - width);
				ULONG	dv = ss->dv << (16 - width);

				// the code below has differences:
				// (1) doesn't use ecx to keep [count]
				// (2) // get U + V*64 is different code

				__asm
				{
					mov			eax,gr
					mov			ebx,zb
					movd		mm7,eax			// mm7 = 00|00|GG|RR
					movd		mm0,ebx			// mm0 = 00|00|00|BB

					mov			eax,dgdr
					mov			ebx,dzdb
					movd		mm6,eax			// mm6 = 00|00|dG|dR
					movd		mm1,ebx			// mm1 = 00|00|00|dB

					xor			eax,eax
					movd		mm5,eax			// mm5 = 00000000

					punpckldq	mm7,mm0			// mm7 = 00|BB|GG|RR
					punpckldq	mm6,mm1			// mm6 = 00|dB|dG|dR

					mov			esi,dest		// esi = destination pointer
					mov			eax,u			// eax = U
					mov			ebx,v			// ebx = V
					mov			edi,count

myloopany:
					mov			count,edi

					// get U + V * <n>
					mov			edi,ebx
					mov			ecx,[shift1]
					shr			edi,cl
					add			ebx,[dv]

					mov			edx,eax
					mov			ecx,[shift2]
					shr			edx,cl
					add			eax,[du]

					and			edi,[mask1]
					and			edx,[mask2]

					add			edi,src

					// load texel
					mov			edx,DWORD PTR [edx + edi]

					// get shade
					movq		mm1,mm7			// mm1 = 00|BB|GG|RR
					psrlw		mm1,8			// mm1 = 00|0B|0G|0R

					// expand 
					movd		mm0,edx
					punpcklbw	mm0,mm5			// mm0 = 00|0B|0G|0R

					// multiply
					pmullw		mm1,mm0			// mm1 = 00|BB|GG|RR shaded colour

					// increment colour
					paddw		mm7,mm6			// increment R,G,B

					// double with saturation
					paddusw		mm1,mm1

					// shift down
					psrlw		mm1,8			// mm1 = 00|0B|0G|0R

					// pack back to 00000GBR
					packuswb	mm1,mm5			// mm1 = 00000BGR

					// mov back to integer unit
					movd		edx,mm1

					mov			edi,count

					mov			[esi],edx
					add			esi,4

					dec			edi
					jnz			myloopany
				}
			}
		}
	}

	__asm
	{
		emms
	}
}



void SW_render_spans()
{
	static int which = 0;

	if (Keys[KB_P0])
	{
		Keys[KB_P0] = 0;

		which ^= 1;
	}

	if (which)
	{
		SW_render_spans_eddie();
	}
	else
	{
		SW_render_spans_tom();
	}
}



#else //#if !defined (TARGET_DC)

// DC stuff.



void SW_render_spans()
{
	// Stubbed - shouldn't be used anyway.
	ASSERT(FALSE);
}


#endif //#else //#if !defined (TARGET_DC)




void SW_add_triangle(
		SLONG x1, SLONG y1, SLONG z1, SLONG r1, SLONG g1, SLONG b1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG z2, SLONG r2, SLONG g2, SLONG b2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG z3, SLONG r3, SLONG g3, SLONG b3, SLONG u3, SLONG v3,
		SLONG page,
		SLONG alpha)
{
	if (!WITHIN(page, 0, SW_MAX_TEXTURES - 1))
	{
		//
		// Invalid texture page.
		//

		return;
	}

	SW_Texture *st;

	st = &SW_texture[page];

	if (st->data == NULL)
	{
		//
		// No texture loaded.
		//

		return;
	}

	extern SLONG GAMEMENU_background;

	if (GAMEMENU_background > 640 - 512)
	{
		if (page != POLY_PAGE_NEWFONT_INVERSE)
		{
			SLONG mul;
			
			mul   = GAMEMENU_background - (640 - 512);
			mul >>= mulshift;
			mul   = 256 - mul;

			r1 = r1 * mul >> 8;
			g1 = g1 * mul >> 8;
			b1 = b1 * mul >> 8;

			r2 = r2 * mul >> 8;
			g2 = g2 * mul >> 8;
			b2 = b2 * mul >> 8;

			r3 = r3 * mul >> 8;
			g3 = g3 * mul >> 8;
			b3 = b3 * mul >> 8;

			alpha = alpha * mul >> 8;
		}
	}

	TGA_Pixel *tga      = st->data;
	SLONG      tga_size = st->size;

	if (tga_size != 256)
	{
		if (tga_size == 64)
		{
			u1 >>= 2;
			v1 >>= 2;

			u2 >>= 2;
			v2 >>= 2;

			u3 >>= 2;
			v3 >>= 2;
		}
		else
		if (tga_size == 32)
		{
			u1 >>= 3;
			v1 >>= 3;

			u2 >>= 3;
			v2 >>= 3;

			u3 >>= 3;
			v3 >>= 3;
		}
		else
		if (tga_size == 16)
		{
			u1 >>= 4;
			v1 >>= 4;

			u2 >>= 4;
			v2 >>= 4;

			u3 >>= 4;
			v3 >>= 4;
		}
		else
		if (tga_size == 128)
		{
			u1 >>= 1;
			v1 >>= 1;

			u2 >>= 1;
			v2 >>= 1;

			u3 >>= 1;
			v3 >>= 1;
		}
		else
		{
			ASSERT(0);
		}
	}

	switch(SW_page[page])
	{
		case SW_PAGE_IGNORE:
			return;

		case SW_PAGE_NORMAL:
			
			//
			// Continue with the triangle draw.
			//

			break;

		case SW_PAGE_MASKED:

			//
			// Add to the masked buckets...
			//

			SW_add_masked_triangle(
				x1, y1, z1, r1, g1, b1, u1, v1,
				x2, y2, z2, r2, g2, b2, u2, v2,
				x3, y3, z3, r3, g3, b3, u3, v3,
				tga,
				tga_size);

			return;

		case SW_PAGE_ALPHA:

			//
			// Add the sprite.
			//

			{
				SLONG mode;
		
				mode = SW_MODE_ALPHA;

				if (SW_bucket[page])
				{
					z1 = SW_bucket[page];
					
					mode = SW_MODE_ALPHA_NOZ;
				}

				SW_add_alpha_sprite(
					x1, y1, u1, v1,
					x2, y2, u2, v2,
					x3, y3, u3, v3,
					z1, alpha, r1, g1, b1,
					tga,
					tga_size,
					mode);
			}

			return;

		case SW_PAGE_ADDITIVE:


			//
			// Add the sprite.
			//

			{
				SLONG mode;
		
				mode = SW_MODE_ADDITIVE;

				if (SW_bucket[page])
				{
					z1 = SW_bucket[page];
					
					mode = SW_MODE_ADDITIVE_NOZ;
				}

				SW_add_alpha_sprite(
					x1, y1, u1, v1,
					x2, y2, u2, v2,
					x3, y3, u3, v3,
					z1, alpha, r1 >> 1, g1 >> 1, b1 >> 1,
					tga,
					tga_size,
					mode);
			}

			return;

		default:
			ASSERT(0);
			break;
	}

	SLONG  xa, xb, za, zb, ra, rb, ga, gb, ba, bb, ua, ub, va, vb;
	SLONG  dax, dbx, daz, dbz, dar, dbr, dag, dbg, dab, dbb, dau, dbu, dav, dbv;
	SLONG  xmid, zmid, rmid, gmid, bmid, umid, vmid;
	SLONG  dz, dr, dg, db, du, dv;
	SLONG  z, r, g, b, u, v;
	SLONG  U,V;
	SLONG  xl, xr;
	SLONG  w;
	SLONG  x, y;
	SLONG  i, j;
	SLONG  shade;
	ULONG *dest;
	UBYTE  colour;

	SLONG  recip_y3y1;
	SLONG  recip_y2y1;

	SW_Span *ss;

	// Sort the points so that y1 <= y2 <= y3

	if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(z1,z2); SWAP(r1,r2); SWAP(g1,g2); SWAP(b1,b2); SWAP(u1,u2); SWAP(v1,v2);}
	if (y2>y3) {SWAP(x2,x3); SWAP(y2,y3); SWAP(z2,z3); SWAP(r2,r3); SWAP(g2,g3); SWAP(b2,b3); SWAP(u2,u3); SWAP(v2,v3);}
	if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(z1,z2); SWAP(r1,r2); SWAP(g1,g2); SWAP(b1,b2); SWAP(u1,u2); SWAP(v1,v2);}

	if (abs(y3 - y1) >= ALWAYS_HOW_MANY_RECIPS)
	{
		return;
	}

	recip_y3y1 = RECIP(y3 - y1);
	recip_y2y1 = RECIP(y2 - y1);

	if (y1 == y2)
	{
		if (y2 == y3) return;

		if (x1 < x2)
		{
			// The a dda runs down the left hand side...

			xa = x1 << 16;
			xb = x2 << 16;
			w  = xb - xa;

			if (w < (1 << 16))
			{
				dz = 0;
				dr = 0;
				db = 0;
				dg = 0;
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				w += 0x10000;

				dz = DIV16((z2 << 16) - (z1 << 16), w);
				dr = DIV16((r2 << 16) - (r1 << 16), w);
				dg = DIV16((g2 << 16) - (g1 << 16), w);
				db = DIV16((b2 << 16) - (b1 << 16), w);
				du = DIV16((u2 << 16) - (u1 << 16), w);
				dv = DIV16((v2 << 16) - (v1 << 16), w);
			}
			
			y = y1;

			dax = MUL16(x3 - x1 << 16, recip_y3y1);
			dbx = MUL16(x3 - x2 << 16, recip_y3y1);

			za  = z1 << 16;
			daz = MUL16(z3 - z1 << 16, recip_y3y1);
			ra  = r1 << 16;
			dar = MUL16(r3 - r1 << 16, recip_y3y1);
			ga  = g1 << 16;
			dag = MUL16(g3 - g1 << 16, recip_y3y1);
			ba  = b1 << 16;
			dab = MUL16(b3 - b1 << 16, recip_y3y1);
			ua  = u1 << 16;
			dau = MUL16(u3 - u1 << 16, recip_y3y1);
			va  = v1 << 16;
			dav = MUL16(v3 - v1 << 16, recip_y3y1);

			goto bottom_pointy_right;
		}
		else
		{

			// The a dda runs down the right hand side...

			xa = x1 << 16;
			xb = x2 << 16;
			w  = xa - xb;

			if (w < (1 << 16))
			{
				dz = 0;
				dr = 0;
				dg = 0;
				db = 0;
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				w += 0x10000;

				dz = DIV16((z1 << 16) - (z2 << 16), w);
				dr = DIV16((r1 << 16) - (r2 << 16), w);
				dg = DIV16((g1 << 16) - (g2 << 16), w);
				db = DIV16((b1 << 16) - (b2 << 16), w);
				du = DIV16((u1 << 16) - (u2 << 16), w);
				dv = DIV16((v1 << 16) - (v2 << 16), w);
			}

			y = y1;

			dax = MUL16(x3 - x1 << 16, recip_y3y1);
			dbx = MUL16(x3 - x2 << 16, recip_y3y1);

			zb  = z2 << 16;
			dbz = MUL16(z3 - z2 << 16, recip_y3y1);
			rb  = r2 << 16;
			dbr = MUL16(r3 - r2 << 16, recip_y3y1);
			gb  = g2 << 16;
			dbg = MUL16(g3 - g2 << 16, recip_y3y1);
			bb  = b2 << 16;
			dbb = MUL16(b3 - b2 << 16, recip_y3y1);
			ub  = u2 << 16;
			dbu = MUL16(u3 - u2 << 16, recip_y3y1);
			vb  = v2 << 16;
			dbv = MUL16(v3 - v2 << 16, recip_y3y1);

			goto bottom_pointy_left;
		}
	}

	// The a dda's run along the unbroken side of the triangle...
	// i.e. from y1 to y3.

	dax = MUL16(x3 - x1 << 16, recip_y3y1);
	dbx = MUL16(x2 - x1 << 16, recip_y2y1);

	// Is the a dda along the left side or the right side of the triangle?
	
	if (dax < dbx)
	{
		// The a dda runs along the left side of the triangle.

		xa = x1 << 16;
		xb = x1 << 16;

		za  = z1 << 16;
		daz = MUL16(z3 - z1 << 16, recip_y3y1);
		ra  = r1 << 16;
		dar = MUL16(r3 - r1 << 16, recip_y3y1);
		ga  = g1 << 16;
		dag = MUL16(g3 - g1 << 16, recip_y3y1);
		ba  = b1 << 16;
		dab = MUL16(b3 - b1 << 16, recip_y3y1);
		ua  = u1 << 16;
		dau = MUL16(u3 - u1 << 16, recip_y3y1);
		va  = v1 << 16;
		dav = MUL16(v3 - v1 << 16, recip_y3y1);

		// Work out xa, ua, and va along the middle horizontal crease.

		xmid = xa + dax * (y2 - y1);
		zmid = za + daz * (y2 - y1);
		rmid = ra + dar * (y2 - y1);
		gmid = ga + dag * (y2 - y1);
		bmid = ba + dab * (y2 - y1);
		umid = ua + dau * (y2 - y1);
		vmid = va + dav * (y2 - y1);

		// w is the width of the horizontal crease.

		w = (x2 << 16) - xmid;

		if (w < (1 << 16))
		{
			dz = 0;
			dr = 0;
			dg = 0;
			db = 0;
			du = 0;
			dv = 0;
		}
		else
		{
			// Work out du and dv as divides of two 16:16 fixed points.

			w += 0x10000;

			dz = DIV16((z2 << 16) - zmid, w);
			dr = DIV16((r2 << 16) - rmid, w);
			dg = DIV16((g2 << 16) - gmid, w);
			db = DIV16((b2 << 16) - bmid, w);
			du = DIV16((u2 << 16) - umid, w);
			dv = DIV16((v2 << 16) - vmid, w);
		}

		y = y1;

		while (y < y2)
		{
			if (y >= 0 && y <= SW_buffer_height - 1)
			{
				xl = xa >> 16;
				xr = xb >> 16;
				w  = xr - xl;

				z = za;
				r = ra;
				g = ga;
				b = ba;
				u = ua;
				v = va;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xa & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xa & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_insert_span(ss, y);
				}
			}

			xa += dax;
			xb += dbx;
			za += daz;
			ra += dar;
			ga += dag;
			ba += dab;
			ua += dau;
			va += dav;
			y  += 1;
		}

		// Reinitialise the b dda so it 'turns the corner' of the triangle.

		if (y2 == y3) return;

		xb  = x2 << 16;
		dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

	  bottom_pointy_right:

		while (y < y3)
		{
			if (y >= 0 && y <= SW_buffer_height - 1)
			{
				xl = xa >> 16;
				xr = xb >> 16;
				w  = xr - xl;

				z = za;
				r = ra;
				g = ga;
				b = ba;
				u = ua;
				v = va;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xa & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xa & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_insert_span(ss, y);
				}
			}

			xa += dax;
			xb += dbx;
			za += daz;
			ra += dar;
			ga += dag;
			ba += dab;
		   	ua += dau;
			va += dav;
			y  += 1;
		}

	}
	else
	{
		// The a dda runs along the right side of the triangle.

		xa = x1 << 16;
		xb = x1 << 16;

		// BILINEAR STUFF...

		za  = z1 << 16;
		daz = MUL16(z3 - z1 << 16, recip_y3y1);
		zb  = z1 << 16;
		dbz = MUL16(z2 - z1 << 16, recip_y2y1);

		ra  = r1 << 16;
		dar = MUL16(r3 - r1 << 16, recip_y3y1);
		rb  = r1 << 16;
		dbr = MUL16(r2 - r1 << 16, recip_y2y1);

		ga  = g1 << 16;
		dag = MUL16(g3 - g1 << 16, recip_y3y1);
		gb  = g1 << 16;
		dbg = MUL16(g2 - g1 << 16, recip_y2y1);

		ba  = b1 << 16;
		dab = MUL16(b3 - b1 << 16, recip_y3y1);
		bb  = b1 << 16;
		dbb = MUL16(b2 - b1 << 16, recip_y2y1);

		ua  = u1 << 16;
		dau = MUL16(u3 - u1 << 16, recip_y3y1);
		ub  = u1 << 16;
		dbu = MUL16(u2 - u1 << 16, recip_y2y1);

		va  = v1 << 16;
		dav = MUL16(v3 - v1 << 16, recip_y3y1);
		vb  = v1 << 16;
		dbv = MUL16(v2 - v1 << 16, recip_y2y1);


		// Work out xa, ua, and va along the middle horizontal crease.

		xmid = xa + dax * (y2 - y1);
		zmid = za + daz * (y2 - y1);
		rmid = ra + dar * (y2 - y1);
		gmid = ga + dag * (y2 - y1);
		bmid = ba + dab * (y2 - y1);
		umid = ua + dau * (y2 - y1);
		vmid = va + dav * (y2 - y1);

		// w is the width of the horizontal crease.

		w = xmid - (x2 << 16);

		if (w < (1 << 16))
		{
			dz = 0;
			dr = 0;
			dg = 0;
			db = 0;
			du = 0;
			dv = 0;
		}
		else
		{
			// Work out du and dv as divides of two 16:16 fixed points.

			w += 0x10000;

			dz = DIV16(zmid - (z2 << 16), w);
			dr = DIV16(rmid - (r2 << 16), w);
			dg = DIV16(gmid - (g2 << 16), w);
			db = DIV16(bmid - (b2 << 16), w);
			du = DIV16(umid - (u2 << 16), w);
			dv = DIV16(vmid - (v2 << 16), w);
		}

	  bottom_pointy_left:

		y  = y1;

		while (y < y2)
		{
			if (y >= 0 && y <= SW_buffer_height - 1)
			{
				xr = xa >> 16;
				xl = xb >> 16;
				w  = xr - xl;

				z = zb;
				r = rb;
				g = gb;
				b = bb;
				u = ub;
				v = vb;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xb & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xb & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_insert_span(ss, y);
				}
			}

			xa += dax;
			xb += dbx;
			zb += dbz;
			rb += dbr;
			gb += dbg;
			bb += dbb;
			ub += dbu;
			vb += dbv;
			y  += 1;
		}

		// Reinitialise the b dda so it 'turns the corner' of the triangle.

		if (y2 == y3) return;

		xb  = x2 << 16;
		dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

		zb  = z2 << 16;
		dbz = MUL16(z3 - z2 << 16, RECIP(y3 - y2));

		rb  = r2 << 16;
		dbr = MUL16(r3 - r2 << 16, RECIP(y3 - y2));

		gb  = g2 << 16;
		dbg = MUL16(g3 - g2 << 16, RECIP(y3 - y2));

		bb  = b2 << 16;
		dbb = MUL16(b3 - b2 << 16, RECIP(y3 - y2));

		ub  = u2 << 16;
		dbu = MUL16(u3 - u2 << 16, RECIP(y3 - y2));

		vb  = v2 << 16;
		dbv = MUL16(v3 - v2 << 16, RECIP(y3 - y2));

		while (y < y3)
		{
			if (y >= 0 && y <= SW_buffer_height - 1)
			{
				xr = xa >> 16;
				xl = xb >> 16;
				w  = xr - xl;

				z = zb;
				r = rb;
				g = gb;
				b = bb;
				u = ub;
				v = vb;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xb & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xb & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_insert_span(ss, y);
				}
			}

			xa += dax;
			xb += dbx;
			zb += dbz;
			rb += dbr;
			gb += dbg;
			bb += dbb;
			ub += dbu;
			vb += dbv;
			y  += 1;
		}
	}
}





void SW_draw_masked_triangle(
		SLONG x1, SLONG y1, SLONG z1, SLONG r1, SLONG g1, SLONG b1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG z2, SLONG r2, SLONG g2, SLONG b2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG z3, SLONG r3, SLONG g3, SLONG b3, SLONG u3, SLONG v3,
		TGA_Pixel *tga,
		SLONG      tga_size)
{
	SLONG  xa, xb, za, zb, ra, rb, ga, gb, ba, bb, ua, ub, va, vb;
	SLONG  dax, dbx, daz, dbz, dar, dbr, dag, dbg, dab, dbb, dau, dbu, dav, dbv;
	SLONG  xmid, zmid, rmid, gmid, bmid, umid, vmid;
	SLONG  dz, dr, dg, db, du, dv;
	SLONG  z, r, g, b, u, v;
	SLONG  U,V;
	SLONG  xl, xr;
	SLONG  w;
	SLONG  x, y;
	SLONG  i, j;
	SLONG  shade;
	ULONG *dest;
	UBYTE  colour;

	SLONG  recip_y3y1;
	SLONG  recip_y2y1;

	SW_Span *ss;

	// Sort the points so that y1 <= y2 <= y3

	if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(z1,z2); SWAP(r1,r2); SWAP(g1,g2); SWAP(b1,b2); SWAP(u1,u2); SWAP(v1,v2);}
	if (y2>y3) {SWAP(x2,x3); SWAP(y2,y3); SWAP(z2,z3); SWAP(r2,r3); SWAP(g2,g3); SWAP(b2,b3); SWAP(u2,u3); SWAP(v2,v3);}
	if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(z1,z2); SWAP(r1,r2); SWAP(g1,g2); SWAP(b1,b2); SWAP(u1,u2); SWAP(v1,v2);}

	if (abs(y3 - y1) >= ALWAYS_HOW_MANY_RECIPS)
	{
		return;
	}

	recip_y3y1 = RECIP(y3 - y1);
	recip_y2y1 = RECIP(y2 - y1);

	if (y1 == y2)
	{
		if (y2 == y3) return;

		if (x1 < x2)
		{
			// The a dda runs down the left hand side...

			xa = x1 << 16;
			xb = x2 << 16;
			w  = xb - xa;

			if (w < (1 << 16))
			{
				dz = 0;
				dr = 0;
				db = 0;
				dg = 0;
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				dz = DIV16((z2 << 16) - (z1 << 16), w);
				dr = DIV16((r2 << 16) - (r1 << 16), w);
				dg = DIV16((g2 << 16) - (g1 << 16), w);
				db = DIV16((b2 << 16) - (b1 << 16), w);
				du = DIV16((u2 << 16) - (u1 << 16), w);
				dv = DIV16((v2 << 16) - (v1 << 16), w);
			}
			
			y = y1;

			dax = MUL16(x3 - x1 << 16, recip_y3y1);
			dbx = MUL16(x3 - x2 << 16, recip_y3y1);

			za  = z1 << 16;
			daz = MUL16(z3 - z1 << 16, recip_y3y1);
			ra  = r1 << 16;
			dar = MUL16(r3 - r1 << 16, recip_y3y1);
			ga  = g1 << 16;
			dag = MUL16(g3 - g1 << 16, recip_y3y1);
			ba  = b1 << 16;
			dab = MUL16(b3 - b1 << 16, recip_y3y1);
			ua  = u1 << 16;
			dau = MUL16(u3 - u1 << 16, recip_y3y1);
			va  = v1 << 16;
			dav = MUL16(v3 - v1 << 16, recip_y3y1);

			goto bottom_pointy_right;
		}
		else
		{

			// The a dda runs down the right hand side...

			xa = x1 << 16;
			xb = x2 << 16;
			w  = xa - xb;

			if (w < (1 << 16))
			{
				dz = 0;
				dr = 0;
				dg = 0;
				db = 0;
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				dz = DIV16((z1 << 16) - (z2 << 16), w);
				dr = DIV16((r1 << 16) - (r2 << 16), w);
				dg = DIV16((g1 << 16) - (g2 << 16), w);
				db = DIV16((b1 << 16) - (b2 << 16), w);
				du = DIV16((u1 << 16) - (u2 << 16), w);
				dv = DIV16((v1 << 16) - (v2 << 16), w);
			}

			y = y1;

			dax = MUL16(x3 - x1 << 16, recip_y3y1);
			dbx = MUL16(x3 - x2 << 16, recip_y3y1);

			zb  = z2 << 16;
			dbz = MUL16(z3 - z2 << 16, recip_y3y1);
			rb  = r2 << 16;
			dbr = MUL16(r3 - r2 << 16, recip_y3y1);
			gb  = g2 << 16;
			dbg = MUL16(g3 - g2 << 16, recip_y3y1);
			bb  = b2 << 16;
			dbb = MUL16(b3 - b2 << 16, recip_y3y1);
			ub  = u2 << 16;
			dbu = MUL16(u3 - u2 << 16, recip_y3y1);
			vb  = v2 << 16;
			dbv = MUL16(v3 - v2 << 16, recip_y3y1);

			goto bottom_pointy_left;
		}
	}

	// The a dda's run along the unbroken side of the triangle...
	// i.e. from y1 to y3.

	dax = MUL16(x3 - x1 << 16, recip_y3y1);
	dbx = MUL16(x2 - x1 << 16, recip_y2y1);

	// Is the a dda along the left side or the right side of the triangle?
	
	if (dax < dbx)
	{
		// The a dda runs along the left side of the triangle.

		xa = x1 << 16;
		xb = x1 << 16;

		za  = z1 << 16;
		daz = MUL16(z3 - z1 << 16, recip_y3y1);
		ra  = r1 << 16;
		dar = MUL16(r3 - r1 << 16, recip_y3y1);
		ga  = g1 << 16;
		dag = MUL16(g3 - g1 << 16, recip_y3y1);
		ba  = b1 << 16;
		dab = MUL16(b3 - b1 << 16, recip_y3y1);
		ua  = u1 << 16;
		dau = MUL16(u3 - u1 << 16, recip_y3y1);
		va  = v1 << 16;
		dav = MUL16(v3 - v1 << 16, recip_y3y1);

		// Work out xa, ua, and va along the middle horizontal crease.

		xmid = xa + dax * (y2 - y1);
		zmid = za + daz * (y2 - y1);
		rmid = ra + dar * (y2 - y1);
		gmid = ga + dag * (y2 - y1);
		bmid = ba + dab * (y2 - y1);
		umid = ua + dau * (y2 - y1);
		vmid = va + dav * (y2 - y1);

		// w is the width of the horizontal crease.

		w = (x2 << 16) - xmid;

		if (w < (1 << 16))
		{
			dz = 0;
			dr = 0;
			dg = 0;
			db = 0;
			du = 0;
			dv = 0;
		}
		else
		{
			// Work out du and dv as divides of two 16:16 fixed points.

			dz = DIV16((z2 << 16) - zmid, w);
			dr = DIV16((r2 << 16) - rmid, w);
			dg = DIV16((g2 << 16) - gmid, w);
			db = DIV16((b2 << 16) - bmid, w);
			du = DIV16((u2 << 16) - umid, w);
			dv = DIV16((v2 << 16) - vmid, w);
		}

		y = y1;

		while (y < y2)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xl = xa >> 16;
				xr = xb >> 16;
				w  = xr - xl;

				z = za;
				r = ra;
				g = ga;
				b = ba;
				u = ua;
				v = va;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xa & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xa & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_clip_and_draw_span(ss, y, SW_MODE_MASKED);
				}
			}

			xa += dax;
			xb += dbx;
			za += daz;
			ra += dar;
			ga += dag;
			ba += dab;
			ua += dau;
			va += dav;
			y  += 1;
		}

		// Reinitialise the b dda so it 'turns the corner' of the triangle.

		if (y2 == y3) return;

		xb  = x2 << 16;
		dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

	  bottom_pointy_right:

		while (y < y3)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xl = xa >> 16;
				xr = xb >> 16;
				w  = xr - xl;

				z = za;
				r = ra;
				g = ga;
				b = ba;
				u = ua;
				v = va;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xa & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xa & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_clip_and_draw_span(ss, y, SW_MODE_MASKED);
				}
			}

			xa += dax;
			xb += dbx;
			za += daz;
			ra += dar;
			ga += dag;
			ba += dab;
		   	ua += dau;
			va += dav;
			y  += 1;
		}

	}
	else
	{
		// The a dda runs along the right side of the triangle.

		xa = x1 << 16;
		xb = x1 << 16;

		// BILINEAR STUFF...

		za  = z1 << 16;
		daz = MUL16(z3 - z1 << 16, recip_y3y1);
		zb  = z1 << 16;
		dbz = MUL16(z2 - z1 << 16, recip_y2y1);

		ra  = r1 << 16;
		dar = MUL16(r3 - r1 << 16, recip_y3y1);
		rb  = r1 << 16;
		dbr = MUL16(r2 - r1 << 16, recip_y2y1);

		ga  = g1 << 16;
		dag = MUL16(g3 - g1 << 16, recip_y3y1);
		gb  = g1 << 16;
		dbg = MUL16(g2 - g1 << 16, recip_y2y1);

		ba  = b1 << 16;
		dab = MUL16(b3 - b1 << 16, recip_y3y1);
		bb  = b1 << 16;
		dbb = MUL16(b2 - b1 << 16, recip_y2y1);

		ua  = u1 << 16;
		dau = MUL16(u3 - u1 << 16, recip_y3y1);
		ub  = u1 << 16;
		dbu = MUL16(u2 - u1 << 16, recip_y2y1);

		va  = v1 << 16;
		dav = MUL16(v3 - v1 << 16, recip_y3y1);
		vb  = v1 << 16;
		dbv = MUL16(v2 - v1 << 16, recip_y2y1);


		// Work out xa, ua, and va along the middle horizontal crease.

		xmid = xa + dax * (y2 - y1);
		zmid = za + daz * (y2 - y1);
		rmid = ra + dar * (y2 - y1);
		gmid = ga + dag * (y2 - y1);
		bmid = ba + dab * (y2 - y1);
		umid = ua + dau * (y2 - y1);
		vmid = va + dav * (y2 - y1);

		// w is the width of the horizontal crease.

		w = xmid - (x2 << 16);

		if (w < (1 << 16))
		{
			dz = 0;
			dr = 0;
			dg = 0;
			db = 0;
			du = 0;
			dv = 0;
		}
		else
		{
			// Work out du and dv as divides of two 16:16 fixed points.

			dz = DIV16(zmid - (z2 << 16), w);
			dr = DIV16(rmid - (r2 << 16), w);
			dg = DIV16(gmid - (g2 << 16), w);
			db = DIV16(bmid - (b2 << 16), w);
			du = DIV16(umid - (u2 << 16), w);
			dv = DIV16(vmid - (v2 << 16), w);
		}

	  bottom_pointy_left:

		y  = y1;

		while (y < y2)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xr = xa >> 16;
				xl = xb >> 16;
				w  = xr - xl;

				z = zb;
				r = rb;
				g = gb;
				b = bb;
				u = ub;
				v = vb;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xb & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xb & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_clip_and_draw_span(ss, y, SW_MODE_MASKED);
				}
			}

			xa += dax;
			xb += dbx;
			zb += dbz;
			rb += dbr;
			gb += dbg;
			bb += dbb;
			ub += dbu;
			vb += dbv;
			y  += 1;
		}

		// Reinitialise the b dda so it 'turns the corner' of the triangle.

		if (y2 == y3) return;

		xb  = x2 << 16;
		dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

		zb  = z2 << 16;
		dbz = MUL16(z3 - z2 << 16, RECIP(y3 - y2));

		rb  = r2 << 16;
		dbr = MUL16(r3 - r2 << 16, RECIP(y3 - y2));

		gb  = g2 << 16;
		dbg = MUL16(g3 - g2 << 16, RECIP(y3 - y2));

		bb  = b2 << 16;
		dbb = MUL16(b3 - b2 << 16, RECIP(y3 - y2));

		ub  = u2 << 16;
		dbu = MUL16(u3 - u2 << 16, RECIP(y3 - y2));

		vb  = v2 << 16;
		dbv = MUL16(v3 - v2 << 16, RECIP(y3 - y2));

		while (y < y3)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xr = xa >> 16;
				xl = xb >> 16;
				w  = xr - xl;

				z = zb;
				r = rb;
				g = gb;
				b = bb;
				u = ub;
				v = vb;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					z += dxl * dz;
					r += dxl * dr;
					g += dxl * dg;
					b += dxl * db;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->r        = r;
					ss->dr       = dr;
					ss->g        = g;
					ss->dg       = dg;
					ss->b        = b;
					ss->db       = db;
					ss->z        = z;
					ss->dz       = dz;
					ss->u        = u - MUL16((xb & 0xffff), du);;
					ss->du       = du;
					ss->v        = v - MUL16((xb & 0xffff), dv);;
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					SW_clip_and_draw_span(ss, y, SW_MODE_MASKED);
				}
			}

			xa += dax;
			xb += dbx;
			zb += dbz;
			rb += dbr;
			gb += dbg;
			bb += dbb;
			ub += dbu;
			vb += dbv;
			y  += 1;
		}
	}
}





void SW_draw_alpha_sprite(
		SLONG x1, SLONG y1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG u3, SLONG v3,
		SLONG wz, SLONG wa, SLONG wr, SLONG wg, SLONG wb,
		SLONG mode,
		TGA_Pixel *tga,
		SLONG      tga_size)
{
	SLONG  xa, xb, ua, ub, va, vb;
	SLONG  dax, dbx, dau, dbu, dav, dbv;
	SLONG  xmid, umid, vmid;
	SLONG  du, dv;
	SLONG  u, v;
	SLONG  U,V;
	SLONG  xl, xr;
	SLONG  w;
	SLONG  x, y;
	SLONG  i, j;
	SLONG  shade;
	ULONG *dest;
	UBYTE  colour;

	SLONG  recip_y3y1;
	SLONG  recip_y2y1;

	SW_Span *ss;

	//
	// Make the zed compatible with the other triangle draws.
	//

	wz -= 5;

	if (wz < 2)
	{
		wz = 2;
	}

	wz <<= 16;

	// Sort the points so that y1 <= y2 <= y3

	if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(u1,u2); SWAP(v1,v2);}
	if (y2>y3) {SWAP(x2,x3); SWAP(y2,y3); SWAP(u2,u3); SWAP(v2,v3);}
	if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(u1,u2); SWAP(v1,v2);}

	if (abs(y3 - y1) >= ALWAYS_HOW_MANY_RECIPS)
	{
		return;
	}

	recip_y3y1 = RECIP(y3 - y1);
	recip_y2y1 = RECIP(y2 - y1);

	if (y1 == y2)
	{
		if (y2 == y3) return;

		if (x1 < x2)
		{
			// The a dda runs down the left hand side...

			xa = x1 << 16;
			xb = x2 << 16;
			w  = xb - xa;

			if (w < (1 << 16))
			{
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				w += 0x10000;

				du = DIV16((u2 << 16) - (u1 << 16), w);
				dv = DIV16((v2 << 16) - (v1 << 16), w);
			}
			
			y = y1;

			dax = MUL16(x3 - x1 << 16, recip_y3y1);
			dbx = MUL16(x3 - x2 << 16, recip_y3y1);

			ua  = u1 << 16;
			dau = MUL16(u3 - u1 << 16, recip_y3y1);
			va  = v1 << 16;
			dav = MUL16(v3 - v1 << 16, recip_y3y1);

			goto bottom_pointy_right;
		}
		else
		{

			// The a dda runs down the right hand side...

			xa = x1 << 16;
			xb = x2 << 16;
			w  = xa - xb;

			if (w < (1 << 16))
			{
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				w += 0x10000;

				du = DIV16((u1 << 16) - (u2 << 16), w);
				dv = DIV16((v1 << 16) - (v2 << 16), w);
			}

			y = y1;

			dax = MUL16(x3 - x1 << 16, recip_y3y1);
			dbx = MUL16(x3 - x2 << 16, recip_y3y1);

			ub  = u2 << 16;
			dbu = MUL16(u3 - u2 << 16, recip_y3y1);
			vb  = v2 << 16;
			dbv = MUL16(v3 - v2 << 16, recip_y3y1);

			goto bottom_pointy_left;
		}
	}

	// The a dda's run along the unbroken side of the triangle...
	// i.e. from y1 to y3.

	dax = MUL16(x3 - x1 << 16, recip_y3y1);
	dbx = MUL16(x2 - x1 << 16, recip_y2y1);

	// Is the a dda along the left side or the right side of the triangle?
	
	if (dax < dbx)
	{
		// The a dda runs along the left side of the triangle.

		xa = x1 << 16;
		xb = x1 << 16;

		ua  = u1 << 16;
		dau = MUL16(u3 - u1 << 16, recip_y3y1);
		va  = v1 << 16;
		dav = MUL16(v3 - v1 << 16, recip_y3y1);

		// Work out xa, ua, and va along the middle horizontal crease.

		xmid = xa + dax * (y2 - y1);
		umid = ua + dau * (y2 - y1);
		vmid = va + dav * (y2 - y1);

		// w is the width of the horizontal crease.

		w = (x2 << 16) - xmid;

		if (w < (1 << 16))
		{
			du = 0;
			dv = 0;
		}
		else
		{
			// Work out du and dv as divides of two 16:16 fixed points.

			w += 0x10000;

			du = DIV16((u2 << 16) - umid, w);
			dv = DIV16((v2 << 16) - vmid, w);
		}

		y = y1;

		while (y < y2)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xl = xa >> 16;
				xr = xb >> 16;
				w  = xr - xl;

				u = ua;
				v = va;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->a        = wa;
					ss->r        = wr;
					ss->g        = wg;
					ss->b        = wb;
					ss->z        = wz;
					ss->u        = u - MUL16((xa & 0xffff), du);
					ss->du       = du;
					ss->v        = v - MUL16((xa & 0xffff), dv);
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					if (mode == SW_MODE_ALPHA_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ALPHA);
					}
					else
					if (mode == SW_MODE_ADDITIVE_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ADDITIVE);
					}
					else
					{
						SW_clip_and_draw_span(ss, y, mode);
					}
				}
			}

			xa += dax;
			xb += dbx;
			ua += dau;
			va += dav;
			y  += 1;
		}

		// Reinitialise the b dda so it 'turns the corner' of the triangle.

		if (y2 == y3) return;

		xb  = x2 << 16;
		dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

	  bottom_pointy_right:

		while (y < y3)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xl = xa >> 16;
				xr = xb >> 16;
				w  = xr - xl;

				u = ua;
				v = va;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->a        = wa;
					ss->r        = wr;
					ss->g        = wg;
					ss->b        = wb;
					ss->z        = wz;
					ss->u        = u - MUL16((xa & 0xffff), du);
					ss->du       = du;
					ss->v        = v - MUL16((xa & 0xffff), dv);
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					if (mode == SW_MODE_ALPHA_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ALPHA);
					}
					else
					if (mode == SW_MODE_ADDITIVE_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ADDITIVE);
					}
					else
					{
						SW_clip_and_draw_span(ss, y, mode);
					}
				}
			}

			xa += dax;
			xb += dbx;
		   	ua += dau;
			va += dav;
			y  += 1;
		}

	}
	else
	{
		// The a dda runs along the right side of the triangle.

		xa = x1 << 16;
		xb = x1 << 16;

		// BILINEAR STUFF...

		ua  = u1 << 16;
		dau = MUL16(u3 - u1 << 16, recip_y3y1);
		ub  = u1 << 16;
		dbu = MUL16(u2 - u1 << 16, recip_y2y1);

		va  = v1 << 16;
		dav = MUL16(v3 - v1 << 16, recip_y3y1);
		vb  = v1 << 16;
		dbv = MUL16(v2 - v1 << 16, recip_y2y1);


		// Work out xa, ua, and va along the middle horizontal crease.

		xmid = xa + dax * (y2 - y1);
		umid = ua + dau * (y2 - y1);
		vmid = va + dav * (y2 - y1);

		// w is the width of the horizontal crease.

		w = xmid - (x2 << 16);

		if (w < (1 << 16))
		{
			du = 0;
			dv = 0;
		}
		else
		{
			// Work out du and dv as divides of two 16:16 fixed points.

			w += 0x10000;

			du = DIV16(umid - (u2 << 16), w);
			dv = DIV16(vmid - (v2 << 16), w);
		}

	  bottom_pointy_left:

		y  = y1;

		while (y < y2)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xr = xa >> 16;
				xl = xb >> 16;
				w  = xr - xl;

				u = ub;
				v = vb;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->a        = wa;
					ss->r        = wr;
					ss->g        = wg;
					ss->b        = wb;
					ss->z        = wz;
					ss->u        = u - MUL16((xb & 0xffff), du);
					ss->du       = du;
					ss->v        = v - MUL16((xb & 0xffff), dv);
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					if (mode == SW_MODE_ALPHA_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ALPHA);
					}
					else
					if (mode == SW_MODE_ADDITIVE_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ADDITIVE);
					}
					else
					{
						SW_clip_and_draw_span(ss, y, mode);
					}
				}
			}

			xa += dax;
			xb += dbx;
			ub += dbu;
			vb += dbv;
			y  += 1;
		}

		// Reinitialise the b dda so it 'turns the corner' of the triangle.

		if (y2 == y3) return;

		xb  = x2 << 16;
		dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

		ub  = u2 << 16;
		dbu = MUL16(u3 - u2 << 16, RECIP(y3 - y2));

		vb  = v2 << 16;
		dbv = MUL16(v3 - v2 << 16, RECIP(y3 - y2));

		while (y < y3)
		{
			if (y >= SW_render_top && y < SW_render_bot)
			{
				xr = xa >> 16;
				xl = xb >> 16;
				w  = xr - xl;

				u = ub;
				v = vb;

				if (xl < 0)
				{
					SLONG dxl;

					dxl = 0 - xl;

					w -= dxl;
					u += dxl * du;
					v += dxl * dv;
					xl = 0;
				}

				dest = SW_PIXEL(xl, y);

				if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

				//
				// Create a new span.
				//

				if (w > 0)
				{
					ASSERT(WITHIN(SW_span_upto, 0, SW_MAX_SPANS - 1));

					ss = &SW_span[SW_span_upto++];

					ss->x1       = xl;
					ss->x2       = xl + w;
					ss->a        = wa;
					ss->r        = wr;
					ss->g        = wg;
					ss->b        = wb;
					ss->z        = wz;
					ss->u        = u - MUL16((xb & 0xffff), du);
					ss->du       = du;
					ss->v        = v - MUL16((xb & 0xffff), dv);
					ss->dv       = dv;
					ss->tga      = tga;
					ss->tga_size = tga_size;

					if (mode == SW_MODE_ALPHA_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ALPHA);
					}
					else
					if (mode == SW_MODE_ADDITIVE_NOZ)
					{
						SW_draw_span(ss, y, SW_MODE_ADDITIVE);
					}
					else
					{
						SW_clip_and_draw_span(ss, y, mode);
					}
				}
			}

			xa += dax;
			xb += dbx;
			ub += dbu;
			vb += dbv;
			y  += 1;
		}
	}
}




#if !defined(TARGET_DC)
//
// Returns the number of processor ticks since the processor was reset / 65536
//

ULONG SW_rdtsc(void)
{
	ULONG hi;
	ULONG lo;

	_asm
	{
		rdtsc
		mov		hi, edx
		mov		lo, eax
	}

	ULONG ans;

	ans  = lo >> 16;
	ans |= hi << 16;

	return ans;
}

#else //#if !defined(TARGET_DC)

ULONG SW_rdtsc(void)
{
	return 1;
}

#endif //#else //#if !defined(TARGET_DC)


SLONG SW_tick1;
SLONG SW_tick2;

void SW_render()
{
	extern SLONG EWAY_stop_player_moving(void);

	if (EWAY_stop_player_moving())
	{
		SW_render_top = 80  * SW_buffer_height / 480;
		SW_render_bot = 400 * SW_buffer_height / 480;
	}
	else
	{
		SW_render_top = 0;
		SW_render_bot = SW_buffer_height;
	}

	SW_tick1 = SW_rdtsc();

	SW_render_spans();

	SW_tick2 = SW_rdtsc();
	
	//
	// Now render the buckets!
	//

	SLONG i;
	SLONG mode;

	SW_Masked *sm;
	SW_Alpha  *sa;

	for (i = SW_MAX_BUCKETS - 1; i >= 0; i--)
	{
		if (i == 2)
		{
			//
			// The first few buckets...
			//

			SW_render_top = 0;
			SW_render_bot = SW_buffer_height;
		}

		for (sm = SW_masked_bucket[i]; sm; sm = sm->next)
		{
			SW_draw_masked_triangle(
				sm->x1, sm->y1, sm->z1, sm->r1, sm->g1, sm->b1, sm->u1, sm->v1,
				sm->x2, sm->y2, sm->z2, sm->r2, sm->g2, sm->b2, sm->u2, sm->v2,
				sm->x3, sm->y3, sm->z3, sm->r3, sm->g3, sm->b3, sm->u3, sm->v3,
				sm->tga,
				sm->tga_size);
		}

		for (sa = SW_alpha_bucket[i]; sa; sa = sa->next)
		{
			SW_draw_alpha_sprite(
				sa->x1, sa->y1, sa->u1, sa->v1,
				sa->x2, sa->y2, sa->u2, sa->v2,
				sa->x3, sa->y3, sa->u3, sa->v3,
				sa->wz, sa->wa, sa->wr, sa->wg, sa->wb,
				sa->mode,
				sa->tga,
				sa->tga_size);
		}
	}
}




void SW_test_triangle(
		SLONG x1, SLONG y1, SLONG z1, SLONG s1, SLONG u1, SLONG v1,
		SLONG x2, SLONG y2, SLONG z2, SLONG s2, SLONG u2, SLONG v2,
		SLONG x3, SLONG y3, SLONG z3, SLONG s3, SLONG u3, SLONG v3,
		SLONG page)
{
	if (!WITHIN(page, 0, SW_MAX_TEXTURES - 1))
	{
		//
		// Invalid texture page.
		//

		return;
	}

	SW_Texture *st;

	st = &SW_texture[page];

	if (st->data == NULL)
	{
		//
		// No texture loaded.
		//

		return;
	}

	TGA_Pixel *tga      = st->data;
	SLONG      tga_size = st->size;

	if (tga_size != 256)
	{
		if (tga_size == 64)
		{
			u1 >>= 2;
			v1 >>= 2;

			u2 >>= 2;
			v2 >>= 2;

			u3 >>= 2;
			v3 >>= 2;
		}
		else
		if (tga_size == 32)
		{
			u1 >>= 3;
			v1 >>= 3;

			u2 >>= 3;
			v2 >>= 3;

			u3 >>= 3;
			v3 >>= 3;
		}
		else
		if (tga_size == 128)
		{
			u1 >>= 1;
			v1 >>= 1;

			u2 >>= 1;
			v2 >>= 1;

			u3 >>= 1;
			v3 >>= 1;
		}
	}

	//
	// This is code ripped from the software testbed.
	//

	{
		SLONG  xa, xb, za, zb, sa, sb, ua, ub, va, vb;
		SLONG  dax, dbx, daz, dbz, das, dbs, dau, dbu, dav, dbv;
		SLONG  xmid, zmid, smid, umid, vmid;
		SLONG  dz, ds, du, dv;
		SLONG  z, s, u, v;
		SLONG  U,V;
		SLONG  xl, xr;
		SLONG  w;
		SLONG  x, y;
		SLONG  i, j;
		SLONG  r,g,b;
		SLONG  shade;
		ULONG *dest;
		UBYTE  colour;

		SLONG  recip_y3y1;
		SLONG  recip_y2y1;

		// Sort the points so that y1 <= y2 <= y3

		if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(z1,z2); SWAP(s1,s2); SWAP(u1,u2); SWAP(v1,v2);}
		if (y2>y3) {SWAP(x2,x3); SWAP(y2,y3); SWAP(z2,z3); SWAP(s2,s3); SWAP(u2,u3); SWAP(v2,v3);}
		if (y1>y2) {SWAP(x1,x2); SWAP(y1,y2); SWAP(z1,z2); SWAP(s1,s2); SWAP(u1,u2); SWAP(v1,v2);}

		recip_y3y1 = RECIP(y3 - y1);
		recip_y2y1 = RECIP(y2 - y1);

		if (y1 == y2)
		{
			if (y2 == y3) return;

			if (x1 < x2)
			{
				// The a dda runs down the left hand side...

				xa = x1 << 16;
				xb = x2 << 16;
				w  = xb - xa;

				if (w < (1 << 16))
				{
					dz = 0;
					ds = 0;
					du = 0;
					dv = 0;
				}
				else
				{
					// Work out du and dv as divides of two 16:16 fixed points.

					dz = DIV16((z2 << 16) - (z1 << 16), w);
					ds = DIV16((s2 << 16) - (s1 << 16), w);
					du = DIV16((u2 << 16) - (u1 << 16), w);
					dv = DIV16((v2 << 16) - (v1 << 16), w);
				}
				
				y = y1;

				dax = MUL16(x3 - x1 << 16, recip_y3y1);
				dbx = MUL16(x3 - x2 << 16, recip_y3y1);

				za  = z1 << 16;
				daz = MUL16(z3 - z1 << 16, recip_y3y1);
				sa  = s1 << 16;
				das = MUL16(s3 - s1 << 16, recip_y3y1);
				ua  = u1 << 16;
				dau = MUL16(u3 - u1 << 16, recip_y3y1);
				va  = v1 << 16;
				dav = MUL16(v3 - v1 << 16, recip_y3y1);

				goto bottom_pointy_right;
			}
			else
			{

				// The a dda runs down the right hand side...

				xa = x1 << 16;
				xb = x2 << 16;
				w  = xa - xb;

				if (w < (1 << 16))
				{
					dz = 0;
					ds = 0;
					du = 0;
					dv = 0;
				}
				else
				{
					// Work out du and dv as divides of two 16:16 fixed points.

					dz = DIV16((z1 << 16) - (z2 << 16), w);
					ds = DIV16((s1 << 16) - (s2 << 16), w);
					du = DIV16((u1 << 16) - (u2 << 16), w);
					dv = DIV16((v1 << 16) - (v2 << 16), w);
				}

				y = y1;

				dax = MUL16(x3 - x1 << 16, recip_y3y1);
				dbx = MUL16(x3 - x2 << 16, recip_y3y1);

				zb  = z2 << 16;
				dbz = MUL16(z3 - z2 << 16, recip_y3y1);
				sb  = s2 << 16;
				dbs = MUL16(s3 - s2 << 16, recip_y3y1);
				ub  = u2 << 16;
				dbu = MUL16(u3 - u2 << 16, recip_y3y1);
				vb  = v2 << 16;
				dbv = MUL16(v3 - v2 << 16, recip_y3y1);

				goto bottom_pointy_left;
			}
		}

		// The a dda's run along the unbroken side of the triangle...
		// i.e. from y1 to y3.

		dax = MUL16(x3 - x1 << 16, recip_y3y1);
		dbx = MUL16(x2 - x1 << 16, recip_y2y1);

		// Is the a dda along the left side or the right side of the triangle?
		
		if (dax < dbx)
		{
			// The a dda runs along the left side of the triangle.

			xa = x1 << 16;
			xb = x1 << 16;

			za  = z1 << 16;
			daz = MUL16(z3 - z1 << 16, recip_y3y1);
			sa  = s1 << 16;
			das = MUL16(s3 - s1 << 16, recip_y3y1);
			ua  = u1 << 16;
			dau = MUL16(u3 - u1 << 16, recip_y3y1);
			va  = v1 << 16;
			dav = MUL16(v3 - v1 << 16, recip_y3y1);

			// Work out xa, ua, and va along the middle horizontal crease.

			xmid = xa + dax * (y2 - y1);
			zmid = za + daz * (y2 - y1);
			smid = sa + das * (y2 - y1);
			umid = ua + dau * (y2 - y1);
			vmid = va + dav * (y2 - y1);

			// w is the width of the horizontal crease.

			w = (x2 << 16) - xmid;

			if (w < (1 << 16))
			{
				dz = 0;
				ds = 0;
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				dz = DIV16((z2 << 16) - smid, w);
				ds = DIV16((s2 << 16) - smid, w);
				du = DIV16((u2 << 16) - umid, w);
				dv = DIV16((v2 << 16) - vmid, w);
			}

			y = y1;

			while (y < y2)
			{
				if (y >= 0 && y <= SW_buffer_height - 1)
				{
					xl = xa >> 16;
					xr = xb >> 16;
					w  = xr - xl;

					z = za;
					s = sa;
					u = ua;
					v = va;

					if (xl < 0)
					{
						SLONG dxl;

						dxl = 0 - xl;

						w -= dxl;
						z += dxl * dz;
						s += dxl * ds;
						u += dxl * du;
						v += dxl * dv;
						xl = 0;
					}

					dest = SW_PIXEL(xl, y);

					if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

					for (x = xl; w-- > 0; x++)
					{
						shade  = s >> 16;
						U      = u >> 16;
						V      = v >> 16;
						U     &= tga_size - 1;
						V     &= tga_size - 1;

						ASSERT(WITHIN(shade, 0, 255));

						r = tga[U + V * tga_size].red;
						g = tga[U + V * tga_size].green;
						b = tga[U + V * tga_size].blue;

						r = r * shade >> 8;
						g = g * shade >> 8;
						b = b * shade >> 8;

						*dest = (r << 16) | (g << 8) | b;

						z    += dz;
						s    += ds;
						u    += du;
						v    += dv;
						dest += 1;
					}
				}

				xa += dax;
				xb += dbx;
				za += daz;
				sa += das;
				ua += dau;
				va += dav;
				y  += 1;
			}

			// Reinitialise the b dda so it 'turns the corner' of the triangle.

			if (y2 == y3) return;

			xb  = x2 << 16;
			dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

		  bottom_pointy_right:

			while (y < y3)
			{
				if (y >= 0 && y <= SW_buffer_height - 1)
				{
					xl = xa >> 16;
					xr = xb >> 16;
					w  = xr - xl;

					z = za;
					s = sa;
					u = ua;
					v = va;

					if (xl < 0)
					{
						SLONG dxl;

						dxl = 0 - xl;

						w -= dxl;
						z += dxl * dz;
						s += dxl * ds;
						u += dxl * du;
						v += dxl * dv;
						xl = 0;
					}

					dest = SW_PIXEL(xl, y);

					if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

					for (x = xl; w-- > 0; x++)
					{
						shade  = s >> 16;
						U      = u >> 16;
						V      = v >> 16;
						U     &= tga_size - 1;
						V     &= tga_size - 1;

						ASSERT(WITHIN(shade, 0, 255));

						r = tga[U + V * tga_size].red;
						g = tga[U + V * tga_size].green;
						b = tga[U + V * tga_size].blue;

						r = r * shade >> 8;
						g = g * shade >> 8;
						b = b * shade >> 8;

						*dest = (r << 16) | (g << 8) | b;

						z    += dz;
						s    += ds;
						u    += du;
						v    += dv;
						dest += 1;
					}
				}

				xa += dax;
				xb += dbx;
				za += daz;
				sa += das;
		   		ua += dau;
				va += dav;
				y  += 1;
			}

		}
		else
		{
			// The a dda runs along the right side of the triangle.

			xa = x1 << 16;
			xb = x1 << 16;

			// BILINEAR STUFF...

			za  = z1 << 16;
			daz = MUL16(z3 - z1 << 16, recip_y3y1);
			zb  = z1 << 16;
			dbz = MUL16(z2 - z1 << 16, recip_y2y1);

			sa  = s1 << 16;
			das = MUL16(s3 - s1 << 16, recip_y3y1);
			sb  = s1 << 16;
			dbs = MUL16(s2 - s1 << 16, recip_y2y1);

			ua  = u1 << 16;
			dau = MUL16(u3 - u1 << 16, recip_y3y1);
			ub  = u1 << 16;
			dbu = MUL16(u2 - u1 << 16, recip_y2y1);

			va  = v1 << 16;
			dav = MUL16(v3 - v1 << 16, recip_y3y1);
			vb  = v1 << 16;
			dbv = MUL16(v2 - v1 << 16, recip_y2y1);


			// Work out xa, ua, and va along the middle horizontal crease.

			xmid = xa + dax * (y2 - y1);
			zmid = za + daz * (y2 - y1);
			smid = sa + das * (y2 - y1);
			umid = ua + dau * (y2 - y1);
			vmid = va + dav * (y2 - y1);

			// w is the width of the horizontal crease.

			w = xmid - (x2 << 16);

			if (w < (1 << 16))
			{
				dz = 0;
				ds = 0;
				du = 0;
				dv = 0;
			}
			else
			{
				// Work out du and dv as divides of two 16:16 fixed points.

				dz = DIV16(zmid - (z2 << 16), w);
				ds = DIV16(smid - (s2 << 16), w);
				du = DIV16(umid - (u2 << 16), w);
				dv = DIV16(vmid - (v2 << 16), w);
			}

		  bottom_pointy_left:

			y  = y1;

			while (y < y2)
			{
				if (y >= 0 && y <= SW_buffer_height - 1)
				{
					xr = xa >> 16;
					xl = xb >> 16;
					w  = xr - xl;

					z = zb;
					s = sb;
					u = ub;
					v = vb;

					if (xl < 0)
					{
						SLONG dxl;

						dxl = 0 - xl;

						w -= dxl;
						z += dxl * dz;
						s += dxl * ds;
						u += dxl * du;
						v += dxl * dv;
						xl = 0;
					}

					dest = SW_PIXEL(xl, y);

					if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

					for (x = xl; w-- > 0; x++)
					{
						shade  = s >> 16;
						U      = u >> 16;
						V      = v >> 16;
						U     &= tga_size - 1;
						V     &= tga_size - 1;

						ASSERT(WITHIN(shade, 0, 255));

						r = tga[U + V * tga_size].red;
						g = tga[U + V * tga_size].green;
						b = tga[U + V * tga_size].blue;

						r = r * shade >> 8;
						g = g * shade >> 8;
						b = b * shade >> 8;

						*dest = (r << 16) | (g << 8) | b;

						z    += dz;
						s    += ds;
						u    += du;
						v    += dv;
						dest += 1;
					}
				}

				xa += dax;
				xb += dbx;
				zb += dbz;
				sb += dbs;
				ub += dbu;
				vb += dbv;
				y  += 1;
			}

			// Reinitialise the b dda so it 'turns the corner' of the triangle.

			if (y2 == y3) return;

			xb  = x2 << 16;
			dbx = MUL16(x3 - x2 << 16, RECIP(y3 - y2));

			zb  = z2 << 16;
			dbz = MUL16(z3 - z2 << 16, RECIP(y3 - y2));

			sb  = s2 << 16;
			dbs = MUL16(s3 - s2 << 16, RECIP(y3 - y2));

			ub  = u2 << 16;
			dbu = MUL16(u3 - u2 << 16, RECIP(y3 - y2));

			vb  = v2 << 16;
			dbv = MUL16(v3 - v2 << 16, RECIP(y3 - y2));

			while (y < y3)
			{
				if (y >= 0 && y <= SW_buffer_height - 1)
				{
					xr = xa >> 16;
					xl = xb >> 16;
					w  = xr - xl;

					z = zb;
					s = sb;
					u = ub;
					v = vb;

					if (xl < 0)
					{
						SLONG dxl;

						dxl = 0 - xl;

						w -= dxl;
						z += dxl * dz;
						s += dxl * ds;
						u += dxl * du;
						v += dxl * dv;
						xl = 0;
					}

					dest = SW_PIXEL(xl, y);

					if (xl + w > SW_buffer_width - 1 + 1) {w = SW_buffer_width - 1 + 1 - xl;}

					for (x = xl; w-- > 0; x++)
					{
						shade  = s >> 16;
						U      = u >> 16;
						V      = v >> 16;
						U     &= tga_size - 1;
						V     &= tga_size - 1;

						r = tga[U + V * tga_size].red;
						g = tga[U + V * tga_size].green;
						b = tga[U + V * tga_size].blue;

						ASSERT(WITHIN(shade, 0, 255));

						r = r * shade >> 8;
						g = g * shade >> 8;
						b = b * shade >> 8;

						*dest = (r << 16) | (g << 8) | b;

						z    += dz;
						s    += ds;
						u    += du;
						v    += dv;
						dest += 1;
					}
				}

				xa += dax;
				xb += dbx;
				zb += dbz;
				sb += dbs;
				ub += dbu;
				vb += dbv;
				y  += 1;
			}
		}
	}
}










void SW_copy_to_bb()
{
	SLONG  x;
	SLONG  y;
	ULONG *source;
	ULONG *dest_l;
	UWORD *dest_w;

	//
	// Render the scene anc copy over the back buffer.
	//

	the_display.screen_lock();

	if (the_display.screen)
	{
		SLONG width  = MIN(the_display.screen_width,  SW_buffer_width);
		SLONG height = MIN(the_display.screen_height, SW_buffer_height);

		if (the_display.screen_bbp == 32)
		{
			//
			// This should be easy!
			//

			SW_buffer       = (ULONG *) the_display.screen;
			SW_buffer_pitch = the_display.screen_pitch >> 2;

			SW_render();

			/*

			for (y = 0; y < height; y++)
			{
				source = SW_PIXEL(0,y);
				dest_l = (ULONG *) (the_display.screen + y * the_display.screen_pitch);

				memcpy(dest_l, source, 4 * width);
			}

			*/
		}
		else
		if (the_display.screen_bbp == 16)
		{
			SW_buffer = SW_buffer_memory;

			memset(SW_buffer_memory, 0, SW_buffer_width * SW_buffer_height * sizeof(ULONG));

			SW_render();

			for (y = 0; y < height; y++)
			{
				source = SW_PIXEL(0,y);
				dest_w = (UWORD *) (the_display.screen + y * the_display.screen_pitch);

				for (x = 0; x < width; x++)
				{
				   *dest_w = the_display.GetFormattedPixel(*source >> 16, (*source >> 8) & 0xff, *source & 0xff);

					dest_w += 1;
					source += 1;
				}
			}
		}

		the_display.screen_unlock();
	}
}



#endif //#ifndef TARGET_DC


