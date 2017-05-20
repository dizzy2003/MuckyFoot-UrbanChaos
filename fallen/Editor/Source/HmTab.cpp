//
// A simple editor for putting HM stuff around prims
//

#include "Editor.hpp"
#include "EditMod.hpp"
#include "ModeTab.hpp"
#include	"c:\fallen\headers\memory.h"

#include "c:\fallen\headers\hm.h"


//
// The info for each prim.
//

#define HMTAB_MAX_RES 8

typedef struct
{
	UBYTE x_res;
	UBYTE y_res;
	UBYTE z_res;
	UBYTE defined;

	SLONG x_point[HMTAB_MAX_RES];
	SLONG y_point[HMTAB_MAX_RES];
	SLONG z_point[HMTAB_MAX_RES];

	float x_dgrav;
	float y_dgrav;
	float z_dgrav;

} HMTAB_Prim;

#define HMTAB_MAX_PRIMS	256

HMTAB_Prim HMTAB_prim[HMTAB_MAX_PRIMS];

//
// The current prim.
//

SLONG HMTAB_current_prim;



//
// Stuff to help a 3d view.
//

#define VD_CAM_VIEW_X	1
#define VD_CAM_VIEW_Y	2
#define VD_CAM_VIEW_Z	3

SLONG VD_window_mid_x;
SLONG VD_window_mid_y;
SLONG VD_window_half_w;
SLONG VD_window_half_h;

SLONG VD_cam_view;
SLONG VD_cam_scale;
SLONG VD_cam_x;
SLONG VD_cam_y;
SLONG VD_cam_z;


//
// Returns the point on screen for the given 3d point.
//

void VD_transform(
		SLONG  x_3d,
		SLONG  y_3d,
		SLONG  z_3d,
		SLONG *x_2d,
		SLONG *y_2d)
{
	SLONG xc;
	SLONG yc;

	x_3d -= VD_cam_x;
	y_3d -= VD_cam_y;
	z_3d -= VD_cam_z;

	switch(VD_cam_view)
	{
		case VD_CAM_VIEW_X:
			xc = z_3d;
			yc = y_3d;
			break;

		case VD_CAM_VIEW_Y:
			xc = -z_3d;
			yc =  x_3d;
			break;

		case VD_CAM_VIEW_Z:
			xc = -x_3d;
			yc =  y_3d;
			break;

		default:
			ASSERT(0);
			break;
	}

	xc = MUL64(xc, VD_cam_scale);
	yc = MUL64(yc, VD_cam_scale);

   *x_2d = VD_window_mid_x + MUL64(xc, VD_window_half_w);
   *y_2d = VD_window_mid_y - MUL64(yc, VD_window_half_h);
}


//
// Only two out of the three 3d coordinates are filled in, depending on the
// view.
//

void VD_untransform(SLONG x_2d, SLONG y_2d, SLONG *x_3d, SLONG *y_3d, SLONG *z_3d)
{
	SLONG xc;
	SLONG yc;

	xc = DIV64( x_2d - VD_window_mid_x, VD_window_half_w);
	yc = DIV64(-y_2d + VD_window_mid_y, VD_window_half_h);

	xc = DIV64(xc, VD_cam_scale);
	yc = DIV64(yc, VD_cam_scale);

	switch(VD_cam_view)
	{
		case VD_CAM_VIEW_X:
			*z_3d = xc;
			*y_3d = yc;
			break;

		case VD_CAM_VIEW_Y:
			*z_3d = -xc;
			*x_3d =  yc;
			break;

		case VD_CAM_VIEW_Z:
			*x_3d = -xc;
			*y_3d =  yc;
			break;

		default:
			ASSERT(0);
			break;
	}

	*x_3d += VD_cam_x;
	*y_3d += VD_cam_y;
	*z_3d += VD_cam_z;
}


//
// Saves all the prims with interesting info.
//

