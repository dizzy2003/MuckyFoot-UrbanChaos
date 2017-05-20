//
// An interface between the low level D3D drawing and
// the virtual machine.
//

#include "always.h"
#include "ll.h"
#include "mem.h"
#include "os.h"





//
// All the textures.
//

#define LL_MAX_TEXTURES 4096

LL_Texture *LL_texture[LL_MAX_TEXTURES];



//
// All the sounds.
//

#define LL_MAX_SOUNDS 4096

LL_Sound *LL_sound[LL_MAX_SOUNDS];










LL_Texture *LL_create_texture(CBYTE *fname)
{
	SLONG i;

	//
	// Create the OS texture.
	//

	OS_Texture *ot = OS_texture_create(fname);

	//
	// Do we already have this texture?
	//

	for (i = 0; i < LL_MAX_TEXTURES; i++)
	{
		if (LL_texture[i] && LL_texture[i]->ot == ot)
		{
			//
			// We already have this texture!
			//

			LL_texture[i]->ref_count += 1;

			return LL_texture[i];
		}
	}

	//
	// Create a new texture.
	//

	LL_Texture *lt;
	
	lt            = (LL_Texture *) MEM_alloc(sizeof(LL_Texture));
	lt->ot        = ot;
	lt->width     = OS_texture_width (ot);
	lt->height    = OS_texture_height(ot);
	lt->ref_count = 1;

	//
	// Add it to our list of textures.
	//

	for (i = 0; i < LL_MAX_TEXTURES; i++)
	{
		if (LL_texture[i] == NULL)
		{
			LL_texture[i] = lt;

			return lt;
		}
	}

	//
	// No more textures!
	//

	ASSERT(0);

	return NULL;
}


void LL_free_texture(LL_Texture *lt)
{
	//
	// Free everything! We can't free OS_Textures!
	//

	MEM_free(lt);

	//
	// Get rid of the reference in the LL_texture[] array.
	//
	
	SLONG i;

	for (i = 0; i < LL_MAX_TEXTURES; i++)
	{
		if (LL_texture[i] == lt)
		{
			LL_texture[i] = NULL;

			return;
		}
	}

	ASSERT(0);
}







LL_Sound *LL_create_sound(CBYTE *fname)
{
	SLONG i;

	//
	// Create the OS sound.
	//

	OS_Sound *os = OS_sound_create(fname, OS_SOUND_TYPE_2D);

	//
	// Do we already have this sound?
	//

	for (i = 0; i < LL_MAX_SOUNDS; i++)
	{
		if (LL_sound[i] && LL_sound[i]->os == os)
		{
			//
			// We already have this sound!
			//

			LL_sound[i]->ref_count += 1;

			return LL_sound[i];
		}
	}

	//
	// Create a new sound.
	//

	LL_Sound *ls;
	
	ls            = (LL_Sound *) MEM_alloc(sizeof(LL_Sound));
	ls->os        = os;
	ls->ref_count = 1;

	//
	// Add it to our list of sounds.
	//

	for (i = 0; i < LL_MAX_SOUNDS; i++)
	{
		if (LL_sound[i] == NULL)
		{
			LL_sound[i] = ls;

			return ls;
		}
	}

	//
	// No more sounds!
	//

	ASSERT(0);

	return NULL;
}


void LL_free_sound(LL_Sound *ls)
{
	//
	// Free everything! We can't free OS_Sounds!
	//

	MEM_free(ls);

	//
	// Get rid of the reference in the LL_sound[] array.
	//
	
	SLONG i;

	for (i = 0; i < LL_MAX_SOUNDS; i++)
	{
		if (LL_sound[i] == ls)
		{
			LL_sound[i] = NULL;

			return;
		}
	}

	ASSERT(0);
}







LL_Buffer *LL_create_buffer(
				SLONG  type,
				void  *vert,
				SLONG  num_verts,
				UWORD *index,
				SLONG  num_indices)
{
	ASSERT(
		type == LL_BUFFER_TYPE_TLV ||
		type == LL_BUFFER_TYPE_LV);

	//
	// Create a new buffer.
	//

	LL_Buffer *lb = (LL_Buffer *) MEM_alloc(sizeof(LL_Buffer));

	lb->type        = type;
	lb->vert_data   = vert;
	lb->index       = index;
	lb->num_verts   = num_verts;
	lb->num_indices = num_indices;
	lb->ref_count   = 1;

	return lb;
}



void LL_free_buffer(LL_Buffer *lb)
{
	//
	// Free up data.
	//

	MEM_free(lb->vert);
	
	if (lb->index)
	{
		MEM_free(lb->index);
	}

	//
	// Now free up the actual buffer.
	//

	MEM_free(lb);
}





//
// Draws a buffer
//

OS_Vert LL_vert[OS_MAX_TRANS];

void LL_draw_buffer(
		LL_Buffer  *lb,
		LL_Texture *lt,		// NULL => Draw untextured
		ULONG       rs)		// The LL_RS_* renderstates ORed together.
{
	SLONG i;

	OS_Buffer *ob;
	OS_Trans  *ot;
	OS_Vert   *ov;
	LL_Tlvert *tl;

	ob = OS_buffer_new();

	//
	// Are there too many points in this buffer?
	//

	ASSERT(lb->num_verts <= OS_MAX_TRANS);

	//
	// Build the OS_trans and OS_vert buffers.
	//

	switch(lb->type)
	{
		case LL_BUFFER_TYPE_TLV:
			
			//
			// These are already transformed, so we can write
			// directly into the OS_trans[] array.
			//

			for (i = 0; i < lb->num_verts; i++)
			{
				ot = &OS_trans   [i];
				ov = &LL_vert    [i];
				tl = &lb->vert_tl[i];

				ot->X    = tl->x;
				ot->Y    = tl->y;
				ot->Z    = tl->rhw;
				ot->z    = tl->z;
				ot->clip = OS_CLIP_TRANSFORMED;

				ov->trans    = i;
				ov->index    = 0;
				ov->colour   = tl->colour;
				ov->specular = tl->specular;
				ov->u1       = tl->u;
				ov->v1       = tl->v;
				ov->u2       = 0.0F;
				ov->v2       = 0.0F;
			}

			if (lb->index)
			{
				ASSERT(lb->num_indices % 3 == 0);

				for (i = 0; i < lb->num_indices; i += 3)
				{
					OS_buffer_add_triangle(
						ob,
					   &LL_vert[lb->index[i + 0]],
					   &LL_vert[lb->index[i + 1]],
					   &LL_vert[lb->index[i + 2]]);
				}
			}
			else
			{
				for (i = 0; i < lb->num_verts; i += 3)
				{
					OS_buffer_add_triangle(
						ob,
					   &LL_vert[i + 0],
					   &LL_vert[i + 1],
					   &LL_vert[i + 2]);
				}
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Now draw the buffer.
	//

	OS_buffer_draw(ob, (lt) ? lt->ot : NULL, NULL, rs | OS_DRAW_DOUBLESIDED | OS_DRAW_ZALWAYS | OS_DRAW_NOZWRITE);
}



void LL_cls(ULONG colour, float z)
{
	SLONG r = (colour >> 16) & 0xff;
	SLONG g = (colour >>  8) & 0xff;
	SLONG b = (colour >>  0) & 0xff;

	ASSERT(WITHIN(z, 0.0F, 1.0F));

	OS_clear_screen(r,g,b,z);
}


void LL_flip()
{
	OS_show();
}

