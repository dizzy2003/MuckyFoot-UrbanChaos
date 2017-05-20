#include	"game.h"
#include	"supermap.h"
#include	"pap.h"
#include	"ob.h"
#include	"memory.h"

#ifdef _DEBUG
#define	FACET_REMOVAL_TEST		// if defined, still put removed facets into the map
#endif

#ifndef	PSX
void	calc_ladder_ends(SLONG *x1,SLONG *z1,SLONG *x2,SLONG *z2)
{
	SLONG	dx,dz;

	dx=*x2-*x1;
	dz=*z2-*z1;


	*x1+=dx/3;
	*z1+=dz/3;

	*x2-=dx/3;
	*z2-=dz/3;

	*x1+=dz>>3;
	*x2+=dz>>3;

	*z1-=dx>>3;
	*z2-=dx>>3;

}



//
// find an available block of links
//
SLONG	find_empty_facet_block(SLONG count)
{
	SLONG	c0,c1;

	//
	// Is there space at the end?
	//
		
	if((MAX_FACET_LINK)-next_facet_link>count)
	{
		next_facet_link+=count;
		return(next_facet_link-count);
	}
	else
	{
		//
		// search for gap
		//

//		return(0);
		for(c0=1;c0<next_facet_link;c0++)
		{
			if(facet_links[c0]==0)
			{
				SLONG	empty=1;
				for(c1=c0+1;(c1<next_facet_link) && (empty<count) ;c1++)
				{
					if(facet_links[c1])
						break;
					empty++;
				}
				if(empty==count)
					return(c0);
			}
		}
	}
	return(0);
}

//
// facet index is the block to insert a gap before
//
SLONG	garbage_collection(void)
{
	SLONG	index,next=1;
	SWORD	*mem;
	SLONG	x,z;
	SLONG	ret=0;
	UWORD	per_map[150],c0,saved=0;

	memset(per_map,0,300);

	mem=(SWORD*)MemAlloc(MAX_FACET_LINK*sizeof(SWORD));
	ASSERT(mem);
//	DebugText(" garbage collect nfl %d\n",next_facet_link);	

	for(x=0;x<PAP_SIZE_LO;x++)
	for(z=0;z<PAP_SIZE_LO;z++)
	{
		SLONG	count;
		if(index=PAP_2LO(x,z).ColVectHead)
		{

			PAP_2LO(x,z).ColVectHead=next;
#ifdef	SAVE_A_FEW_MORE_BYTES
			for(c0=1;c0<next;c0++)
			{
				SLONG	match=0;

				count=0;
				while(mem[c0+match]==facet_links[index+match] && (c0+match)<next)
				{
					count++;
					if(facet_links[index+match]<0)
					{
						saved+=count;
//						DebugText(" found match   saved %d (total %d)\n",count,saved);
						PAP_2LO(x,z).ColVectHead=c0;
						goto	repeated_duplicate;
					}
					match++;
				}
			}
#endif

			count=0;
			while(1)
			{
//				DebugText(" copy from %d to %d   (data %d) count %d \n",index,next,facet_links[index],count);
				count++;
				mem[next++]=facet_links[index++];
				if(facet_links[index-1]<0)
				{
					ASSERT(count<150);
					per_map[count]++;
					break;
				}
			}
#ifdef	SAVE_A_FEW_MORE_BYTES
repeated_duplicate:;
#endif
		}
	}

	for(c0=0;c0<150;c0++)
	{
		DebugText(" FACET_PER MAP (%d) ==%d \n",c0,per_map[c0]);
	}
	memcpy((UBYTE*)&facet_links[0],(UBYTE*) mem,next*sizeof(UWORD));
	next_facet_link=next;

	DebugText(" after nfl=%d +saved=%d\n",next_facet_link,next_facet_link+saved);
	
	for(x=0;x<PAP_SIZE_LO;x++)
	for(z=0;z<PAP_SIZE_LO;z++)
	{
		if(PAP_2LO(x,z).ColVectHead)
			ASSERT(facet_links[PAP_2LO(x,z).ColVectHead]!=0);
	}

	MemFree((UBYTE*)mem);
	
	return(0);
}