void HMTAB_save_primgrids(CBYTE *fname)
{
	SLONG i;
	SLONG j;

	HMTAB_Prim *hp;

	HM_Header   hm_h;
	HM_Primgrid hm_pg;

	FILE *handle;

	handle = fopen(fname, "wb");

	if (handle == NULL)
	{
		TRACE("Could not open file %s\n", fname);

		return;
	}

	//
	// Build the header.
	//

	hm_h.version       = 1;
	hm_h.num_primgrids = 0;

	//
	// Count how many primgrids we should save.
	//

	for (i = 1; i < next_prim_object; i++)
	{
		hp = &HMTAB_prim[i];

		if (hp->defined)
		{
			if (hp->x_res      != 2		  ||
				hp->y_res      != 2		  ||
				hp->z_res      != 2		  ||
				hp->x_point[0] != 0		  ||
				hp->y_point[0] != 0		  ||
				hp->z_point[0] != 0       ||
				hp->x_point[1] != 0x10000 ||
				hp->y_point[1] != 0x10000 ||
				hp->z_point[1] != 0x10000)
			{
				hm_h.num_primgrids += 1;
			}
		}
	}

	//
	// Save out the header.
	//

	if (fwrite(&hm_h, sizeof(HM_Header), 1, handle) != 1) goto file_error;

	//
	// Save out each HM_Primgrid in turn.
	//

	for (i = 1; i < next_prim_object; i++)
	{
		hp = &HMTAB_prim[i];

		if (hp->defined)
		{
			if (hp->x_res      != 2		  ||
				hp->y_res      != 2		  ||
				hp->z_res      != 2		  ||
				hp->x_point[0] != 0		  ||
				hp->y_point[0] != 0		  ||
				hp->z_point[0] != 0       ||
				hp->x_point[1] != 0x10000 ||
				hp->y_point[1] != 0x10000 ||
				hp->z_point[1] != 0x10000)
			{
				hm_pg.prim  = i;
				hm_pg.x_res = hp->x_res;
				hm_pg.y_res = hp->y_res;
				hm_pg.z_res = hp->z_res;

				for (j = 0; j < HM_MAX_RES; j++)
				{
					hm_pg.x_point[j] = hp->x_point[j];
					hm_pg.y_point[j] = hp->y_point[j];
					hm_pg.z_point[j] = hp->z_point[j];
				}

				hm_pg.x_dgrav = hp->x_dgrav;
				hm_pg.y_dgrav = hp->y_dgrav;
				hm_pg.z_dgrav = hp->z_dgrav;

				if (fwrite(&hm_pg, sizeof(HM_Primgrid), 1, handle) != 1) goto file_error;
			}
		}
	}

	fclose(handle);

	return;

  file_error:;
  
	fclose(handle);

	TRACE("Error saving file %s\n", fname);

	return;
}

void HMTAB_load_primgrids(CBYTE *fname)
{
	SLONG i;
	SLONG j;

	HMTAB_Prim *hp;

	HM_Header   hm_h;
	HM_Primgrid hm_pg;

	FILE *handle;

	handle = fopen(fname, "rb");

	if (handle == NULL)
	{
		TRACE("Could not open file %s\n", fname);

		return;
	}

	//
	// Load the header.
	//

	if (fread(&hm_h, sizeof(HM_Header), 1, handle) != 1) goto file_error;

	if (hm_h.version != 1)
	{
		TRACE("File %s is an obsolete version\n", fname);
		fclose(handle);
		return;
	}

	//
	// Load in each HM_Primgrid in turn.
	//

	for (i = 0; i < hm_h.num_primgrids; i++)
	{
		if (fread(&hm_pg, sizeof(HM_Primgrid), 1, handle) != 1) goto file_error;

		ASSERT(WITHIN(hm_pg.prim, 0, HMTAB_MAX_PRIMS));

		hp = &HMTAB_prim[hm_pg.prim];

		hp->defined = TRUE;
		hp->x_res   = hm_pg.x_res;
		hp->y_res   = hm_pg.y_res;
		hp->z_res   = hm_pg.z_res;
		
		for (j = 0; j < HM_MAX_RES; j++)
		{
			hp->x_point[j] = hm_pg.x_point[j];
			hp->y_point[j] = hm_pg.y_point[j];
			hp->z_point[j] = hm_pg.z_point[j];
		}

		hp->x_dgrav = hm_pg.x_dgrav;
		hp->y_dgrav = hm_pg.y_dgrav;
		hp->z_dgrav = hm_pg.z_dgrav;
	}

	fclose(handle);

	return;

  file_error:;
  
	fclose(handle);

	TRACE("Error loading file %s\n", fname);

	return;
}



//
// Draws a prim from the current camera.
//

void HmTab::draw_prim(UWORD prim)
{
	ASSERT(WITHIN(prim, 1, next_prim_object - 1));

	PrimObject *po = &prim_objects[prim];
	PrimPoint  *pp1;
	PrimPoint  *pp2;
	PrimFace3  *f3;
	PrimFace4  *f4;

	SLONG i;
	SLONG j;
	SLONG p1;
	SLONG p2;
	SLONG sx1;
	SLONG sy1;
	SLONG sx2;
	SLONG sy2;

	for (i = po->StartFace3; i < po->EndFace3; i++)
	{
		ASSERT(WITHIN(i, 1, next_prim_face3 - 1));

		f3 = &prim_faces3[i];

		for (j = 0; j < 3; j++)
		{
			p1 = f3->Points[(j + 0) % 3];
			p2 = f3->Points[(j + 1) % 3];

			ASSERT(WITHIN(p1, 1, next_prim_point - 1));
			ASSERT(WITHIN(p2, 1, next_prim_point - 1));
			
			pp1 = &prim_points[p1];
			pp2 = &prim_points[p2];

			VD_transform(
				pp1->X,
				pp1->Y,
				pp1->Z,
			   &sx1,
			   &sy1);

			VD_transform(
				pp2->X,
				pp2->Y,
				pp2->Z,
			   &sx2,
			   &sy2);

			DrawLineC(sx1, sy1, sx2, sy2, RED_COL);
		}
	}

	static UBYTE line_order[4] = {0, 1, 3, 2};

	for (i = po->StartFace4; i < po->EndFace4; i++)
	{
		ASSERT(WITHIN(i, 1, next_prim_face4 - 1));

		f4 = &prim_faces4[i];

		for (j = 0; j < 4; j++)
		{
			p1 = f4->Points[line_order[(j + 0) % 4]];
			p2 = f4->Points[line_order[(j + 1) % 4]];

			ASSERT(WITHIN(p1, 1, next_prim_point - 1));
			ASSERT(WITHIN(p2, 1, next_prim_point - 1));
			
			pp1 = &prim_points[p1];
			pp2 = &prim_points[p2];

			VD_transform(
				pp1->X,
				pp1->Y,
				pp1->Z,
			   &sx1,
			   &sy1);

			VD_transform(
				pp2->X,
				pp2->Y,
				pp2->Z,
			   &sx2,
			   &sy2);

			DrawLineC(sx1, sy1, sx2, sy2, RED_COL);
		}
	}
}

