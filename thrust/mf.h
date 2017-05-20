//
// Functions that act on the imported meshes
//

#ifndef _MF_
#define _MF_


#include "imp.h"


//
// Loads all the textures for the mesh.
//

void MF_load_textures(IMP_Mesh *im);


//
// Backs up the mesh, so it can be rotated later on.
//

void MF_backup(IMP_Mesh *im);


//
// MAKE SURE YOU'VE ALREADY CALLED MF_backup()
//

void MF_rotate_mesh(
		IMP_Mesh *im,
		float     yaw,
		float     pitch = 0.0F,
		float     roll  = 0.0F,
		float     scale = 1.0F,
		float     pos_x = 0.0F,
		float     pos_y = 0.0F,
		float     pos_z = 0.0F);

void MF_rotate_mesh(
		IMP_Mesh *im,
		float     pos_x,
		float     pos_y,
		float     pos_z,
		float     matrix[9]);	// Matrix must be normalised or the mesh normals will be fucked up.


//
// Transforms all the points of the mesh into the OS_trans array
//

void MF_transform_points(IMP_Mesh *im);


//
// Sets the (lu,lv)s of the vertices of the mesh to give a diffuse spotlight.
// It also sets the colour field of the shared vertices for diffuse lighting
// depending on the normal and the bumpmap (du,dv)s in the shared vectices.
//

void MF_diffuse_spotlight(
		IMP_Mesh *im,
		float     light_x,
		float     light_y,
		float     light_z,
		float     light_matrix[9],
		float     light_lens);		// The bigger the lens the smaller the spotlight.


//
// Sets the (lu,lv)s of the shared vertices of the given light to
// create a specular highlight.  This treats the light as a spot-light.
// It also sets the 'colour' field which should be used as a gouraud value
// for the specular map.
//

void MF_specular_spotlight(
		IMP_Mesh *im,
		float     light_x,
		float     light_y,
		float     light_z,
		float     light_matrix[9],
		float     light_lens);		// The bigger the lens the smaller the spotlight.




//
// Adds all the faces of the mesh normally...
// 

void MF_add_triangles_normal(IMP_Mesh *im, ULONG draw = OS_DRAW_NORMAL);


//
// Adds all the faces of the mesh normally but uses the given gouraud shade
// value for all the points.
// 

void MF_add_triangles_normal_colour(IMP_Mesh *im, ULONG draw = OS_DRAW_NORMAL, ULONG colour = 0xffffff);


//
// Creates each face using the (u,v)s from the verts and the colour from the
// sverts.  Draws using the given texture page.
//

void MF_add_triangles_light           (IMP_Mesh *im, OS_Texture *ot, ULONG draw = OS_DRAW_ADD | OS_DRAW_CLAMP);
void MF_add_triangles_light_bumpmapped(IMP_Mesh *im, OS_Texture *ot, ULONG draw = OS_DRAW_ADD | OS_DRAW_CLAMP);


//
// Adds the faces using a mesh light with MF_specular_spotlight. It takes the (u,v) from
// the sverts.  The texture (ot) should be a specular spotlight texture.
//

void MF_add_triangles_specular           (IMP_Mesh *im, OS_Texture *ot, ULONG draw = OS_DRAW_ADD | OS_DRAW_CLAMP);
void MF_add_triangles_specular_bumpmapped(IMP_Mesh *im, OS_Texture *ot, ULONG draw = OS_DRAW_ADD | OS_DRAW_CLAMP);

//
// Draws the specular shadowed using the diffuse spotlight.
//

void MF_add_triangles_specular_shadowed(IMP_Mesh *im, OS_Texture *ot_specdot, OS_Texture *ot_diffdot, ULONG draw = OS_DRAW_ADD | OS_DRAW_CLAMP | OS_DRAW_TEX_MUL);



#endif