//
// create an extra facet for a block.
//

SLONG	create_extra_facet(SLONG facet,SLONG findex)
{
	SLONG	pos,count=1;
	SLONG	garbage=0;

	//
	// count number of facets on this facetlink (-ve is end flag)
	//
	pos=findex;
	while(facet_links[pos++]>0)
	{
		ASSERT(facet_links[pos-1]!=facet);
		count++;
	}
	ASSERT(abs(facet_links[pos-1])!=abs(facet));

	//
	// find place with count+1 available slots (if room just stick them on the end)
	//
try_again:;
	pos=find_empty_facet_block(count+1);
	if(pos)
	{
		SLONG	pos_copy;
		pos_copy=pos+1;
		//
		// copy to new place, and erase old place
		//
		while(count)
		{
			facet_links[pos_copy++]=facet_links[findex++];
			facet_links[findex-1]=0;
			count--;
		}
	}
	else
	{
		//
		// failed to find a gap so garbage collect
		//
		ASSERT(garbage==0);
		garbage++;
		garbage_collection();
		goto	try_again;
	}

	//
	// return place found for count+1 facets
	//
	return(pos);
}
#endif
#ifndef PSX
void	link_facet_to_mapwho(SLONG mx,SLONG mz,SLONG facet)
{
	SLONG	index;
	SLONG	pos;

//	ASSERT(facet!=17);
//	if(facet==2456)
//	{
//		ASSERT(0);
//	}

	if(mx==13 && mz==5)
		LogText(" add FACET %d to %d,%d\n",facet,mx,mz);

	if(mx<0||mx>PAP_SIZE_LO||mz<0||mz>PAP_SIZE_LO)
	{
		LogText("ERROR add facet %d to mx %d %d  nfl %d\n",facet,mx,mz,next_facet_link);
		return;
	}

//	DebugText(" add facet %d to mx %d %d  nfl %d link count %d\n",facet,mx,mz,next_facet_link,facet_link_count);

	facet_link_count++;

	if(facet_link_count>=MAX_FACET_LINK)
		return;

	if(index=PAP_2LO(mx,mz).ColVectHead)
	{
		//
		// mapwho allready contains some data
		//
		pos=create_extra_facet(facet,index);
		if(pos)
		{
//			ASSERT(pos!=129);
//			MAP2(mx,mz).ColVectHead=pos;
			PAP_2LO(mx,mz).ColVectHead=pos;
//			ASSERT(pos!=23);
			facet_links[pos]=facet;
		}
		else
			ASSERT(0);
	}
	else
	{

		//
		// no data there so start list
		//

		pos=find_empty_facet_block(1);
//		ASSERT(pos!=129);

		if(pos)
		{
//			MAP2(mx,mz).ColVectHead=pos;
			PAP_2LO(mx,mz).ColVectHead=pos;
//			ASSERT(pos!=23);
			facet_links[pos]=-facet;
		}
		else
		{
			ASSERT(0);
			// out of facet links
		}
	}
}