//
// Draw the lines for the given prim and returns the point the mouse is over, or
// -1 if the mouse isn't over any point.
//

void HmTab::draw_grid(UWORD prim)
{
	SLONG i;

	SLONG x;
	SLONG y;

	SLONG x1;
	SLONG y1;
	SLONG x2;
	SLONG y2;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG px1;
	SLONG py1;
	SLONG pz1;
	SLONG px2;
	SLONG py2;
	SLONG pz2;

	SLONG gx1;
	SLONG gy1;
	SLONG gx2;
	SLONG gy2;

	ASSERT(WITHIN(prim, 1, next_prim_object - 1));
	ASSERT(WITHIN(prim, 1, HMTAB_MAX_PRIMS  - 1));

	PrimInfo   *pi =  get_prim_info(prim);
	PrimObject *po = &prim_objects [prim];
	HMTAB_Prim *hp = &HMTAB_prim   [prim];
	
	if (!hp->defined)
	{
		//
		// We had better define it!
		//

		hp->defined = TRUE;

		hp->x_res = 2;
		hp->y_res = 2;
		hp->z_res = 2;

		hp->x_point[0] = 0x00000;
		hp->x_point[1] = 0x10000;

		hp->y_point[0] = 0x00000;
		hp->y_point[1] = 0x10000;

		hp->z_point[0] = 0x00000;
		hp->z_point[1] = 0x10000;

		hp->x_dgrav = 0.0F;
		hp->y_dgrav = 0.0F;
		hp->z_dgrav = 0.0F;
	}

	//
	// The size of the bounding box of the prim.
	//

	SLONG bbdx = pi->maxx - pi->minx;
	SLONG bbdy = pi->maxy - pi->miny;
	SLONG bbdz = pi->maxz - pi->minz;

	ASSERT(WITHIN(hp->x_res, 2, HMTAB_MAX_RES));
	ASSERT(WITHIN(hp->y_res, 2, HMTAB_MAX_RES));
	ASSERT(WITHIN(hp->z_res, 2, HMTAB_MAX_RES));

	//
	// Work out the bounding box of the grid on the screen.
	//

	px1 = pi->minx + MUL64(bbdx, hp->x_point[0]);
	px2 = pi->minx + MUL64(bbdx, hp->x_point[hp->x_res - 1]);

	py1 = pi->miny + MUL64(bbdy, hp->y_point[0]);
	py2 = pi->miny + MUL64(bbdy, hp->y_point[hp->y_res - 1]);

	pz1 = pi->minz + MUL64(bbdz, hp->z_point[0]);
	pz2 = pi->minz + MUL64(bbdz, hp->z_point[hp->z_res - 1]);

	VD_transform(px1, py1, pz1, &gx1, &gy1);
	VD_transform(px1, py2, pz2, &gx2, &gy2);
	
	if (VD_cam_view != VD_CAM_VIEW_X)
	{
		//
		// Draw the grey grid.
		//

		for (i = 0; i < hp->x_res; i++)
		{
			px = pi->minx + MUL64(bbdx, hp->x_point[i]);

			if (VD_cam_view == VD_CAM_VIEW_Y)
			{
				VD_transform(px, py1, pz1, &x1, &y1);
				VD_transform(px, py1, pz2, &x2, &y2);

				DrawLineC(x1,y1, x2,y2, GREY_COL);
			}
			else
			{
				ASSERT(VD_cam_view == VD_CAM_VIEW_Z);

				VD_transform(px, py1, pz1, &x1, &y1);
				VD_transform(px, py2, pz1, &x2, &y2);

				DrawLineC(x1,y1, x2,y2, GREY_COL);
			}
		}

		//
		// Draw a line across the x-axis.
		//

		VD_transform(px1, py1, pz1, &x1, &y1);
		VD_transform(px2, py1, pz1, &x2, &y2);

		DrawLineC(x1,y1, x2,y2, GREY_COL);

		//
		// Draw all the points.
		//

		for (i = 0; i < hp->x_res; i++)
		{
			px = pi->minx + MUL64(bbdx, hp->x_point[i]);

			VD_transform(px, py1, pz1, &x, &y);

			#define HMTAB_POINT_RADIUS 2

			x1 = x - HMTAB_POINT_RADIUS;
			y1 = y - HMTAB_POINT_RADIUS;

			x2 = x + HMTAB_POINT_RADIUS;
			y2 = y + HMTAB_POINT_RADIUS;

			DrawLineC(x1, y1, x2, y1, WHITE_COL);
			DrawLineC(x2, y1, x2, y2, WHITE_COL);
			DrawLineC(x2, y2, x1, y2, WHITE_COL);
			DrawLineC(x1, y2, x1, y1, WHITE_COL);
		}

		//
		// Label the axis.
		//

		VD_transform(px2, py1, pz1, &x, &y);

		QuickTextC(x + 4, y, "X", GREEN_COL);
	}

	if (VD_cam_view != VD_CAM_VIEW_Y)
	{
		//
		// Draw the grey grid.
		//

		for (i = 0; i < hp->y_res; i++)
		{
			py = pi->miny + MUL64(bbdy, hp->y_point[i]);

			if (VD_cam_view == VD_CAM_VIEW_X)
			{
				VD_transform(px1, py, pz1, &x1, &y1);
				VD_transform(px1, py, pz2, &x2, &y2);

				DrawLineC(x1,y1, x2,y2, GREY_COL);
			}
			else
			{
				ASSERT(VD_cam_view == VD_CAM_VIEW_Z);

				VD_transform(px1, py, pz1, &x1, &y1);
				VD_transform(px2, py, pz1, &x2, &y2);

				DrawLineC(x1,y1, x2,y2, GREY_COL);
			}
		}

		//
		// Draw a line across the y-axis.
		//

		VD_transform(px1, py1, pz1, &x1, &y1);
		VD_transform(px1, py2, pz1, &x2, &y2);

		DrawLineC(x1,y1, x2,y2, GREY_COL);

		//
		// Draw all the points.
		//

		for (i = 0; i < hp->y_res; i++)
		{
			py = pi->miny + MUL64(bbdy, hp->y_point[i]);

			VD_transform(px1, py, pz1, &x, &y);

			#define HMTAB_POINT_RADIUS 2

			x1 = x - HMTAB_POINT_RADIUS;
			y1 = y - HMTAB_POINT_RADIUS;

			x2 = x + HMTAB_POINT_RADIUS;
			y2 = y + HMTAB_POINT_RADIUS;

			DrawLineC(x1, y1, x2, y1, WHITE_COL);
			DrawLineC(x2, y1, x2, y2, WHITE_COL);
			DrawLineC(x2, y2, x1, y2, WHITE_COL);
			DrawLineC(x1, y2, x1, y1, WHITE_COL);
		}

		//
		// Label the axis.
		//

		VD_transform(px1, py2, pz1, &x, &y);

		QuickTextC(x + 4, y, "Y", GREEN_COL);
	}

	if (VD_cam_view != VD_CAM_VIEW_Z)
	{
		//
		// Draw the grey grid.
		//

		for (i = 0; i < hp->z_res; i++)
		{
			pz = pi->minz + MUL64(bbdz, hp->z_point[i]);

			if (VD_cam_view == VD_CAM_VIEW_Y)
			{
				VD_transform(px1, py1, pz, &x1, &y1);
				VD_transform(px2, py1, pz, &x2, &y2);

				DrawLineC(x1,y1, x2,y2, GREY_COL);
			}
			else
			{
				ASSERT(VD_cam_view == VD_CAM_VIEW_X);

				VD_transform(px1, py1, pz, &x1, &y1);
				VD_transform(px1, py2, pz, &x2, &y2);

				DrawLineC(x1,y1, x2,y2, GREY_COL);
			}
		}

		//
		// Draw a line across the z-axis.
		//

		VD_transform(px1, py1, pz1, &x1, &y1);
		VD_transform(px1, py1, pz2, &x2, &y2);

		DrawLineC(x1,y1, x2,y2, GREY_COL);

		//
		// Draw all the points.
		//

		for (i = 0; i < hp->z_res; i++)
		{
			pz = pi->minz + MUL64(bbdz, hp->z_point[i]);

			VD_transform(px1, py1, pz, &x, &y);

			#define HMTAB_POINT_RADIUS 2

			x1 = x - HMTAB_POINT_RADIUS;
			y1 = y - HMTAB_POINT_RADIUS;

			x2 = x + HMTAB_POINT_RADIUS;
			y2 = y + HMTAB_POINT_RADIUS;

			DrawLineC(x1, y1, x2, y1, WHITE_COL);
			DrawLineC(x2, y1, x2, y2, WHITE_COL);
			DrawLineC(x2, y2, x1, y2, WHITE_COL);
			DrawLineC(x1, y2, x1, y1, WHITE_COL);
		}

		//
		// Label the axis.
		//

		VD_transform(px1, py1, pz2, &x, &y);

		QuickTextC(x + 4, y, "Z", GREEN_COL);
	}
}

