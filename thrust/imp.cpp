//
// Imports SEX files.
//

#include "always.h"
#include "imp.h"



//
// Returns TRUE if the normals are similar enough.
//

SLONG IMP_norm_similar(
		float     nx1,
		float     ny1,
		float     nz1,
		float     nx2,
		float     ny2,
		float     nz2)
{
	float dprod;

	dprod = nx1*nx2 + ny1*ny2 + nz1*nz2;

	if (dprod > 0.999F)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//
// Normalises a vector.
//

void IMP_normalise_vector(
		float *vx,
		float *vy,
		float *vz)
{
	float len = sqrt(*vx * *vx + *vy * *vy + *vz * *vz);

	if (len < 0.00001F)
	{
	   *vx = 1.0F;
	   *vy = 1.0F;
	   *vz = 1.0F;
	}
	else
	{
		float overlen = 1.0F / len;

	   *vx *= overlen;
	   *vy *= overlen;
	   *vz *= overlen;
	}
}




IMP_Mesh IMP_load(CBYTE *fname, float scale)
{
	SLONG i;
	SLONG j;
	SLONG k;
	SLONG l;

	float x;
	float y;
	float z;

	float r;
	float g;
	float b;

	float u;
	float v;

	float sh;
	float ss;

	SLONG m;
	SLONG v1;
	SLONG v2;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	SLONG t1;
	SLONG t2;
	SLONG t3;
	SLONG e1;
	SLONG e2;
	SLONG e3;
	SLONG p1o;
	SLONG p2o;
	SLONG v1o;
	SLONG v2o;

	CBYTE sided[16];
	CBYTE alpha[16];
	CBYTE tname[128];
	CBYTE bname[128];

	SLONG shared[3];

	SLONG group;
	SLONG match;

	float pivot_x;
	float pivot_y;
	float pivot_z;
	SLONG pivot_valid = FALSE;
	
	SLONG offset_vert;
	SLONG offset_tvert;
	SLONG offset_mat;

	SLONG max_mats;
	SLONG max_verts;
	SLONG max_tverts;
	SLONG max_faces;
	SLONG max_sverts;
	SLONG max_quads;
	SLONG max_edges;

	Point3d fs_norm[3];
	Point3d fs_u   [3];
	Point3d fs_v   [3];

	IMP_Svert fs[3];

	IMP_Mat   *im;
	IMP_Vert  *iv;
	IMP_Tvert *it;
	IMP_Face  *ic;
	IMP_Face  *ico;
	IMP_Svert *is;
	IMP_Quad  *iq;
	IMP_Edge  *ie;
	
	IMP_Mesh ans;
	CBYTE    line[256];
	CBYTE    oname[32];

	FILE *handle;

	//
	// Initialise the mesh.
	//

	max_mats   = 4;
	max_verts  = 32;
	max_tverts = 32;
	max_faces  = 32;
	max_sverts = 32;
	max_quads  = 16;
	max_edges  = 32;

	memset(&ans, 0, sizeof(ans));

	ans.mat   = (IMP_Mat   *) malloc(sizeof(IMP_Mat  ) * max_mats  );
	ans.vert  = (IMP_Vert  *) malloc(sizeof(IMP_Vert ) * max_verts );
	ans.tvert = (IMP_Tvert *) malloc(sizeof(IMP_Tvert) * max_tverts);
	ans.face  = (IMP_Face  *) malloc(sizeof(IMP_Face ) * max_faces );
	ans.svert = (IMP_Svert *) malloc(sizeof(IMP_Svert) * max_sverts);
	ans.quad  = (IMP_Quad  *) malloc(sizeof(IMP_Quad ) * max_quads );
	ans.edge  = (IMP_Edge  *) malloc(sizeof(IMP_Edge ) * max_edges );

	if (ans.mat   == NULL ||
		ans.vert  == NULL ||
		ans.tvert == NULL ||
		ans.face  == NULL ||
		ans.svert == NULL ||
		ans.quad  == NULL ||
		ans.edge  == NULL)
	{
		goto file_error;
	}

	//
	// Open the file.
	//

	handle = fopen(fname, "rb");

	if (!handle)
	{
		goto file_error;
	}

	//
	// Read the data.
	//

	while(fgets(line, 256, handle))
	{
		if (line[0] == '#')
		{
			//
			// Ignore comments.
			//

			continue;
		}

		match = sscanf(line, "Triangle mesh: %s", oname);

		if (match == 1)
		{
			if (ans.name[0] == '\000')
			{
				//
				// This is the first mesh in the SEX file- use its name,
				//

				strcpy(ans.name, oname);
			}

			offset_vert  = ans.num_verts;
			offset_tvert = ans.num_tverts;
			offset_mat   = ans.num_mats;

			continue;
		}

		match = sscanf(line, "Pivot: (%f,%f,%f)", &x, &y, &z);

		if (match == 3)
		{
			//
			// The pivot of the current object.
			//

			if (!pivot_valid)
			{
				pivot_x = x;
				pivot_y = y;
				pivot_z = z;

				pivot_valid = TRUE;
			}
			else
			{
				//
				// Only use the first pivot.
				//
			}

			continue;
		}

		match = sscanf(line, "Material: DiffuseRGB (%f,%f,%f), shininess %f, shinstr %f, %s sided, %s alpha, diffuse %s bumpmap %s", &r, &b, &g, &sh, &ss, sided, alpha, tname, bname);

		if (match == 9)
		{
			//
			// Found a new material.
			//

			if (ans.num_mats >= max_mats)
			{
				//
				// We have to allocate a larger materials array and copy
				// over the data.
				//

				max_mats *= 2;

				ans.mat = (IMP_Mat *) realloc(ans.mat, sizeof(IMP_Mat) * max_mats);

				if (ans.mat == NULL)
				{
					goto file_error;
				}
			}

			im = &ans.mat[ans.num_mats++];

			im->r           = r;
			im->g           = g;
			im->b           = b;
			im->shininess   = sh;
			im->shinstr     = ss;
			im->alpha       = (strcmp(alpha, "Filtered")  == 0) ? IMP_ALPHA_FILTERED : IMP_ALPHA_ADDITIVE;
			im->sided       = (strcmp(sided, "Single")    == 0) ? IMP_SIDED_SINGLE   : IMP_SIDED_DOUBLE;
			im->has_texture = (strcmp(tname, "none") != 0);
			im->has_bumpmap = (strcmp(bname, "none") != 0);

			strncpy(im->tname, tname, 32);
			strncpy(im->bname, bname, 32);

			continue;
		}

		match = sscanf(line, "Vertex: (%f,%f,%f)", &x, &y, &z);

		if (match == 3)
		{
			//
			// Found a point. Convert from 3ds orientation to a sensible one.
			//

			SWAP_FL(y, z);
			z = -z;
			x = -x;

			x *= scale;
			y *= scale;
			z *= scale;

			if (ans.num_verts >= max_verts)
			{
				//
				// Lengthen the array.
				//

				max_verts *= 2;

				ans.vert = (IMP_Vert *) realloc(ans.vert, sizeof(IMP_Vert) * max_verts);

				if (ans.vert == NULL)
				{
					goto file_error;
				}
			}

			iv = &ans.vert[ans.num_verts++];

			iv->x = x;
			iv->y = y;
			iv->z = z;

			continue;
		}

		match = sscanf(line, "Texture Vertex: (%f,%f)", &u, &v);

		if (match == 2)
		{
			//
			// Found a texture vertex.
			//

			if (ans.num_tverts >= max_tverts)
			{
				//
				// Lengthen the array.
				//

				max_tverts *= 2;

				ans.tvert = (IMP_Tvert *) realloc(ans.tvert, sizeof(IMP_Tvert) * max_tverts);

				if (ans.tvert == NULL)
				{
					goto file_error;
				}
			}

			it = &ans.tvert[ans.num_tverts++];

			it->u = u;
			it->v = v;

			continue;
		}

		match = sscanf(line, "Face: Material %d xyz (%d,%d,%d) uv (%d,%d,%d) edge (%d,%d,%d) group %d", &m, &p1, &p2, &p3, &t1, &t2, &t3, &e1, &e2, &e3, &group);

		if (match == 11)
		{
			//
			// Found a face.
			//

			if (ans.num_faces >= max_faces)
			{
				//
				// We have to allocate a larger faceerials array and copy
				// over the data.
				//

				max_faces *= 2;

				ans.face = (IMP_Face *) realloc(ans.face, sizeof(IMP_Face) * max_faces);

				if (ans.face == NULL)
				{
					goto file_error;
				}
			}

			p1 += offset_vert;
			p2 += offset_vert;
			p3 += offset_vert;

			t1 += offset_tvert;
			t2 += offset_tvert;
			t3 += offset_tvert;

			m  += offset_mat;

			ASSERT(WITHIN(p1, 0, ans.num_verts - 1));
			ASSERT(WITHIN(p2, 0, ans.num_verts - 1));
			ASSERT(WITHIN(p3, 0, ans.num_verts - 1));

			ASSERT(WITHIN(t1, 0, ans.num_tverts - 1));
			ASSERT(WITHIN(t2, 0, ans.num_tverts - 1));
			ASSERT(WITHIN(t3, 0, ans.num_tverts - 1));

			ASSERT(WITHIN(m, 0, ans.num_mats - 1));

			if (!ans.mat[m].has_texture &&
				!ans.mat[m].has_bumpmap)
			{
				//
				// Set all the uv points to (0,0)- so that the sverts can
				// be shared across all faces.
				//

				t1 = 0;
				t2 = 0;
				t3 = 0;
			}

			//
			// Add the face to the mesh.
			//

			ic = &ans.face[ans.num_faces++];

			ic->v[0]  = p1;
			ic->v[1]  = p2;
			ic->v[2]  = p3;
			ic->t[0]  = t1;
			ic->t[1]  = t2;
			ic->t[2]  = t3;
			ic->mat   = m;
			ic->group = group;
			ic->flag  = 0;

			if (e1) {ic->flag |= IMP_FACE_FLAG_EDGE_A;}
			if (e2) {ic->flag |= IMP_FACE_FLAG_EDGE_B;}
			if (e3) {ic->flag |= IMP_FACE_FLAG_EDGE_C;}

			//
			// Calculate the face normal.
			//

			{
				IMP_Vert *v1 = &ans.vert[p1];
				IMP_Vert *v2 = &ans.vert[p2];
				IMP_Vert *v3 = &ans.vert[p3];

				float ax = v2->x - v1->x;
				float ay = v2->y - v1->y;
				float az = v2->z - v1->z;

				float bx = v3->x - v1->x;
				float by = v3->y - v1->y;
				float bz = v3->z - v1->z;

				float nx = ay*bz - az*by;
				float ny = az*bx - ax*bz;
				float nz = ax*by - ay*bx;

				IMP_normalise_vector(
				   &nx,
				   &ny,
				   &nz);

				ic->nx = nx;
				ic->ny = ny;
				ic->nz = nz;
			}

			//
			// Calculate the vectors that lie along the u and v axis of the texture.
			//

			{
				IMP_Vert *iv1 = &ans.vert[ic->v[0]];
				IMP_Vert *iv2 = &ans.vert[ic->v[1]];
				IMP_Vert *iv3 = &ans.vert[ic->v[2]];

				IMP_Tvert *it1 = &ans.tvert[ic->t[0]];
				IMP_Tvert *it2 = &ans.tvert[ic->t[1]];
				IMP_Tvert *it3 = &ans.tvert[ic->t[2]];

				float x1 = iv2->x - iv1->x;
				float y1 = iv2->y - iv1->y;
				float z1 = iv2->z - iv1->z;

				float x2 = iv3->x - iv1->x;
				float y2 = iv3->y - iv1->y;
				float z2 = iv3->z - iv1->z;

				float u1 = it2->u - it1->u;
				float v1 = it2->v - it1->v;

				float u2 = it3->u - it1->u;
				float v2 = it3->v - it1->v;

				float ucrossv     = u1*v2 - u2*v1;
				float overucrossv = 1.0F / ucrossv;

				ic->dxdu = (x1*v2 - x2*v1) * overucrossv;
				ic->dydu = (y1*v2 - y2*v1) * overucrossv;
				ic->dzdu = (z1*v2 - z2*v1) * overucrossv;

				ic->dxdv = (x1*u2 - x2*u1) * -overucrossv;
				ic->dydv = (y1*u2 - y2*u1) * -overucrossv;
				ic->dzdv = (z1*u2 - z2*u1) * -overucrossv;

				IMP_normalise_vector(
				   &ic->dxdu,
				   &ic->dydu,
				   &ic->dzdu);

				IMP_normalise_vector(
				   &ic->dxdv,
				   &ic->dydv,
				   &ic->dzdv);
			}

			continue;
		}
	}

	//
	// All ok.
	//

	fclose(handle);

	//
	// Construct the shared vertices.
	//

	for (i = 0; i < ans.num_faces; i++)
	{
		ic = &ans.face[i];

		for (j = 0; j < 3; j++)
		{
			ASSERT(WITHIN(ic->v[j], 0, ans.num_verts  - 1));
			ASSERT(WITHIN(ic->t[j], 0, ans.num_tverts - 1));

			//
			// Create this shared vertex.
			//

			fs[j].vert = ic->v[j];
			fs[j].u    = ans.tvert[ic->t[j]].u;
			fs[j].v    = ans.tvert[ic->t[j]].v;
			fs[j].mat  = ic->mat;
			fs[j].nx   = 0.0F;
			fs[j].ny   = 0.0F;
			fs[j].nz   = 0.0F;
			fs[j].dxdu = 0.0F;
			fs[j].dydu = 0.0F;
			fs[j].dzdu = 0.0F;
			fs[j].dxdv = 0.0F;
			fs[j].dydv = 0.0F;
			fs[j].dzdv = 0.0F;

			//
			// Initialise the normal and the (u,v) vectors
			//

			fs_norm[j].x = 0;
			fs_norm[j].y = 0;
			fs_norm[j].z = 0;

			fs_u[j].x = 0;
			fs_u[j].y = 0;
			fs_u[j].z = 0;

			fs_v[j].x = 0;
			fs_v[j].y = 0;
			fs_v[j].z = 0;

			shared[j] = 0;
		}

		//
		// Build the normal of the shared vertices.
		//
		// Look through all the other faces to find faces of the
		// same smoothing group that share this vertex.
		//

		for (j = 0; j < ans.num_faces; j++)
		{
			ico = &ans.face[j];

			//
			// A face can belong to no smoothing group- i.e. have
			// a group of zero. Check for this case!
			//

			if ((ico->group & ic->group) || (ico == ic /* In case the face has no smoothing group */ ))
			{
				//
				// These faces belong to the same smoothing group.
				// Do they share any vertices?
				//

				for (k = 0; k < 3; k++)
				for (l = 0; l < 3; l++)
				{
					if (ic->v[k] == ico->v[l])
					{
						//
						// Update the normal at this shared vertex.
						//

						fs_norm[k].x += ico->nx;
						fs_norm[k].y += ico->ny;
						fs_norm[k].z += ico->nz;

						//
						// Update the u,v vectors at this shared vertex.
						//

						fs_u[k].x += ico->dxdu;
						fs_u[k].y += ico->dydu;
						fs_u[k].z += ico->dzdu;

						fs_v[k].x += ico->dxdv;
						fs_v[k].y += ico->dydv;
						fs_v[k].z += ico->dzdv;

						shared[k] += 1;
					}
				}
			}
		}

		//
		// Normalise the shared vertex normals and put the u,v vectors into
		// the plane of the normal.
		//

		for (j = 0; j < 3; j++)
		{
			ASSERT(shared[j] > 0);

			if (shared[j] > 1)
			{
				IMP_normalise_vector(
				   &fs_norm[j].x,
				   &fs_norm[j].y,
				   &fs_norm[j].z);
			}

			//
			// Into the plane of the normal.
			//

			float dprod;

			dprod =
				fs_norm[j].x * fs_u[j].x + 
				fs_norm[j].y * fs_u[j].y +
				fs_norm[j].z * fs_u[j].z;

			fs_u[j].x -= dprod * fs_norm[j].x;
			fs_u[j].y -= dprod * fs_norm[j].y;
			fs_u[j].z -= dprod * fs_norm[j].z;

			dprod =
				fs_norm[j].x * fs_v[j].x + 
				fs_norm[j].y * fs_v[j].y +
				fs_norm[j].z * fs_v[j].z;

			fs_v[j].x -= dprod * fs_norm[j].x;
			fs_v[j].y -= dprod * fs_norm[j].y;
			fs_v[j].z -= dprod * fs_norm[j].z;

			//
			// Normalise...
			//

			IMP_normalise_vector(
			   &fs_u[j].x,
			   &fs_u[j].y,
			   &fs_u[j].z);

			IMP_normalise_vector(
			   &fs_v[j].x,
			   &fs_v[j].y,
			   &fs_v[j].z);
		}

		//
		// Look for sverts in the array that match.
		//

		for (j = 0; j < 3; j++)
		{
			for (k = ans.num_sverts - 1; k >= 0; k--)
			{
				is = &ans.svert[k];

				if (is->vert == fs[j].vert &&
					is->mat  == fs[j].mat  &&
					is->u    == fs[j].u    &&
					is->v    == fs[j].v)
				{
					//
					// Almost found a match! If the normal is the same, then we can use it.
					//

					if (IMP_norm_similar(
							is->nx,
							is->ny,
							is->nz,
							fs_norm[j].x,
							fs_norm[j].y,
							fs_norm[j].z))
					{
						//
						// The normals are similar enough. Just use this shared vertex.
						// Assume that the u,v vectors are going to be the same too...
						//

						ic->s[j] = k;

						goto found_matching_svert;
					}
				}
			}

			//
			// There is no svert in the array that matches our one. We
			// have to create a new svert.
			//

			if (ans.num_sverts >= max_sverts)
			{
				//
				// Lengthen the array.
				//

				max_sverts *= 2;

				ans.svert = (IMP_Svert *) realloc(ans.svert, sizeof(IMP_Svert) * max_sverts);

				if (ans.svert == NULL)
				{
					goto file_error;
				}
			}

			fs[j].nx = fs_norm[j].x;
			fs[j].ny = fs_norm[j].y;
			fs[j].nz = fs_norm[j].z;

			fs[j].dxdu = fs_u[j].x;
			fs[j].dydu = fs_u[j].y;
			fs[j].dzdu = fs_u[j].z;

			fs[j].dxdv = fs_v[j].x;
			fs[j].dydv = fs_v[j].y;
			fs[j].dzdv = fs_v[j].z;

			ans.svert[ans.num_sverts] = fs[j];
			ic->s[j]                  = ans.num_sverts;
			ans.num_sverts           += 1;

		  found_matching_svert:;
		}
	}

	//
	// Finds the quads in the mesh.
	//

	for (i = 0; i < ans.num_faces; i++)
	{
		ic = &ans.face[i];

		if ((ic->flag & (IMP_FACE_FLAG_EDGE_A|IMP_FACE_FLAG_EDGE_B|IMP_FACE_FLAG_EDGE_C)) == (IMP_FACE_FLAG_EDGE_A|IMP_FACE_FLAG_EDGE_B|IMP_FACE_FLAG_EDGE_C))
		{
			//
			// This face is a triangle.
			//

			continue;
		}

		if (ic->flag & IMP_FACE_FLAG_QUADDED)
		{
			//
			// Already part of a quad.
			//

			continue;
		}

		for (j = 0; j < 3; j++)
		{
			if (ic->flag & (IMP_FACE_FLAG_EDGE << j))
			{
				//
				// This edge is okay.
				//
			}
			else
			{
				//
				// This is the shared edge of a quad. What two verts does it use?
				//

				p1 = j + 0;
				p2 = j + 1;

				if (p2 == 3) {p2 = 0;}

				v1 = ic->v[p1];
				v2 = ic->v[p2];
				
				//
				// Look for another triangle that uses this edge.
				//

				for (k = i + 1; k < ans.num_faces; k++)
				{	
					ico = &ans.face[k];

					if ((ico->flag & (IMP_FACE_FLAG_EDGE_A|IMP_FACE_FLAG_EDGE_B|IMP_FACE_FLAG_EDGE_C)) == (IMP_FACE_FLAG_EDGE_A|IMP_FACE_FLAG_EDGE_B|IMP_FACE_FLAG_EDGE_C))
					{
						//
						// This face is a triangle.
						//

						continue;
					}

					if (ico->flag & IMP_FACE_FLAG_QUADDED)
					{
						//
						// Already part of a quad.
						//

						continue;
					}

					//
					// All quads must be co-planar
					//

					if (IMP_norm_similar(ic->nx, ic->ny, ic->nz, ico->nx, ico->ny, ico->nz))
					{
						continue;
					}

					for (l = 0; l < 3; l++)
					{
						if (ico->flag & (IMP_FACE_FLAG_EDGE << l))
						{
							//
							// This is a real edge.
							//
						}
						else
						{
							//
							// This edge is the diagonal of a quad. Is it our one?
							//

							p1o = j + 0;
							p2o = j + 1;

							if (p2o == 3) {p2o = 0;}

							v1o = ico->v[p1o];
							v2o = ico->v[p2o];

							if (v2o == v1 &&
								v1o == v2)
							{
								//
								// These two triangles are part of a quad togther.
								//

								if (ans.num_quads >= max_quads)
								{
									max_quads *= 2;

									ans.quad = (IMP_Quad *) realloc(ans.quad, sizeof(IMP_Quad) * max_quads);

									if (ans.quad == NULL)
									{
										goto file_error;
									}
								}

								iq = &ans.quad[ans.num_quads++];

								iq->v[0] = ic->v [(j + 1) % 3];
								iq->v[1] = ic->v [(j + 2) % 3];
								iq->v[2] = ic->v [(j + 0) % 3];
								iq->v[3] = ico->v[(l + 2) % 3];

								//
								// Mark the two faces concerned as being part of a quad.
								//

								ic->flag  |= IMP_FACE_FLAG_QUADDED;
								ico->flag |= IMP_FACE_FLAG_QUADDED;

								goto found_quad;
							}
						}
					}
				}
			}
		}

	  found_quad:;
	}

	//
	// Find the edges of the mesh.
	//

	for (i = 0; i < ans.num_faces; i++)
	{
		ic = &ans.face[i];

		//
		// For each edge...
		//

		for (j = 0; j < 3; j++)
		{
			p1 = j + 0;
			p2 = j + 1;

			if (p2 == 3)
			{
				p2 = 0;
			}

			v1 = ic->v[p1];
			v2 = ic->v[p2];

			//
			// Is there an edge between these two vertices already?
			//

			for (k = ans.num_edges - 1; k >= 0; k--)
			{
				ie = &ans.edge[k];

				if (ie->v1 == v2 &&
					ie->v2 == v1)
				{
					//
					// We have found the other half of our edge!
					//

					if (ie->f2 == 0xffff)
					{
						//
						// And the second edge hasn't been taken yet!
						//

						ie->f2 = i;

						goto found_edge;
					}
				}
			}

			//
			// We must create a new edge.
			//

			if (ans.num_edges >= max_edges)
			{
				max_edges *= 2;

				ans.edge = (IMP_Edge *) realloc(ans.edge, sizeof(IMP_Edge) * max_edges);

				if (ans.edge == NULL)
				{
					goto file_error;
				}
			}

			ans.edge[ans.num_edges].v1 = v1;
			ans.edge[ans.num_edges].v2 = v2;
			ans.edge[ans.num_edges].f1 = i;
			ans.edge[ans.num_edges].f2 = 0xffff;	// 0xffff => No buddy found yet.

			ans.num_edges += 1;

		  found_edge:;
		}
	}

	ans.valid = TRUE;

	return ans;

  file_error:;
	
	//
	// Decallocate any memory.
	//

	IMP_free(&ans);

	if (handle)
	{
		fclose(handle);
	}

	memset(&ans, 0, sizeof(ans));

	ans.valid = FALSE;

	return ans;
}


void IMP_free(IMP_Mesh *im)
{
	if (im->mat  ) {free(im->mat  ); im->mat   = NULL;}
	if (im->vert ) {free(im->vert ); im->vert  = NULL;}
	if (im->tvert) {free(im->tvert); im->tvert = NULL;}
	if (im->face ) {free(im->face ); im->face  = NULL;}
	if (im->svert) {free(im->svert); im->svert = NULL;}
	if (im->quad ) {free(im->quad ); im->quad  = NULL;}
}