void	add_facet_to_map(SLONG facet)
{
	SLONG	x1,z1,x2,z2,dx,dz;
	SLONG	count;
	struct	DFacet	*p_f;

	p_f=&dfacets[facet];

//	if(facet==2456)
//	{
//		ASSERT(0);
//	}

	if(facet==288||facet==289)
		LogText("break");

	x1=p_f->x[0] << 8;
	x2=p_f->x[1] << 8;
	z1=p_f->z[0] << 8;
	z2=p_f->z[1] << 8;

	dx=x2-x1;
	dz=z2-z1;

	if(dx==0&&dz==0)
		return;

	switch(p_f->FacetType)
	{
		case	STOREY_TYPE_LADDER:
			{
				SLONG	y,extra_height;

				calc_ladder_ends(&x1,&z1,&x2,&z2);	
			}

			break;
	}


	{
		SLONG x;
		SLONG z;

		SLONG mx;
		SLONG mz;

		SLONG end_mx;
		SLONG end_mz;

		SLONG frac;

		SLONG xfrac;
		SLONG zfrac;

		#ifndef NDEBUG
		SLONG count = 0;
		#endif

		dx = x2 - x1;
		dz = z2 - z1;

		SLONG adx = abs(dx);
		SLONG adz = abs(dz);



		mx     = x1 >> PAP_SHIFT_LO;
		mz     = z1 >> PAP_SHIFT_LO;
		end_mx = x2 >> PAP_SHIFT_LO;
		end_mz = z2 >> PAP_SHIFT_LO;

		xfrac = x1 & ((1 << PAP_SHIFT_LO) - 1);
		zfrac = z1 & ((1 << PAP_SHIFT_LO) - 1);

		if (adx > adz)
		{
			frac = (dz << PAP_SHIFT_LO) / dx;

			if (dx > 0)
			{
				z  = z1;
				z -= frac * xfrac >> PAP_SHIFT_LO;
			}
			else
			{
				z  = z1;
				z += frac * ((1 << PAP_SHIFT_LO) - xfrac) >> PAP_SHIFT_LO;
			}

			while(1)
			{
				#ifndef NDEBUG
				ASSERT(count++ < 64);
				#endif

				if (WITHIN(mx, 0, PAP_SIZE_LO - 1) &&
					WITHIN(mz, 0, PAP_SIZE_LO - 1))
				{
					link_facet_to_mapwho(mx,mz,facet);
				}

				if (mx == end_mx &&
					mz == end_mz)
				{
					return;
				}

				//
				// Step in z.
				//

				if (dx > 0)
				{
					z += frac;
				}
				else
				{
					z -= frac;
				}

				if ((z >> PAP_SHIFT_LO) != mz)
				{
					//
					// Step up/down in z through another mapsquare.
					//

					mz = z >> PAP_SHIFT_LO;

					if (WITHIN(mx, 0, PAP_SIZE_LO - 1) &&
						WITHIN(mz, 0, PAP_SIZE_LO - 1))
					{
						link_facet_to_mapwho(mx,mz,facet);
					}
				}

				//
				// Step in x.
				//

				if (dx > 0)
				{
					mx += 1;
					if (mx > end_mx)
					{
						return;
					}
				}
				else
				{
					mx -= 1;
					if (mx < end_mx)
					{
						return;
					}
				}
			}
		}
		else
		{
			frac = (dx << PAP_SHIFT_LO) / dz;

			if (dz > 0)
			{
				x  = x1;
				x -= frac * zfrac >> PAP_SHIFT_LO;
			}
			else
			{
				x  = x1;
				x += frac * ((1 << PAP_SHIFT_LO) - zfrac) >> PAP_SHIFT_LO;
			}

			while(1)
			{
				#ifndef NDEBUG
				ASSERT(count++ < 64);
				#endif

				if (WITHIN(mx, 0, PAP_SIZE_LO - 1) &&
					WITHIN(mz, 0, PAP_SIZE_LO - 1))
				{
					link_facet_to_mapwho(mx,mz,facet);
				}

				if (mx == end_mx &&
					mz == end_mz)
				{
					return;
				}

				//
				// Step in x.
				//

				if (dz > 0)
				{
					x += frac;
				}
				else
				{
					x -= frac;
				}

				if ((x >> PAP_SHIFT_LO) != mx)
				{
					//
					// Step up/down in z through another mapsquare.
					//

					mx = x >> PAP_SHIFT_LO;

					if (WITHIN(mx, 0, PAP_SIZE_LO - 1) &&
						WITHIN(mz, 0, PAP_SIZE_LO - 1))
					{
						link_facet_to_mapwho(mx,mz,facet);
					}
				}

				//
				// Step in z.
				//

				if (dz > 0)
				{
					mz += 1;

					if (mz > end_mz)
					{
						return;
					}
				}
				else
				{
					mz -= 1;

					if (mz < end_mz)
					{
						return;
					}
				}
			}
		}
	}

/*

	dx=x2-x1;
	dz=z2-z1;

	if(abs(dz)>abs(dx))
	{

		//
		// step along dz
		//
		count=(abs(dz)+((1<<PAP_SHIFT_LO)-1))>>PAP_SHIFT_LO;
		

		if(dz<0)
			dz=-(1<<PAP_SHIFT_LO);
		else
			dz=(1<<PAP_SHIFT_LO);

		while(count--)
		{
			link_facet_to_mapwho(x1>>PAP_SHIFT_LO,z1>>PAP_SHIFT_LO,facet);

			z1+=dz;
		}
	}
	else
	{
		//
		// step along dx
		//
		count=(abs(dx)+((1<<PAP_SHIFT_LO)-1))>>PAP_SHIFT_LO;

		if(dx<0)
			dx=-(1<<PAP_SHIFT_LO);
		else
			dx=(1<<PAP_SHIFT_LO);

		while(count--)
		{

			link_facet_to_mapwho(x1>>PAP_SHIFT_LO,z1>>PAP_SHIFT_LO,facet);

			x1+=dx;
			
		}
	}
	*/
}