void HmTab::draw_cog(UWORD prim)
{
	UBYTE hm_index;

	SLONG x;
	SLONG y;

	float cog_x;
	float cog_y;
	float cog_z;

	ASSERT(WITHIN(prim, 1, next_prim_object - 1));
	ASSERT(WITHIN(prim, 1, HMTAB_MAX_PRIMS  - 1));

	PrimInfo   *pi =  get_prim_info(prim);
	PrimObject *po = &prim_objects [prim];
	HMTAB_Prim *hp = &HMTAB_prim   [prim];
	
	if (!hp->defined)
	{
		//
		// We had better define it!
		//

		hp->defined = TRUE;

		hp->x_res = 2;
		hp->y_res = 2;
		hp->z_res = 2;

		hp->x_point[0] = 0x00000;
		hp->x_point[1] = 0x10000;

		hp->y_point[0] = 0x00000;
		hp->y_point[1] = 0x10000;

		hp->z_point[0] = 0x00000;
		hp->z_point[1] = 0x10000;

		hp->x_dgrav = 0.0F;
		hp->y_dgrav = 0.0F;
		hp->z_dgrav = 0.0F;
	}

	//
	// Create a hypermatter object.
	//

	hm_index = HM_create(
					prim,
					0, 0, 0,
					0, 0, 0, 
					0, 0, 0,
					hp->x_res,
					hp->y_res,
					hp->z_res,
					hp->x_point,
					hp->y_point,
					hp->z_point,
					hp->x_dgrav,
					hp->y_dgrav,
					hp->z_dgrav,
					0.0F, 0.0F, 0.0F, 0.0F);

	if (hm_index != HM_NO_MORE_OBJECTS)
	{
		//
		// Find its centre of gravity.
		//

		HM_find_cog(
			 hm_index,
			&cog_x,
			&cog_y,
			&cog_z);

		//
		// Draw the cog.
		//

		VD_transform(
			SLONG(cog_x),
			SLONG(cog_y),
			SLONG(cog_z),
		   &x,
		   &y);

		#define HMTAB_COG_SIZE 8

		DrawLineC(x, y, x + HMTAB_COG_SIZE, y, GREEN_COL);
		DrawLineC(x, y, x - HMTAB_COG_SIZE, y, GREEN_COL);
		DrawLineC(x, y, x, y + HMTAB_COG_SIZE, GREEN_COL);
		DrawLineC(x, y, x, y - HMTAB_COG_SIZE, GREEN_COL);

		//
		// Destroy the hm_object.
		// 

		HM_destroy(hm_index);
	}
}

