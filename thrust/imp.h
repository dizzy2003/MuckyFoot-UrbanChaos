//
// Imports SEX files.
//

#ifndef _IMP_
#define _IMP_

#include "os.h"

//
// Loads a SEX file and returns an IMP_Mesh structure.  If (!valid) in the
// the mesh structure, then it could not be loaded.  The name is the name of
// the first object found in the SEX file.
//

//
// Any faces with undefined textures (i.e. using qmark.tga) has the uv's of
// its points set to (0,0).
//

#define IMP_ALPHA_FILTERED 1
#define IMP_ALPHA_ADDITIVE 2

#define IMP_SIDED_SINGLE 1
#define IMP_SIDED_DOUBLE 2

typedef struct
{
	float r;
	float g;
	float b;
	float shininess;
	float shinstr;
	UBYTE alpha;		// IMP_ALPHA_*
	UBYTE sided;		// IMP_SIDED_*
	UBYTE has_texture;	// There is a texture
	UBYTE has_bumpmap;	// There is a bumpmap
	CBYTE tname[32];	// The texture filename or "none"
	CBYTE bname[32];	// The bumpmap filename or "none"

	//
	// Put extra stuff in here if you want...
	//

	OS_Texture *ot_tex;		// For the texture.
	OS_Texture *ot_bpos;	// For the bumpmap.
	OS_Texture *ot_bneg;	// For 1 - the bumpmap.

	OS_Buffer  *ob;

} IMP_Mat;

typedef struct
{
	float x;
	float y;
	float z;

	//
	// Add extra fields here if you want.
	//

	float lu;
	float lv;

} IMP_Vert;

typedef struct
{
	float u;
	float v;

} IMP_Tvert;

//
// Only faces of the same smoothing group with the same material
// and the same uvs share vertices.
//

typedef struct
{
	float u;
	float v;

	float nx;
	float ny;
	float nz;

	float dxdu;
	float dydu;
	float dzdu;

	float dxdv;
	float dydv;
	float dzdv;

	UWORD vert;	// Index into the vertex array for position.
	UWORD mat;	// The material of the faces that use this shared vertex.

	//
	// Add extra fields here if you want.
	//

	//
	// Lighting.
	//

	ULONG colour;
	ULONG specular;

	float lu;
	float lv;

	//
	// Bumpmapping.
	//

	float du;
	float dv;

} IMP_Svert;

#define IMP_FACE_FLAG_EDGE		(1 << 0)
#define IMP_FACE_FLAG_EDGE_A	(1 << 0)
#define IMP_FACE_FLAG_EDGE_B	(1 << 1)
#define IMP_FACE_FLAG_EDGE_C	(1 << 2)
#define IMP_FACE_FLAG_QUADDED	(1 << 3)	// This face is part of a quad.
#define IMP_FACE_FLAG_BACKFACE	(1 << 4)	// EXTRA FLAG! Not set by the importer...

typedef struct
{
	UWORD v[3];		// Index into the vertex array.
	UWORD t[3];		// index into the texture vertex array.
	UWORD s[3];		// Index into the shared vertex array.
	UBYTE mat;		// Index info the material array
	UBYTE flag;
	ULONG group;

	float nx;
	float ny;
	float nz;

	float dxdu;
	float dydu;
	float dzdu;

	float dxdv;
	float dydv;
	float dzdv;
 
} IMP_Face;

//
// Using the edge flags exported from MAX to find quads.
//

typedef struct
{
	UWORD v[4];
 
} IMP_Quad;


//
// The edges of the mesh. Which vertices they are between and the
// face that lies on each edge.
//

typedef struct
{
	UWORD v1;
	UWORD v2;
	UWORD f1;
	UWORD f2;	// 0xffff => The edge belongs to only one face.

} IMP_Edge;


typedef struct
{
	SLONG      valid;
	CBYTE      name[32];
	SLONG      num_mats;
	SLONG      num_verts;
	SLONG      num_tverts;
	SLONG      num_faces;
	SLONG      num_sverts;
	SLONG      num_quads;
	SLONG      num_edges;
	IMP_Mat   *mat;
	IMP_Vert  *vert;
	IMP_Tvert *tvert;
	IMP_Face  *face;
	IMP_Svert *svert;
	IMP_Quad  *quad;
	IMP_Edge  *edge;

	//
	// For backing-up when we rotate.
	//

	IMP_Vert  *old_vert;
	IMP_Svert *old_svert;

} IMP_Mesh;

IMP_Mesh IMP_load(CBYTE *fname, float scale = 1.0F);


//
// All the arrays in the IMP_Mesh structure are dynamically allocated.
// This function frees up all that memory.
//

void IMP_free(IMP_Mesh *im);



#endif