void	process_facet(SLONG facet)
{
	if(dfacets[facet].FacetType!=STOREY_TYPE_INSIDE)
		if(dfacets[facet].FacetType!=STOREY_TYPE_OINSIDE)
			if(dfacets[facet].FacetType!=STOREY_TYPE_INSIDE_DOOR)
				add_facet_to_map(facet);
}

void	process_building(SLONG build)
{
	SLONG	c0;
	struct	DBuilding *p_build;

	p_build=&dbuildings[build];

	for(c0=p_build->StartFacet;c0<p_build->EndFacet;c0++)
	{
		// only process if not marked as invisible
#ifndef FACET_REMOVAL_TEST
		if (dfacets[c0].FacetFlags & FACET_FLAG_INVISIBLE)
		{
			if (dbuildings[dfacets[c0].Building].Type == BUILDING_TYPE_CRATE_IN)
			{
				//
				// We must add the facet to the map for CRATE_INSIDEs because it make the
				// floor of Poshetas dissapear.
				//
			}
			else
			{
				continue;
			}
		}
#endif
		process_facet(c0);
	}
}

void	clear_colvects(void)
{
	SLONG	x,z;

	for(x=0;x<PAP_SIZE_LO;x++)
	for(z=0;z<PAP_SIZE_LO;z++)
	{
		PAP_2LO(x,z).ColVectHead=0;
		PAP_2LO(x,z).Walkable=0;
	}

}
void	clear_facet_links(void)
{
	SLONG	c0;
	for(c0=0;c0<MAX_FACET_LINK;c0++)
	{
		facet_links[c0]=0;
	}
}
#endif
void	attach_walkable_to_map(SLONG face)
{

	SLONG	x=0,z=0;
	SLONG	c0;

	if(face>0)
	{
		for(c0=0;c0<4;c0++)
		{
			x+=prim_points[prim_faces4[face].Points[c0]].X;
			z+=prim_points[prim_faces4[face].Points[c0]].Z;
		}

		x >>= 2;
		z >>= 2;

		x >>= PAP_SHIFT_LO;
		z >>= PAP_SHIFT_LO;

		SATURATE(x, 0, PAP_SIZE_LO - 1);
		SATURATE(z, 0, PAP_SIZE_LO - 1);

		prim_faces4[face].WALKABLE=PAP_2LO(x,z).Walkable;
		PAP_2LO(x,z).Walkable=face;
	}
	else
	{

		roof_faces4[-face].Next=PAP_2LO((roof_faces4[-face].RX&127)>>2,(roof_faces4[-face].RZ&127)>>2).Walkable;
		PAP_2LO((roof_faces4[-face].RX&127)>>2,(roof_faces4[-face].RZ&127)>>2).Walkable=face;
	}

}