//
// Looks for a point in the given HMTAB_prim that is close to the
// mouse and moves it to the mouse position.
//
// (mouse_x, mouse_y) should be in window coordinates. Returns TRUE
// if it was close enough to change a point.
//

SLONG move_point(UWORD prim, SLONG mouse_x, SLONG mouse_y)
{
	SLONG i;

	SLONG px1;
	SLONG py1;
	SLONG pz1;
	SLONG px2;
	SLONG py2;
	SLONG pz2;

	SLONG mx;
	SLONG my;
	SLONG mz;

	SLONG px;
	SLONG py;
	SLONG pz;

	SLONG x;
	SLONG y;

	SLONG dx;
	SLONG dy;
	SLONG dist;
	SLONG change = FALSE;

	ASSERT(WITHIN(prim, 1, next_prim_object - 1));
	ASSERT(WITHIN(prim, 1, HMTAB_MAX_PRIMS  - 1));

	PrimInfo   *pi =  get_prim_info(prim);
	PrimObject *po = &prim_objects [prim];
	HMTAB_Prim *hp = &HMTAB_prim   [prim];
	
	if (!hp->defined)
	{
		//
		// We had better define it!
		//

		hp->defined = TRUE;

		hp->x_res = 2;
		hp->y_res = 2;
		hp->z_res = 2;

		hp->x_point[0] = 0x00000;
		hp->x_point[1] = 0x10000;

		hp->y_point[0] = 0x00000;
		hp->y_point[1] = 0x10000;

		hp->z_point[0] = 0x00000;
		hp->z_point[1] = 0x10000;

		hp->x_dgrav = 0.0F;
		hp->y_dgrav = 0.0F;
		hp->z_dgrav = 0.0F;
	}

	//
	// Where the mouse is in 3d
	//

	VD_untransform(mouse_x, mouse_y, &mx, &my, &mz);

	//
	// The size of the bounding box of the prim.
	//

	SLONG bbdx = pi->maxx - pi->minx;
	SLONG bbdy = pi->maxy - pi->miny;
	SLONG bbdz = pi->maxz - pi->minz;

	ASSERT(WITHIN(hp->x_res, 2, HMTAB_MAX_RES));
	ASSERT(WITHIN(hp->y_res, 2, HMTAB_MAX_RES));
	ASSERT(WITHIN(hp->z_res, 2, HMTAB_MAX_RES));

	//
	// Work out the bounding box of the grid.
	//

	px1 = pi->minx + MUL64(bbdx, hp->x_point[0]);
	px2 = pi->minx + MUL64(bbdx, hp->x_point[hp->x_res - 1]);

	py1 = pi->miny + MUL64(bbdy, hp->y_point[0]);
	py2 = pi->miny + MUL64(bbdy, hp->y_point[hp->y_res - 1]);

	pz1 = pi->minz + MUL64(bbdz, hp->z_point[0]);
	pz2 = pi->minz + MUL64(bbdz, hp->z_point[hp->z_res - 1]);

	if (VD_cam_view != VD_CAM_VIEW_X)
	{
		for (i = 0; i < hp->x_res; i++)
		{
			px = pi->minx + MUL64(bbdx, hp->x_point[i]);
			
			VD_transform(px, py1, pz1, &x, &y);

			dx = abs(x - mouse_x);
			dy = abs(y - mouse_y);

			dist = dx + dy;

			if (dist <= 9)
			{
				//
				// Move the point to the position of the mouse.
				//

				hp->x_point[i] = DIV64(mx - pi->minx, bbdx);

				change = TRUE;
			}
		}
	}

	if (VD_cam_view != VD_CAM_VIEW_Y)
	{
		for (i = 0; i < hp->y_res; i++)
		{
			py = pi->miny + MUL64(bbdy, hp->y_point[i]);
			
			VD_transform(px1, py, pz1, &x, &y);

			dx = abs(x - mouse_x);
			dy = abs(y - mouse_y);

			dist = dx + dy;

			if (dist <= 9)
			{
				//
				// Move the point to the position of the mouse.
				//

				hp->y_point[i] = DIV64(my - pi->miny, bbdy);

				change = TRUE;
			}
		}
	}

	if (VD_cam_view != VD_CAM_VIEW_Z)
	{
		for (i = 0; i < hp->z_res; i++)
		{
			pz = pi->minz + MUL64(bbdz, hp->z_point[i]);
			
			VD_transform(px1, py1, pz, &x, &y);

			dx = abs(x - mouse_x);
			dy = abs(y - mouse_y);

			dist = dx + dy;

			if (dist <= 9)
			{
				//
				// Move the point to the position of the mouse.
				//

				hp->z_point[i] = DIV64(mz - pi->minz, bbdz);

				change = TRUE;
			}
		}
	}

	//
	// Make sure that no point of the prim lies outside the hypermatter.
	//

	if (hp->x_point[0] > 0) {hp->x_point[0] = 0;}
	if (hp->y_point[0] > 0) {hp->y_point[0] = 0;}
	if (hp->z_point[0] > 0) {hp->z_point[0] = 0;}

	if (hp->x_point[hp->x_res - 1] < 0x10000) {hp->x_point[hp->x_res - 1] = 0x10000;}
	if (hp->y_point[hp->y_res - 1] < 0x10000) {hp->y_point[hp->y_res - 1] = 0x10000;}
	if (hp->z_point[hp->z_res - 1] < 0x10000) {hp->z_point[hp->z_res - 1] = 0x10000;}

	return change;
}





