// Thing.cpp
// Guy Simmons, 14th October 1997.

#include	"Editor.hpp"

#include	"DarkCity.h"
#include	"Structs.h"
#include	"Thing.h"
#include	"map.h"
#include	"prim.h"


extern	void	apply_light_to_map(SLONG x,SLONG y,SLONG z,SLONG bright);




//****************************************
// old style thing functions
// courtesy of miked
//****************************************
//its only the editor, no need for 2 way link lists


//data
struct	MapThing	map_things[MAX_MAP_THINGS];


UWORD	find_empty_map_thing(void)
{
	SLONG	c0;
	for(c0=1;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==0)
		{
			map_things[c0].MapParent=0;
			map_things[c0].MapChild=0;
			return(c0);
		}
	}
	return(0);
}

UWORD	count_empty_map_things(void)
{
	SLONG	c0,count=0;
	for(c0=1;c0<MAX_MAP_THINGS;c0++)
	{
		if(map_things[c0].Type==0)
		{
			count++;
		}
	}
	return(count);
}


void	add_thing_to_edit_map(SLONG x,SLONG z,UWORD	thing)
{
	if(x<0||x>=EDIT_MAP_WIDTH||z<0||z>=EDIT_MAP_DEPTH)
	{
		ERROR_MSG(1," add thing off map");
		return;
	}
	if(edit_map[x][z].MapThingIndex)
	{
		map_things[edit_map[x][z].MapThingIndex].MapParent=thing;
		map_things[thing].MapChild=edit_map[x][z].MapThingIndex;
		map_things[thing].MapParent=0;
		edit_map[x][z].MapThingIndex=thing;
	}
	else
	{
		edit_map[x][z].MapThingIndex=thing;
//		LogText(" add thing x %d z %d \n",x,z);
		map_things[thing].MapChild=0;
		map_things[thing].MapParent=0;
	}
}

void	delete_thing_from_edit_map(SLONG x,SLONG z,UWORD	thing)
{
	if(x<0||x>=EDIT_MAP_WIDTH||z<0||z>=EDIT_MAP_DEPTH)
	{
		ERROR_MSG(1," del thing off map");
		return;
	}

	 //does the thing we are deleting have a parent
	if(map_things[thing].MapParent)
	{
		map_things[map_things[thing].MapParent].MapChild=map_things[thing].MapChild; 

		//if thing has a child then must update the childs parent
		if(map_things[thing].MapChild)
		{ 
			map_things[map_things[thing].MapChild].MapParent = map_things[thing].MapParent;
		}
	}
	else
	//removing directly off mapwho
	{
		edit_map[x][z].MapThingIndex=map_things[thing].MapChild;
		if(map_things[thing].MapChild)
			map_things[map_things[thing].MapChild].MapParent=0;
	}

}

SLONG	move_thing_on_cells(UWORD thing,SLONG x,SLONG y,SLONG z)
{
	if( (x>>ELE_SHIFT) != (map_things[thing].X>>ELE_SHIFT)||	
		(z>>ELE_SHIFT) != (map_things[thing].Z>>ELE_SHIFT) )
	{
		delete_thing_from_edit_map((map_things[thing].X>>ELE_SHIFT),(map_things[thing].Z>>ELE_SHIFT),thing);
		add_thing_to_edit_map(x>>ELE_SHIFT,(z>>ELE_SHIFT),thing);
	}
	map_things[thing].X=x;
	map_things[thing].Y=y;
	map_things[thing].Z=z;
	return(1);
}


void	delete_thing(SWORD index)
{
	struct	MapThing	*p_mthing;
	p_mthing=TO_MTHING(index);

	switch(p_mthing->Type)
	{
		case MAP_THING_TYPE_LIGHT:
			apply_light_to_map(p_mthing->X,p_mthing->Y,p_mthing->Z,-p_mthing->IndexOther);
			break;
	}
	delete_thing_from_edit_map(p_mthing->X>>ELE_SHIFT,p_mthing->Z>>ELE_SHIFT,index);	
	p_mthing->Type=0;
}

