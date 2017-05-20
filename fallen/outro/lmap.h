//
// Creates light maps.
//

#ifndef _LMAP_
#define _LMAP_


#include "imp.h"
#include "os.h"



//
// A lightmap...
//

typedef struct lmap_lmap LMAP_Lmap;



//
// Creates a new lightmap.  Resolution should be a sensible power of
// two (16 to 256).
//

LMAP_Lmap *LMAP_create(SLONG resolution);



//
// Only call this functions for one lightmap at a time.
//


//
// Makes the lightmap blank (i.e. completely blank!).
//

void LMAP_init(LMAP_Lmap *lmap);

//
// Adds the shadow of the mesh onto the lightmap.
//

void LMAP_add_shadow(
		LMAP_Lmap *lmap,
		IMP_Mesh  *im,
		float      light_x,
		float      light_y,
		float      light_z,
		float      light_matrix[9],
		float      light_lens);

//
// Renders the lightmap together with any shadows. You can call LMAP_shadow() and
// LMAP_render() multiple times per call to LMAP_init().  The texture must be
// the same resolution as the lightmap!
//

void LMAP_render(LMAP_Lmap *lmap, OS_Texture *ot);





#endif