void remove_walkable_from_map(SLONG face)
{
	SLONG	x=0,z=0;
	SLONG	c0;

	SWORD *prev;
	SWORD  next;
	SWORD count=50;

	for(c0=0;c0<4;c0++)
	{
		x+=prim_points[prim_faces4[face].Points[c0]].X;
		z+=prim_points[prim_faces4[face].Points[c0]].Z;
	}

	x >>= 2;
	z >>= 2;

	x >>= PAP_SHIFT_LO;
	z >>= PAP_SHIFT_LO;
	
	SATURATE(x, 0, PAP_SIZE_LO - 1);
	SATURATE(z, 0, PAP_SIZE_LO - 1);

	prev = &PAP_2LO(x,z).Walkable;
	next =  PAP_2LO(x,z).Walkable;

	while(count--)
	{
//		ASSERT(WITHIN(next, 1, next_prim_face4 - 1));

		if (next == face)
		{
			if(face<0)
			{
				*prev = roof_faces4[-next].Next;
			}
			else
			{
				*prev = prim_faces4[next].WALKABLE;

			}

			return;
		}
		if(next<0)
		{
			prev = &roof_faces4[-next].Next;
			next =  roof_faces4[-next].Next;
		}
		else
		{

			prev = &prim_faces4[next].WALKABLE;
			next =  prim_faces4[next].WALKABLE;
		}
	}
	//printf("Couldn't find face %d to remove.\n",face);
	ASSERT(0);
}


/*

NOGO flag is not used anymore

void	set_nogo_pap_flags(void)
{
	SLONG	x,z;

	for(x=0;x<PAP_SIZE_HI;x++)
	for(z=0;z<PAP_SIZE_HI;z++)
	{
		SLONG	min,max,h;

		h=PAP_hi[(x)&(PAP_SIZE_HI-1)][(z)&(PAP_SIZE_HI-1)].Alt;
		min=h;
		max=h;
		h=PAP_hi[(x+1)&(PAP_SIZE_HI-1)][(z)&(PAP_SIZE_HI-1)].Alt;
		min=MIN(h,min);
		max=MAX(h,max);
		h=PAP_hi[(x+1)&(PAP_SIZE_HI-1)][(z+1)&(PAP_SIZE_HI-1)].Alt;
		min=MIN(h,min);
		max=MAX(h,max);
		h=PAP_hi[(x)&(PAP_SIZE_HI-1)][(z+1)&(PAP_SIZE_HI-1)].Alt;
		min=MIN(h,min);
		max=MAX(h,max);

		if((abs(min-max))>30)
			PAP_hi[x][z].Flags|=PAP_FLAG_NOGO;
	}
}

*/

#ifndef PSX
static void mark_naughty_facets();

void	build_quick_city(void)
{
	SLONG	c0;

	clear_colvects();
	clear_facet_links();

	facet_link_count=0;
	next_facet_link=1;

	// mark those facets which can't be seen
	mark_naughty_facets();

	for(c0=1;c0<next_dbuilding;c0++)
	{
		process_building(c0);
	}
	garbage_collection();

	/*


	{
		SLONG	x,z;
		for(x=0;x<PAP_SIZE_LO;x++)
		for(z=0;z<PAP_SIZE_LO;z++)
		{
			SLONG	index,count=0;
			SLONG	exit=0;

			index=PAP_2LO(x,z).ColVectHead;
			while(!exit)
			{
				if(facet_links[index]<0)
					exit=1;
				index++;
				count++;
			}
			LogText(" map x %d z %d count %d \n",x,z,count);
		}
	}

	*/

	for(c0=1;c0<next_dwalkable;c0++)
	{
		SLONG	face;
		ASSERT(dwalkables[c0].StartFace4<=next_roof_face4);
		ASSERT(dwalkables[c0].EndFace4<=next_roof_face4);

		for(face=dwalkables[c0].StartFace4;face<dwalkables[c0].EndFace4;face++)
		{
/*
			prim_faces4[face].ThingIndex=dwalkables[c0].Building;
			attach_walkable_to_map(face);
			prim_faces4[face].FaceFlags|=FACE_FLAG_WALKABLE;
			prim_faces4[face].FaceFlags&=~FACE_FLAG_SMOOTH;	// This is used by moving walkables.
*/
			attach_walkable_to_map(-face);

		}
	}

	//
	// NOGO flags dont exist any more.
	//
	// set_nogo_pap_flags();
	//
}
#endif

