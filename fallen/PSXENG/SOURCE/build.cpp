//
// Draws buildings.
//

#include "game.h"
#include "c:\fallen\psxeng\headers\engine.h"
//#include "c:\fallen\editor\headers\poly.h"
//#include "light.h"
#include "build.h"

SLONG	check_vect_with_gamut(SLONG x1,SLONG z1,SLONG x2,SLONG z2)
{
	SLONG	step_x,step_z;
	SLONG	count;
/*
	if((z1<NGAMUT_zmin && z2<NGAMUT_zmin) || (z1>NGAMUT_zmax && z2>NGAMUT_zmax))
		return(0); //dont draw

	if((x1<NGAMUT_xmin && x2<NGAMUT_xmin) || (x1>NGAMUT_xmax && x2>NGAMUT_xmax))
		return(0); //dont draw
*/ 
	step_x=x2-x1;
	step_z=z2-z1;



	if(abs(step_z)>=abs(step_x))
	{

		count=abs(step_z);
		if(count==0)
			count=1;
		step_x=(step_x<<8)/count;
		count>>=8;
		if(count==0)
			count=1;


		if(step_z>0)
			step_z=1;
		else
			step_z=-1;


		x1<<=8;
		z1>>=8;
		while(count)
		{

			if(z1>=NGAMUT_zmin&&z1<=NGAMUT_zmax)
			{
				SLONG	temp_x;
				temp_x=x1>>16;
				if(temp_x>=NGAMUT_gamut[z1].xmin&&temp_x<=NGAMUT_gamut[z1].xmax)
					return(1); //draw this facet
			}
			x1+=step_x;
			z1+=step_z;
			count--;
		}
	}
	else
	{
		count=abs(step_x);
		if(count==0)
			count=1;
		step_z=(step_z<<8)/count;
		count>>=8;
		if(count==0)
			count=1;


		if(step_x>0)
			step_x=1;
		else
			step_x=-1;


		x1>>=8;
		z1<<=8;
		while(count)
		{
			SLONG	temp_z;

			temp_z=z1>>16;

			if(temp_z>NGAMUT_zmin&&temp_z<NGAMUT_zmax)
			{
				if(x1>=NGAMUT_gamut[temp_z].xmin && x1<=NGAMUT_gamut[temp_z].xmax)
				{
					return(1); //draw this facet
				}
			}
			x1+=step_x;
			z1+=step_z;
			count--;
		}

	}
	return(0);
}
#ifndef	PSX
SLONG	check_col_vect(SLONG vect)
{
	CollisionVect *p_vect;
	SLONG	draw;
	SLONG	x1,z1,x2,z2;
	SLONG	tx1,tz1,tx2,tz2;
	p_vect = &col_vects[vect];


	x1=p_vect->X[0];
	z1=p_vect->Z[0];


	x2=p_vect->X[1];
	z2=p_vect->Z[1];

	tx1=x1>>8;
	tz1=z1>>8;
	tx2=x2>>8;
	tz2=z2>>8;

	if( (tz1<NGAMUT_zmin && tz2<NGAMUT_zmin) || (tz1>NGAMUT_zmax && tz2>NGAMUT_zmax)) //|| (tx1<NGAMUT_xmin && tx2<NGAMUT_xmin) || (tx1>NGAMUT_xmax && tx2>NGAMUT_xmax) )
	{
		draw=0;
	}
	else
	{
		draw=check_vect_with_gamut(x1,z1,x2,z2);
	}

	//
	// no points found inside the gamut
	//
	if(0)		   
	{
		LINE_F2	*p;

		p=(LINE_F2 *)the_display.CurrentPrim;

		setLineF2(p);
		if(draw==0)
			setRGB0(p,0,0,255);
		else
			setRGB0(p,255,255,255);
		setXY2(p,p_vect->X[0]>>7, p_vect->Z[0]>>7,p_vect->X[1]>>7, p_vect->Z[1]>>7);

		addPrim(&the_display.CurrentDisplayBuffer->ot[4095],p);

	}
	return(draw);

}
SLONG	draw_bound_box(struct BoundBox *p_box,SLONG r,SLONG g,SLONG b)
{
	return(0);

	POLY_F4	*p;
	p=(POLY_F4 *)the_display.CurrentPrim;

	setPolyF4(p);
	setRGB0(p,r,g,b);
	setXY4(p,p_box->MinX<<1,p_box->MinZ<<1,p_box->MaxX<<1,p_box->MinZ<<1,p_box->MinX<<1,p_box->MaxZ<<1,p_box->MaxX<<1,p_box->MaxZ<<1);

	the_display.CurrentPrim+=sizeof(POLY_F4);
	addPrim(&the_display.CurrentDisplayBuffer->ot[4095],p);
}