//
// The buttons in the Tab.
//

#define CTRL_NEXT_PRIM		1
#define CTRL_PREV_PRIM		2
							
#define CTRL_RES_X_UP		3
#define CTRL_RES_X_DOWN		4
#define CTRL_RES_Y_UP		5
#define CTRL_RES_Y_DOWN		6
#define CTRL_RES_Z_UP		7
#define CTRL_RES_Z_DOWN		8
							
#define CTRL_GRAV_U			9
#define CTRL_GRAV_D			10
#define CTRL_GRAV_L			11
#define CTRL_GRAV_R			12
							
#define CTRL_GRAV_CENTRE	13
							
#define CTRL_SAVE			14

ControlDef HMTab_def[] =
{
	{BUTTON, 0, "Next Prim",	10,  20},
	{BUTTON, 0, "Prev Prim",	10,  35},

	{BUTTON, 0, "X res UP",		10,  60},
	{BUTTON, 0, "X res DOWN", 	10,  75},

	{BUTTON, 0, "Y res UP",		90,  60},
	{BUTTON, 0, "Y res DOWN", 	90,  75},

	{BUTTON, 0, "Z res UP",		170, 60},
	{BUTTON, 0, "Z res DOWN", 	170, 75},

	{BUTTON, 0, "U",			100, 100},
	{BUTTON, 0, "D",			100, 130},
	{BUTTON, 0, "L",			 72, 115},
	{BUTTON, 0, "R",			128, 115},

	{BUTTON, 0, "CENTRE",       85,  115},

	{BUTTON, 0, "Save",         10,  200},

	{0}
};

//
// The parent of this tab.
//

EditorModule *HMTAB_parent;


HmTab::HmTab(EditorModule *parent)
{
	//
	// Install our buttons.
	//

	InitControlSet(HMTab_def);

	//
	// Set the default camera.
	//

	VD_cam_view  = VD_CAM_VIEW_X;
	VD_cam_x     = 0;
	VD_cam_y     = 0;
	VD_cam_z     = 0;
	VD_cam_scale = 0xa00000;

	//
	// Set the default prim.
	//

	HMTAB_current_prim = PRIM_OBJ_SOFA;

	//
	// Remember our parent, so we can ask her to
	// do stuff to us.
	//

	HMTAB_parent = parent;

	//
	// Load all the primgrid stuff...
	//

	HMTAB_load_primgrids("data\\primgrid.dat");
}

HmTab::~HmTab()
{
}



void HmTab::DrawTabContent()
{
	EdRect content_rect;

	//
	// Fill with the standard colour.
	//

	content_rect = ContentRect;	
	content_rect.ShrinkRect(1,1);
	content_rect.FillRect(CONTENT_COL);

	//
	// Draw the buttons.
	// 

	SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);
	DrawControlSet();
	ShowWorkWindow(0);
}