// mark_naughty_facets
//
// remove any facets which can never be seen and clear the style value
// for subfacets which can never be seen

static bool facet_is_solid(const DFacet* pf);
static int compare_facets(const DFacet* pf1, const DFacet* pf2);
#ifndef	PSX
static void mark_naughty_facets()
{
	SLONG	ii;
	SLONG	jj;
	SLONG	cnt;

	// mark all the facets as live
	for (ii = 0; ii < next_dfacet; ii++)
	{
		dfacets[ii].FacetFlags &= ~FACET_FLAG_INVISIBLE;
	}

	// mark dead facets
	cnt = 0;
	for (ii = 0; ii < next_dfacet; ii++)
	{
		DFacet*	pf1 = &dfacets[ii];

		if(pf1->x[0]==pf1->x[1] && pf1->z[0]==pf1->z[1])
		{
			pf1->FacetFlags |= FACET_FLAG_INVISIBLE;
			continue;

		}
//		ASSERT(ii!=155 && ii!=157);

		if (pf1->FacetFlags & FACET_FLAG_INVISIBLE)				continue;
		if (!facet_is_solid(pf1))		continue;

		// compare against all others
		for (jj = ii + 1; jj < next_dfacet; jj++)
		{
			DFacet* pf2 = &dfacets[jj];

			

			if(pf2->x[0]==pf2->x[1] && pf2->z[0]==pf2->z[1])
			{
				pf2->FacetFlags |= FACET_FLAG_INVISIBLE;
				continue;

			}
			if (pf2->FacetFlags & FACET_FLAG_INVISIBLE)				continue;
			if (!facet_is_solid(pf2))	continue;


			switch (compare_facets(pf1, pf2))
			{
			case 1:
				pf1->FacetFlags |= FACET_FLAG_INVISIBLE;
				cnt++;
				break;

			case 2:
				pf2->FacetFlags |= FACET_FLAG_INVISIBLE;
				cnt++;
				break;

			case 3:
				pf1->FacetFlags |= FACET_FLAG_INVISIBLE;
				pf2->FacetFlags |= FACET_FLAG_INVISIBLE;
				cnt += 2;
				break;
			}
		}
	}

	TRACE("Removed %d invisible facets\n", cnt);
}

extern void MAV_remove_facet_car(SLONG x1, SLONG z1, SLONG x2, SLONG z2);

void BUILD_car_facets()
{
	for (int ii = 0; ii < next_dfacet; ii++)
	{
		DFacet*	pf = &dfacets[ii];

		if (pf->FacetFlags & FACET_FLAG_INVISIBLE)		continue;
		if (pf->FacetType == STOREY_TYPE_CABLE)			continue;

		MAV_remove_facet_car(pf->x[0], pf->z[0], pf->x[1], pf->z[1]);
	}
}

static bool facet_is_solid(const DFacet* pf)
{
	if (pf->FacetFlags & FACET_FLAG_2SIDED)		return false;
	if (pf->FacetFlags & FACET_FLAG_2TEXTURED)		return false;

	if ((pf->FacetType == STOREY_TYPE_JUST_COLLISION) ||
		(pf->FacetType == STOREY_TYPE_FENCE) ||
		(pf->FacetType == STOREY_TYPE_FENCE_FLAT) ||
		(pf->FacetType == STOREY_TYPE_FENCE_BRICK) ||
		(pf->FacetType == STOREY_TYPE_INSIDE) ||
		(pf->FacetType == STOREY_TYPE_INSIDE_DOOR) ||
		(pf->FacetType == STOREY_TYPE_OUTSIDE_DOOR) ||
		(pf->FacetType == STOREY_TYPE_LADDER) ||
		(pf->FacetType == STOREY_TYPE_CABLE))
	{
		return false;	// not solid
	}

	return true;
}
#endif

