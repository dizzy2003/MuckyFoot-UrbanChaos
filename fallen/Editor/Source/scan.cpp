
#include	"Editor.hpp"
#include	"c:\fallen\headers\memory.h"


void	(*scan_function)(SLONG face,SLONG x,SLONG y,SLONG z,SLONG extra);
extern	SWORD	SelectFlag; //edit.h


void	scan_a_prim_at(UWORD	prim,SLONG x,SLONG y,SLONG z)
{
	SLONG	c0;
	struct	PrimObject	*p_obj;


	p_obj    =	&prim_objects[prim];

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		scan_function(c0,x,y,z,0);
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		scan_function(-c0,x,y,z,0);
	}
}

void	scan_a_building(UWORD	prim,SLONG x,SLONG y,SLONG z)
{
	
	SLONG	c0;
	struct	BuildingObject	*p_obj;


	p_obj    =	&building_objects[prim];

	if(p_obj->EndFace4)
	for(c0=p_obj->StartFace4;c0<p_obj->EndFace4;c0++)
	{
		scan_function(c0,x,y,z,0);
	}

	if(p_obj->EndFace3)
	for(c0=p_obj->StartFace3;c0<p_obj->EndFace3;c0++)
	{
		scan_function(-c0,x,y,z,0);
	}
}






void	scan_map_thing(SLONG	map_thing)
{
	SLONG				c0,c1;
	struct Matrix33		r_matrix;
	struct MapThing		*p_mthing;
	UBYTE	prim_done[1000];

	memset(prim_done,0,1000);


	p_mthing	=	TO_MTHING(map_thing);
	switch(p_mthing->Type)
	{
		case	MAP_THING_TYPE_PRIM:
			//3ds Prim Mesh 
			if(p_mthing->IndexOther<1000)
			if(!prim_done[p_mthing->IndexOther])
			{
				prim_done[p_mthing->IndexOther]=1;
				scan_a_prim_at(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z);
			}
			break;
		case	MAP_THING_TYPE_MULTI_PRIM:
//			scan_a_multi_prim_at(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z);
			break;
		case	MAP_THING_TYPE_ROT_MULTI:
		case	MAP_THING_TYPE_SPRITE:
		case	MAP_THING_TYPE_AGENT:
			break;
		case	MAP_THING_TYPE_BUILDING:
			scan_a_building(p_mthing->IndexOther,p_mthing->X,p_mthing->Y,p_mthing->Z);
			break;

	}
}

void	scan_game_map_thing(SLONG	map_thing)
{
	SLONG				c0,c1;
	struct Matrix33		r_matrix;
	Thing		*p_thing;
	UBYTE	prim_done[1000];

	memset(prim_done,0,1000);


	p_thing	=	TO_THING(map_thing);
	switch(p_thing->DrawType)
	{
		//case	MAP_THING_TYPE_PRIM:
		case	DT_PRIM:
			//3ds Prim Mesh 
			if(p_thing->Index<1000)
			if(!prim_done[p_thing->Index])
			{
				prim_done[p_thing->Index]=1;
				scan_a_prim_at(p_thing->Index,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
			}
			break;
		case	DT_BUILDING:
			scan_a_building(p_thing->Index,p_thing->WorldPos.X,p_thing->WorldPos.Y,p_thing->WorldPos.Z);
			break;

	}
}

void	scan_linked_background(void)
{
	SWORD	index;
	struct	MapThing	*p_thing;
	index=background_prim;
	while(index)
	{
		p_thing=TO_MTHING(index);
		scan_a_prim_at(p_thing->IndexOther,p_thing->X,p_thing->Y,p_thing->Z);
		index=p_thing->IndexNext;
	}
}

void	scan_map(void)
{
	SLONG	dx,dy,dz;
	SLONG	mx,my,mz;
	struct	EditMapElement	*p_ele;
	UWORD	index,count;
	

	mx=(engine.X>>8)>>ELE_SHIFT;
	my=(engine.Y>>8)>>ELE_SHIFT;
	mz=(engine.Z>>8)>>ELE_SHIFT;

	for(dz=0;dz<EDIT_MAP_DEPTH;dz++)
	for(dx=0;dx<EDIT_MAP_WIDTH;dx++)
	{
		index=edit_map[(dx)][(dz)].MapThingIndex;
		count=0;
		while(index&&count++<20)
		{
			scan_map_thing(index);
			index=map_things[index].MapChild;
		}
		
		count=0;
		index=MAP2(dx,dz).MapWho;
		while(index&&count++<20)
		{
			scan_game_map_thing(index);
			index=TO_THING(index)->Child;
		}
		
	}

	if(background_prim)
		scan_linked_background();

	SelectFlag=0;
}