SLONG	check_roof_facet(SLONG bbox)
{
	struct	BoundBox *p_box;
	SLONG	x,z;
	p_box=&roof_bounds[bbox];
		return(1);

	if(p_box->Y>NGAMUT_Ymax||p_box->Y<NGAMUT_Ymin)
	{

		draw_bound_box(p_box,255,0,0);
		return(0);
	}

	if(p_box->MinZ>NGAMUT_zmax || p_box->MaxZ<NGAMUT_zmin)
	{
		draw_bound_box(p_box,128,0,0);
		return(0);
	}

	if(p_box->MinX>NGAMUT_xmax || p_box->MaxX<NGAMUT_xmin)
	{
		draw_bound_box(p_box,64,0,0);
		return(0);
	}



	for(x=p_box->MinX;x<p_box->MaxX;x+=2)
	for(z=p_box->MinZ;z<p_box->MaxZ;z+=2)
	{
		if(z>NGAMUT_zmin&&z<NGAMUT_zmax)
		{
			if(x>NGAMUT_gamut[z].xmin && x<=NGAMUT_gamut[z].xmax)
			{
				draw_bound_box(p_box,255,255,255);

				return(1); //draw the bastard
			}
		}
		
	}
	draw_bound_box(p_box,0,0,255);
	return(0); //dont draw
}




SLONG	check_facet(SLONG bx,SLONG by,SLONG bz,SLONG bf_index)
{
	BuildingFacet  *bf;

	SLONG sp;
	SLONG ep;
	SLONG i;
	SLONG	pass=0;



	bf = &building_facets[bf_index];

	if(bf->ColVect>0)
	{
		if(check_col_vect(bf->ColVect)==0)
		{
			return(0); //dont draw this facet
		}

	}
	else
	if(bf->ColVect<0)
	{
		if(check_roof_facet(-bf->ColVect)==0)
			return(0); //dont draw this facet

	}

		return(1);

	sp = bf->StartPoint;
	ep = bf->EndPoint;

	for (i = sp; i < ep; i++)
	{
		SLONG	y;

		y=by+prim_points[i].Y;

		if(y>NGAMUT_Ymin && y <NGAMUT_Ymax)
		{
			pass=1;
			break;
		}
	}

	if(pass)
	{
		/*
		do more tests
		*/

		return(1);
	}
	else
	{
		//
		// facet off screen
		//
		return(0);
	}

}