// compare_facets
//
// see if we can delete one or both of the facets because they hide each other
//
// note: takes no notice of the facet types; you should first check they are
// solid one-sided facets
//
// we test for multiple cases:
//
// two facets facing each other of same size (remove both)
// two facets facing each other one surrounding the other (remove smaller)
// two facets facing same way, one surrounding the other (remove smaller)
// two facets facing same way of same size (remove one at random)
//
// returns +1 to remove pf1, +2 to remove pf2
#ifndef	PSX
static int compare_facets(const DFacet* pf1, const DFacet* pf2)
{
	// are the facets parallel?
	SLONG	dx1 = pf1->x[1] - pf1->x[0];
	SLONG	dz1 = pf1->z[1] - pf1->z[0];

	SLONG	dx2 = pf2->x[1] - pf2->x[0];
	SLONG	dz2 = pf2->z[1] - pf2->z[0];

	if (dx1 * dz2 != dx2 * dz1)			return 0;

	// are the facets collinear?
	// check three cases - dz == 0, dx == 0 and neither == 0
	if (!dz1)
	{
		if (pf1->z[0] != pf2->z[0])		return 0;
	}
	else if (!dx1)
	{
		if (pf1->x[0] != pf2->x[0])		return 0;
	}
	else
	{
		// intercept point (with x axis) is at zi = z1 - x1 * dz / dx
		// giving zi * dx = z1 * dx - x1 * dz (nice and symettrical)
		// so we need (pf1->z[0] * dx1 - pf1->x[0] * dz1) / dx1 = (pf2->z[0] * dx2 - pf2->x[0] * dz2) / dx2
		// but we can multiply through by dx1 and dx2 so we can work in integers
		SLONG lhs = (pf1->z[0] * dx1 - pf1->x[0] * dz1) * dx2;
		SLONG rhs = (pf2->z[0] * dx2 - pf2->x[0] * dz2) * dx1;

		if (lhs != rhs)					return 0;
	}

	// extract start and end points which can be compared
	SLONG	s1,e1;
	SLONG	s2,e2;
	if (abs(dx1) > abs(dz1))
	{
		s1 = pf1->x[0];		e1 = pf1->x[1];
		s2 = pf2->x[0];		e2 = pf2->x[1];
	}
	else
	{
		s1 = pf1->z[0];		e1 = pf1->z[1];
		s2 = pf2->z[0];		e2 = pf2->z[1];
	}

	// remove either facet if it's null
	int		rc = 0;

	if (s1 == e1)		rc += 1;	// remove facet 1, it's null
	if (s2 == e2)		rc += 2;	// remove facet 2, it's null

	if (rc)				return rc;

	// are the facets facing the same way?
	SLONG	sgn1 = s1 - e1;
	SLONG	sgn2 = s2 - e2;
	bool	sameway = ((sgn1 ^ sgn2) >= 0);	// true iff (s1 - e1) and (s2 - e2) have same sign

	// get y at bottom of start and end, and heights
	SLONG	ys1 = pf1->Y[0];
	SLONG	ye1 = pf1->Y[1];
	SLONG	ys2 = pf2->Y[0];
	SLONG	ye2 = pf2->Y[1];
	SLONG	h1 = (pf1->Height / 4) * pf1->BlockHeight * 16;
	SLONG	h2 = (pf2->Height / 4) * pf2->BlockHeight * 16;

	// put start and end in order
	if (s1 > e1)
	{
		SWAP(s1,e1);
		SWAP(ys1,ye1);
	}
	if (s2 > e2)	
	{
		SWAP(s2,e2);
		SWAP(ys2,ye2);
	}

	// quick special case of exactly the same size
	if ((s1 == s2) && (e1 == e2) && (ys1 == ys2) && (ye1 == ye2) && (h1 == h2))
	{
		if (sameway)
		{
			// remove one at random
//			TRACE("Facets %d and %d are identical but face the same way - removing one at random\n", pf1 - dfacets, pf2 - dfacets);
			return (rand() & 128) ? 1 : 2;
		}
		else
		{
			// remove both
//			TRACE("Facets %d and %d are identical and cancel out - removing both\n", pf1 - dfacets, pf2 - dfacets);
			return 3;
		}
	}

	// equal extents along the ground?
	if ((s1 == s2) && (e1 == e2))
	{
//		TRACE("Facets %d and %d are identical on the ground\n", pf1 - dfacets, pf2 - dfacets);
//		TRACE("y1 - %d -> %d + %d\n", ys1, ye1, h1);
//		TRACE("y2 - %d -> %d + %d\n", ys2, ye2, h2);
		// yes - check for containment either way
		if ((ys1 <= ys2) && (ys1 + h1 >= ys2 + h2) && (ye1 <= ye2) && (ye1 + h1 >= ye2 + h2))
		{
//			TRACE("Removing 2\n");
			return 2;	// 1 contains 2
		}
		if ((ys2 <= ys1) && (ys2 + h2 >= ys1 + h1) && (ye2 <= ye1) && (ye2 + h2 >= ye1 + h1))
		{
//			TRACE("Removing 1\n");
			return 1;	// 2 contains 1
		}
		return 0;
	}

	// 1 contains 2 along ground?
	if ((s1 <= s2) && (e1 >= e2))
	{
//		TRACE("Facet %d contains %d on the ground\n", pf1 - dfacets, pf2 - dfacets);
//		TRACE("(%d - %d) contains (%d - %d)\n", s1,e1, s2,e2);
//		TRACE("y1 - %d -> %d + %d\n", ys1, ye1, h1);
//		TRACE("y2 - %d -> %d + %d\n", ys2, ye2, h2);

		// get ys1 and ye1 when s1 -> s2 and e1 -> e2
		ys1 += (s2 - s1) * (ye1 - ys1) / (e1 - s1);
		s1 = s2;

		ye1 += (e2 - e1) * (ye1 - ys1) / (e1 - s1);
		e1 = e2;

//		TRACE("NEW y1 - %d -> %d + %d\n", ys1, ye1, h1);

		// check for 1 contains 2
		if ((ys1 <= ys2) && (ys1 + h1 >= ys2 + h2) && (ye1 <= ye2) && (ye1 + h1 >= ye2 + h2))
		{
//			TRACE("Removing 2\n");
			return 2;
		}

		return 0;
	}

	// 2 contains 1 along ground?
	if ((s2 <= s1) && (e2 >= e1))
	{
//		TRACE("Facet %d is contained by %d on the ground\n", pf1 - dfacets, pf2 - dfacets);
//		TRACE("(%d - %d) is contained by (%d - %d)\n", s1,e1, s2,e2);
//		TRACE("y1 - %d -> %d + %d\n", ys1, ye1, h1);
//		TRACE("y2 - %d -> %d + %d\n", ys2, ye2, h2);

		// get ys2 and ye2 when s2 -> s1 and e2 -> e1
		ys2 += (s1 - s2) * (ye2 - ys2) / (e2 - s2);
		s2 = s1;

		ye2 += (e1 - e2) * (ye2 - ys2) / (e2 - s2);
		e2 = e1;

//		TRACE("NEW y2 - %d -> %d + %d\n", ys2, ye2, h2);

		// check for 2 contains 1
		if ((ys2 <= ys1) && (ys2 + h2 >= ys1 + h1) && (ye2 <= ye1) && (ye2 + h2 >= ye1 + h1))
		{
//			TRACE("Removing 1\n");
			return 1;
		}

		return 0;
	}

	// neither contains the other
	return 0;
}
#endif