void HmTab::HandleTab(MFPoint *current_point)
{
	SLONG change = FALSE;

	//
	// Do the buttons.
	//

	HandleControlSet(current_point);

	//
	// Do the keyboard.
	//

	if (Keys[KB_HOME]) {VD_cam_scale += 0x30000; change = TRUE;}
	if (Keys[KB_END ]) {VD_cam_scale -= 0x30000; change = TRUE;}

	SATURATE(VD_cam_scale, 0x800000, 0x4000000);

	if (Keys[KB_TAB])
	{
		Keys[KB_TAB] = 0;

		switch(VD_cam_view)
		{
			case VD_CAM_VIEW_X: VD_cam_view = VD_CAM_VIEW_Y; break;
			case VD_CAM_VIEW_Y: VD_cam_view = VD_CAM_VIEW_Z; break;
			case VD_CAM_VIEW_Z: VD_cam_view = VD_CAM_VIEW_X; break;
			default:
				ASSERT(0);
				break;
		}

		change = TRUE;
	}

	if (change)
	{
		//
		// Redraw ourselves..
		//

		RedrawModuleContent = TRUE;
		RequestUpdate();
	}
}

UWORD HmTab::HandleTabClick(UBYTE flags, MFPoint *clicked_point)
{
	UWORD    control_id;
	Control	*current_control;
	MFPoint	 local_point;

	//
	// This is a fudge to update the front screen buffer.
	//

	ShowWorkScreen(0);

	switch(flags)
	{
		case	NO_CLICK:
			break;
		case	LEFT_CLICK:

			SetWorkWindowBounds(ContentLeft()+1,ContentTop()+1,ContentWidth()-1,ContentHeight()-1);

			//
			// Where in the tab is the click?
			//

			local_point = *clicked_point;
			GlobalToLocal(&local_point);

			//
			// Go through all the controls in the tab and see if we have clicked on any of them.
			//

			current_control = GetControlList();

			while(current_control)
			{
				if(!(current_control->GetFlags()&CONTROL_INACTIVE) && current_control->PointInControl(&local_point))
				{	
					//
					// A click on this button.
					//

					control_id = current_control->TrackControl(&local_point);

					HandleControl(control_id);

					//
					// Tidy up display.
					//

					if(LockWorkScreen())
					{
						DrawTab();
						UnlockWorkScreen();
					}

					ShowWorkWindow(0);

					return control_id;
				}

				current_control = current_control->GetNextControl();
			}

			break;
		case	RIGHT_CLICK:
			break;
	}

	return 0;
}

void HmTab::HandleControl(UWORD control_id)
{
	SLONG i;

	SLONG old_x_res;
	SLONG old_y_res;
	SLONG old_z_res;

	float dup;
	float dright;

	SLONG mul;

	ASSERT(WITHIN(HMTAB_current_prim, 1, HMTAB_MAX_PRIMS - 1));

	HMTAB_Prim *hp = &HMTAB_prim[HMTAB_current_prim];

	old_x_res = hp->x_res;
	old_y_res = hp->y_res;
	old_z_res = hp->z_res;

	dup    = 0.0F;
	dright = 0.0F;

	#define HMTAB_DGRAV (0.1F)

	switch(control_id)
	{
		case CTRL_NEXT_PRIM: HMTAB_current_prim += 1; break;
		case CTRL_PREV_PRIM: HMTAB_current_prim -= 1; break;
		
		case CTRL_RES_X_UP:   hp->x_res += 1; break;
		case CTRL_RES_X_DOWN: hp->x_res -= 1; break;

		case CTRL_RES_Y_UP:   hp->y_res += 1; break;
		case CTRL_RES_Y_DOWN: hp->y_res -= 1; break;

		case CTRL_RES_Z_UP:   hp->z_res += 1; break;
		case CTRL_RES_Z_DOWN: hp->z_res -= 1; break;

		case CTRL_GRAV_U: dup = +HMTAB_DGRAV; break;
		case CTRL_GRAV_D: dup = -HMTAB_DGRAV; break;

		case CTRL_GRAV_R: dright = +HMTAB_DGRAV; break;
		case CTRL_GRAV_L: dright = -HMTAB_DGRAV; break;

		case CTRL_GRAV_CENTRE:
			hp->x_dgrav = 0.0F;
			hp->y_dgrav = 0.0F;
			hp->z_dgrav = 0.0F;
			break;

		case CTRL_SAVE: HMTAB_save_primgrids("data\\primgrid.dat"); break;

		default:
			ASSERT(0);
			break;
	}

	switch(VD_cam_view)
	{
		case VD_CAM_VIEW_X:
			hp->z_dgrav += dright;
			hp->y_dgrav += dup;
			break;

		case VD_CAM_VIEW_Y:
			hp->z_dgrav -= dright;
			hp->x_dgrav += dup;
			break;

		case VD_CAM_VIEW_Z:
			hp->x_dgrav -= dright;
			hp->y_dgrav += dup;
			break;

		default:
			ASSERT(0);
			break;
	}

	SATURATE(hp->x_res, 2, HMTAB_MAX_RES - 1);
	SATURATE(hp->y_res, 2, HMTAB_MAX_RES - 1);
	SATURATE(hp->z_res, 2, HMTAB_MAX_RES - 1);

	if (old_x_res != hp->x_res)
	{
		//
		// We have to squeeze the old points into a smaller area or
		// the old points into a bigger area.
		//

		mul = DIV64(old_x_res - 1, hp->x_res - 1);

		for (i = 0; i < old_x_res; i++)
		{
			hp->x_point[i] = MUL64(hp->x_point[i], mul);
		}

		if (hp->x_res > old_x_res)
		{
			hp->x_point[hp->x_res - 1] = 0x10000;
		}
	}

	if (old_y_res != hp->y_res)
	{
		//
		// We have to squeeze the old points into a smaller area or
		// the old points into a bigger area.
		//

		mul = DIV64(old_y_res - 1, hp->y_res - 1);

		for (i = 0; i < old_y_res; i++)
		{
			hp->y_point[i] = MUL64(hp->y_point[i], mul);
		}

		if (hp->y_res > old_y_res)
		{
			hp->y_point[hp->y_res - 1] = 0x10000;
		}
	}

	if (old_z_res != hp->z_res)
	{
		//
		// We have to squeeze the old points into a smaller area or
		// the old points into a bigger area.
		//

		mul = DIV64(old_z_res - 1, hp->z_res - 1);

		for (i = 0; i < old_z_res; i++)
		{
			hp->z_point[i] = MUL64(hp->z_point[i], mul);
		}

		if (hp->z_res > old_z_res)
		{
			hp->z_point[hp->z_res - 1] = 0x10000;
		}
	}

	//
	// In case we have changed the current prim.
	//

	{
		SATURATE(HMTAB_current_prim, 1, next_prim_object - 1);

		ASSERT(WITHIN(HMTAB_current_prim, 1, HMTAB_MAX_PRIMS - 1));

		HMTAB_Prim *hp = &HMTAB_prim[HMTAB_current_prim];

		if (!hp->defined)
		{
			hp->defined = 1;

			hp->x_res = 2;
			hp->y_res = 2;
			hp->z_res = 2;

			hp->x_point[0] = 0x00000;
			hp->x_point[1] = 0x10000;

			hp->y_point[0] = 0x00000;
			hp->y_point[1] = 0x10000;

			hp->z_point[0] = 0x00000;
			hp->z_point[1] = 0x10000;
		}
	}

	//
	// Make sure that no point of the prim lies outside the hypermatter.
	//

	if (hp->x_point[0] > 0) {hp->x_point[0] = 0;}
	if (hp->y_point[0] > 0) {hp->y_point[0] = 0;}
	if (hp->z_point[0] > 0) {hp->z_point[0] = 0;}

	if (hp->x_point[hp->x_res - 1] < 0x10000) {hp->x_point[hp->x_res - 1] = 0x10000;}
	if (hp->y_point[hp->y_res - 1] < 0x10000) {hp->y_point[hp->y_res - 1] = 0x10000;}
	if (hp->z_point[hp->z_res - 1] < 0x10000) {hp->z_point[hp->z_res - 1] = 0x10000;}

	RedrawModuleContent = TRUE;
	RequestUpdate();
}