void BUILD_draw(Thing *p_thing)
{
	SLONG i;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	SLONG page;
	SLONG backface_cull;
	ULONG shadow;
	ULONG face_colour;
	ULONG face_specular;

	SLONG bx = (p_thing->WorldPos.X >> 8);
	SLONG by = (p_thing->WorldPos.Y >> 8);
	SLONG bz = (p_thing->WorldPos.Z >> 8);

	SLONG bo_index;
	SLONG bf_index;

	BuildingFacet  *bf;			 
	BuildingObject *bo;
	static	SLONG	most_quads=300;
	SLONG	count_quad=0;
	
	bo_index =  p_thing->Index;
	bo       = &building_objects[bo_index];

	//
	// Points out of the ambient light.
	//
/*
	shadow =
		((LIGHT_amb_colour.red   >> 1) << 16) |
		((LIGHT_amb_colour.green >> 1) <<  8) |
		((LIGHT_amb_colour.blue  >> 1) <<  0);
  */
	//
	// The ambient light colour.
	//

	ULONG colour;
	ULONG specular;
/*
	LIGHT_get_d3d_colour(
		LIGHT_amb_colour,
	   &colour,
	   &specular);
	   */

	//
	// ONLY TRIANGLES USE THESE VALUES. MAKE THEM STAND OUT.
	// 

	colour   = 0xffffffff;
	specular = 0xffffffff;

	//
	// Draw each facet.
	//

	bf_index = bo->FacetHead;

	while(bf_index)
	{
		SLONG	do_facet=1;
		SLONG	max_z=-999999;

		bf = &building_facets[bf_index];

		//
		// Rotate all the points.
		//

		sp = bf->StartPoint;
		ep = bf->EndPoint;

		POLY_buffer_upto = 0;
		if(ep-sp>8)
		{
			do_facet=check_facet(bx,by,bz,bf_index);

		}

		if(do_facet)
		{
			for (i = sp; i < ep; i++)
			{
				ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

				pp = &POLY_buffer[POLY_buffer_upto++];

				if(prim_points[i].Y + by>NGAMUT_Ymax+260)
				{
					pp->clip=0;
				}
				else

				{
					POLY_transform(
						prim_points[i].X + bx,
						prim_points[i].Y + by,
						prim_points[i].Z + bz,
						pp);
				}

				if (pp->clip & POLY_CLIP_TRANSFORMED)
				{
//					if(pp->Z>max_z)
//						max_z=pp->Z;
	/*
					LIGHT_get_d3d_colour(
						LIGHT_building_point[i],
					   &pp->colour,
					   &pp->specular);

					POLY_fadeout_point(pp);
					*/
				}
			}
/*
			max_z>>=1;
			if(max_z<0)
				max_z=0;
			if(max_z>4095)
				max_z=4095;
			max_z=4095-max_z;
*/

			//
			// Draw all the quads.
			//

			count_quad+=bf->EndFace4-bf->StartFace4;
			for (i = bf->StartFace4; i < bf->EndFace4; i++)
			{
				p_f4 = &prim_faces4[i];

				p0 = p_f4->Points[0] - sp;
				p1 = p_f4->Points[2] - sp;
				p2 = p_f4->Points[1] - sp;
				p3 = p_f4->Points[3] - sp;
				
				ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

				quad[0] = &POLY_buffer[p0];
				quad[1] = &POLY_buffer[p1];
				quad[2] = &POLY_buffer[p2];
				quad[3] = &POLY_buffer[p3];

				backface_cull = !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED);

				if (backface_cull<0)
				{
					//
					// Should this poly be backface culled?
					//


					//
					// The texture page to use.
					//
	//void	add_quad(POLY_Point *quad,MapElement *me)
					{
						POLY_FT4	*p;
						SLONG	z;

						if(the_display.CurrentPrim+sizeof(*p)>&the_display.CurrentDisplayBuffer->PrimMem[BUCKET_MEM])
							return;

						p=(POLY_FT4 *)the_display.CurrentPrim;

						setPolyFT4(p);
						setUV4(p,p_f4->UV[0][0],p_f4->UV[0][1],
								p_f4->UV[2][0],p_f4->UV[2][1],
								p_f4->UV[1][0],p_f4->UV[1][1],
								p_f4->UV[3][0],p_f4->UV[3][1]);

						setRGB0(p,128,128,128);
						setXY4(p,POLY_buffer[p0].X,POLY_buffer[p0].Y,
							POLY_buffer[p1].X,POLY_buffer[p1].Y,
							POLY_buffer[p2].X,POLY_buffer[p2].Y,
							POLY_buffer[p3].X,POLY_buffer[p3].Y);

						z=POLY_buffer[p0].Z;
						if(z<POLY_buffer[p1].Z)
							z=POLY_buffer[p1].Z;
						if(z<POLY_buffer[p2].Z)
							z=POLY_buffer[p2].Z;
						if(z<POLY_buffer[p3].Z)
							z=POLY_buffer[p3].Z;
						z>>=1;
						z=get_z_sort(z);

//						z=max_z; //POLY_buffer[p0].Z;


						page = p_f4->TexturePage;
						p->tpage=psx_tpages[page];	   

						p->clut =psx_tpages_clut[page];


						addPrim(&the_display.CurrentDisplayBuffer->ot[z],p);
						the_display.CurrentPrim+=sizeof(POLY_FT4);

					}

					/*
					if (p_f4->DrawFlags & POLY_FLAG_MASKED)
					{
						page = POLY_PAGE_MASKED;
					}
					else
					{
						page = p_f4->TexturePage;
					}
					*/

				}
			}

			//
			// Draw all the triangles.
			//
	/*
			for (i = bf->StartFace3; i < bf->EndFace3; i++)
			{
				p_f3 = &prim_faces3[i];

				p0 = p_f3->Points[0] - sp;
				p1 = p_f3->Points[1] - sp;
				p2 = p_f3->Points[2] - sp;
				
				ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

				tri[0] = &POLY_buffer[p0];
				tri[1] = &POLY_buffer[p1];
				tri[2] = &POLY_buffer[p2];

				if (POLY_valid_triangle(tri))
				{
					//
					// Should this poly be backface culled?
					//

					backface_cull = !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED);

					//
					// The texture page to use.
					//

					if (p_f3->DrawFlags & POLY_FLAG_MASKED)
					{
						page = POLY_PAGE_MASKED;
					}
					else
					{
						page = p_f3->TexturePage;
					}					

					//
					// Texture the triangle.
					// 

					tri[0]->u = float(p_f3->UV[0][0]) * (1.0F / 256.0F);
					tri[0]->v = float(p_f3->UV[0][1]) * (1.0F / 256.0F);

					tri[1]->u = float(p_f3->UV[1][0]) * (1.0F / 256.0F);
					tri[1]->v = float(p_f3->UV[1][1]) * (1.0F / 256.0F);

					tri[2]->u = float(p_f3->UV[2][0]) * (1.0F / 256.0F);
					tri[2]->v = float(p_f3->UV[2][1]) * (1.0F / 256.0F);

					POLY_add_triangle(tri, page, backface_cull);
				}
			}
		*/
		}
		
		bf_index = bf->NextFacet;
	}
	if(count_quad>most_quads)
	{
		most_quads=count_quad;
//		printf("most quad %d \n",most_quads);
	}

}


