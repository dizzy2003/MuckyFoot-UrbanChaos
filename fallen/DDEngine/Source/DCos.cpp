#if 0

//
// Sprite drawing routines for the DC credits sequence.
//

#include <ddlib.h>


typedef struct dcos_buffer
{
	
	

} DCOS_Buffer;


//
// Add sprites to the buffer.
//

void DCOS_buffer_add_sprite(
		DCOS_Buffer *ob,
		float x1, float y1,
		float x2, float y2,
		float u1 = 0.0F, float v1 = 0.0F,
		float u2 = 1.0F, float v2 = 1.0F,
		float z  = 0.0F,
		ULONG colour   = 0x00ffffff,
		ULONG specular = 0x00000000,
		ULONG fade     = 0);


//
// Draws the given buffer.
//

#define DCOS_DRAW_NORMAL 0
#define DCOS_DRAW_ADD    1

void DCOS_buffer_draw(OS_Buffer *ob, D3DTexture *texture, ULONG draw = DCOS_DRAW_NORMAL);




#endif

