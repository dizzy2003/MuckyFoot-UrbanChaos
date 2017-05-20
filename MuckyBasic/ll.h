//
// An interface between the low level D3D drawing and
// the virtual machine.
//

#ifndef _LL_
#define _LL_

#include "os.h"





//
// A texture.
//

typedef struct
{
	OS_Texture *ot;
	SLONG       width;
	SLONG       height;
	SLONG       ref_count;	// This is up to you! It is set to 1 when you create the texture.

} LL_Texture;



//
// The different types of points we can pass to LL_create_buffer.
//

typedef struct
{
	float x;
	float y;
	float z;
	float rhw;
	ULONG colour;
	ULONG specular;
	float u;
	float v;

} LL_Tlvert;

typedef struct
{
	float x;
	float y;
	float z;
	ULONG reserved;
	ULONG colour;
	ULONG specular;
	float u;
	float v;

} LL_Lvert;

typedef struct
{
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	float u;
	float v;

} LL_Vert;



//
// A buffer.
//

#define LL_BUFFER_TYPE_TLV 0		// Transformed and lit
#define LL_BUFFER_TYPE_LV  0		// Just lit
#define LL_BUFFER_TYPE_V   0		// Neither lit nor transformed

typedef struct
{
	SLONG  type;

	union
	{
		void      *vert_data;
		LL_Tlvert *vert_tl;
		LL_Lvert  *vert_l;
		LL_Vert   *vert;
	};

	UWORD *index;
	SLONG  num_verts;
	SLONG  num_indices;
	SLONG  ref_count;	// This is up to you! It is set to 1 when you create the buffer.

} LL_Buffer;


//
// A sound
//

typedef struct
{
	OS_Sound *os;
	SLONG     ref_count;

} LL_Sound;




//
// Creates a texture from a file. Returns NULL on failure.
//

LL_Texture *LL_create_texture(CBYTE *fname);


//
// Deletes the texture.
//

void LL_free_texture(LL_Texture *lt);



//
// Loads a sound from a file. Returns NULL on failure.
// 

LL_Sound *LL_create_sound(CBYTE *fname);

//
// Deletes the sound.
//

void LL_free_sound(LL_Sound *ls);





//
// Creates a buffer. The vertex and index memory should be
// allocated with the MEM module.
//

LL_Buffer *LL_create_buffer(
				SLONG  type,
				void  *vert,
				SLONG  num_verts,
				UWORD *index,			// If NULL then this is a LIST buffer rather than an INDEXED buffer.
				SLONG  num_indices);

//
// Deletes the buffer.
//

void LL_free_buffer(LL_Buffer *lb);




//
// The renderstates you can OR together.
//

#define LL_RS_NORMAL      OS_DRAW_NORMAL
#define LL_RS_ADD         OS_DRAW_ADD
#define LL_RS_MULTIPLY    OS_DRAW_MULTIPLY
#define LL_RS_CLAMP       OS_DRAW_CLAMP
#define LL_RS_DECAL       OS_DRAW_DECAL
#define LL_RS_TRANSPARENT OS_DRAW_TRANSPARENT
#define LL_RS_DOUBLESIDED OS_DRAW_DOUBLESIDED
#define LL_RS_NOZWRITE    OS_DRAW_NOZWRITE
#define LL_RS_ALPHAREF    OS_DRAW_ALPHAREF
#define LL_RS_ZREVERSE    OS_DRAW_ZREVERSE
#define LL_RS_ZALWAYS     OS_DRAW_ZALWAYS
#define LL_RS_CULLREVERSE OS_DRAW_CULLREVERSE
#define LL_RS_NODITHER    OS_DRAW_NODITHER
#define LL_RS_ALPHABLEND  OS_DRAW_ALPHABLEND
#define LL_RS_TEX_NONE    OS_DRAW_TEX_NONE
#define LL_RS_NOFILTER    OS_DRAW_NOFILTER
#define LL_RS_MULBYONE    OS_DRAW_MULBYONE




//
// Draws a buffer
//

void LL_draw_buffer(
		LL_Buffer  *lb,
		LL_Texture *lt,		// NULL => Draw untextured
		ULONG       rs);	// The LL_RS_* renderstates ORed together.



void LL_cls(ULONG colour, float z);




//
// Flips the screen- actually does a blit or a FLIP depending
// on whether we are running in a window or not.
//

void LL_flip(void);




#endif