void BUILD_draw_inside()
{
#ifdef	POO
	Thing *p_thing = TO_THING(INDOORS_THING);

	SLONG i;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;
	
	float max_height;

	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	ULONG amb_colour;
	ULONG amb_specular;

	float bx = float(p_thing->WorldPos.X >> 8);
	float by = float(p_thing->WorldPos.Y >> 8);
	float bz = float(p_thing->WorldPos.Z >> 8);

	SLONG bo_index;
	SLONG bf_index;

	BuildingFacet  *bf;
	BuildingObject *bo;
	
	bo_index =  p_thing->Index;
	bo       = &building_objects[bo_index];

	//
	// The ambient light colour.
	//

	ULONG colour;
	ULONG specular;
/*
	LIGHT_get_d3d_colour(
		LIGHT_amb_colour,
	   &colour,
	   &specular);
*/

	//
	// Draw each facet.
	//

	bf_index = bo->FacetHead;

	while(bf_index)
	{
		bf = &building_facets[bf_index];

		if (bf->FacetFlags & FACET_FLAG_ROOF)
		{
			max_height = float(INDOORS_HEIGHT_FLOOR + 32);
		}
		else
		{
			if (building_list[p_thing->BuildingList].BuildingType == BUILDING_TYPE_WAREHOUSE)
			{
				max_height = float(INDOORS_HEIGHT_CEILING + 256 + 32);
			}
			else
			{
				max_height = float(INDOORS_HEIGHT_CEILING + 32);
			}
		}

		//
		// Rotate all the points.
		//

		sp = bf->StartPoint;
		ep = bf->EndPoint;

		POLY_buffer_upto = 0;

		for (i = sp; i < ep; i++)
		{
			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			pp = &POLY_buffer[POLY_buffer_upto++];

			if (AENG_dx_prim_points[i].Y + by <= max_height)
			{
				POLY_transform(
					AENG_dx_prim_points[i].X + bx,
					AENG_dx_prim_points[i].Y + by,
					AENG_dx_prim_points[i].Z + bz,
					pp);
				
				if (pp->clip & POLY_CLIP_TRANSFORMED)
				{
					pp->colour   = colour;
					pp->specular = specular;

					POLY_fadeout_point(pp);
				}
			}
			else
			{
				pp->clip = 0;
			}
		}

		//
		// Draw all the quads.
		//

		for (i = bf->StartFace4; i < bf->EndFace4; i++)
		{
			p_f4 = &prim_faces4[i];

			p0 = p_f4->Points[0] - sp;
			p1 = p_f4->Points[1] - sp;
			p2 = p_f4->Points[2] - sp;
			p3 = p_f4->Points[3] - sp;
			
			ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

			quad[0] = &POLY_buffer[p0];
			quad[1] = &POLY_buffer[p1];
			quad[2] = &POLY_buffer[p2];
			quad[3] = &POLY_buffer[p3];

			if (POLY_valid_quad(quad))
			{
				quad[0]->u = float(p_f4->UV[0][0]) * (1.0F / 256.0F);
				quad[0]->v = float(p_f4->UV[0][1]) * (1.0F / 256.0F);

				quad[1]->u = float(p_f4->UV[1][0]) * (1.0F / 256.0F);
				quad[1]->v = float(p_f4->UV[1][1]) * (1.0F / 256.0F);

				quad[2]->u = float(p_f4->UV[2][0]) * (1.0F / 256.0F);
				quad[2]->v = float(p_f4->UV[2][1]) * (1.0F / 256.0F);

				quad[3]->u = float(p_f4->UV[3][0]) * (1.0F / 256.0F);
				quad[3]->v = float(p_f4->UV[3][1]) * (1.0F / 256.0F);

				POLY_add_quad(quad, p_f4->TexturePage, TRUE);
			}
		}


		//
		// Draw all the quads.
		//

		for (i = bf->StartFace3; i < bf->EndFace3; i++)
		{
			p_f3 = &prim_faces3[i];

			p0 = p_f3->Points[0] - sp;
			p1 = p_f3->Points[1] - sp;
			p2 = p_f3->Points[2] - sp;
			
			ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

			tri[0] = &POLY_buffer[p0];
			tri[1] = &POLY_buffer[p1];
			tri[2] = &POLY_buffer[p2];

			if (POLY_valid_triangle(tri))
			{
				tri[0]->u = float(p_f3->UV[0][0]) * (1.0F / 256.0F);
				tri[0]->v = float(p_f3->UV[0][1]) * (1.0F / 256.0F);

				tri[1]->u = float(p_f3->UV[1][0]) * (1.0F / 256.0F);
				tri[1]->v = float(p_f3->UV[1][1]) * (1.0F / 256.0F);

				tri[2]->u = float(p_f3->UV[2][0]) * (1.0F / 256.0F);
				tri[2]->v = float(p_f3->UV[2][1]) * (1.0F / 256.0F);

				POLY_add_triangle(tri, p_f3->TexturePage, TRUE);
			}
		}
		
		bf_index = bf->NextFacet;
	}
#endif
}

#endif