void HmTab::DrawModuleContent(SLONG x,SLONG y,SLONG w,SLONG h)
{
	//
	// To begin with...
	//

	SLONG wwx;
	SLONG wwy;
	SLONG www;
	SLONG wwh;

	SLONG over_x;
	SLONG over_y;
	SLONG over_z;

	EdRect drawrect;

	RedrawModuleContent=0;
	wwx=WorkWindowRect.Left;
	wwy=WorkWindowRect.Top;
	www=WorkWindowRect.Width;
	wwh=WorkWindowRect.Height;
	SetWorkWindowBounds(x,y,w-1,h-1);
	drawrect.SetRect(0,0,w-1,h-1);
	drawrect.FillRect(CONTENT_COL_BR);

	//
	// Actaul drawing we want to do.
	//

	QuickTextC(10, 10, "Hi Guys!", WHITE_COL);

	VD_window_mid_x = w / 2;
	VD_window_mid_y = h / 2;

	VD_window_half_w = w / 2;
	VD_window_half_h = h / 2;

	draw_prim(HMTAB_current_prim);
	draw_grid(HMTAB_current_prim);
	draw_cog (HMTAB_current_prim);

	//
	// To end with...
	//

	SetWorkWindowBounds(wwx, wwy, www, wwh); // Restore clip rectangle.

}

SLONG HmTab::HandleModuleContentClick(MFPoint *clicked_point, UBYTE flags, SLONG x, SLONG y, SLONG w, SLONG h)
{
	SLONG mouse_x;
	SLONG mouse_y;

	TRACE("clicked_point = (%d,%d)\n", clicked_point->X, clicked_point->Y);

	mouse_x = MouseX - x;
	mouse_y = MouseY - y;

	while(move_point(HMTAB_current_prim, mouse_x, mouse_y))
	{
		SLONG wwx = WorkWindowRect.Left;
		SLONG wwy = WorkWindowRect.Top;
		SLONG www = WorkWindowRect.Width;
		SLONG wwh = WorkWindowRect.Height;
		
		SetWorkWindowBounds(x, y, w - 1, h - 1);

		DrawModuleContent  (x + 1, y + 1, w, h);
		SetWorkWindowBounds(x + 1, y + 1, w, h);
		ShowWorkWindow(0);

		if (SHELL_ACTIVE && (LeftButton || RightButton))
		{
			//
			// The new mouse position...
			//

			mouse_x = MouseX - x;
			mouse_y = MouseY - y;
		}
		else
		{
			//
			// Stop dragging.
			//

			SetWorkWindowBounds(wwx,wwy,www,wwh);
		}
	}

	RedrawModuleContent = TRUE;
	RequestUpdate();

	return 0;
}









