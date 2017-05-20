//
// Animals...
//

#ifndef TARGET_DC


#include "game.h"
#include "animal.h"
#include "canid.h"
#include "statedef.h"
#include "C:\fallen\DDEngine\Headers\Matrix.h"
#include "C:\fallen\Editor\Headers\prim.h"
#include "C:\fallen\DDEngine\Headers\figure.h"
#include "C:\fallen\DDEngine\Headers\poly.h"
#include "night.h"
#include	"memory.h"

//
// Custom types
//

GenusFunctions ANIMAL_functions[ANIMAL_NUMBER] =
{
	{ANIMAL_NONE,	NULL},
	{ANIMAL_CANID,	CANID_state_function}
};
/*
struct AnimalBody {
	SBYTE	start;
	SBYTE	size;
//	float	x[10],y[10],z[10];
};
*/

typedef void (*RegisterFn)();

//
// Globals
//

//struct GameKeyFrameChunk animal_anim_chunk[16];
//struct AnimalBody animal_bodies[16];
//SBYTE animal_anim_chunk[16];


extern struct GameKeyFrameChunk game_chunk[MAX_GAME_CHUNKS];


//SLONG body_counter, current_type;

//
// Prototypes
//

SLONG	load_anim_system(struct GameKeyFrameChunk *game_chunk,CBYTE	*name,SLONG peep=0);


//--------------------------------------------------------------------------
// Body part registration scheme...

//
// Registers a specific body part
//

//GameKeyFrameChunk *ANIMAL_register(char *filename, float x, float y, float z) {

/*
GameKeyFrameChunk *ANIMAL_register(char *filename) {
	GameKeyFrameChunk *the_chunk;

	the_chunk=&animal_anim_chunk[body_counter];
	load_anim_system(the_chunk,filename);
/+	animal_bodies[current_type].x[body_counter]=x;
	animal_bodies[current_type].y[body_counter]=y;
	animal_bodies[current_type].z[body_counter]=z;
	+/
	body_counter++;
	animal_bodies[current_type].size++;
	return the_chunk;
}
*/

//
// Registers a given animal
//
/*
void init_body_parts(SLONG animaltype, RegisterFn fn) {
	current_type=animaltype;
	animal_bodies[animaltype].start=body_counter;
	animal_bodies[animaltype].size=0;
	fn();
}
*/
//
// Zeros out the animals in the 'the_game' structure.
// Loads the animprims
//

void init_animals(void)
{
	memset((UBYTE*)ANIMALS, 0, sizeof(ANIMALS));

	ANIMAL_COUNT = 0;
//	body_counter=0;

//	load_anim_system(&animal_anim_chunk[ANIMAL_CANID],"data\\dog.all");
//	init_body_parts(ANIMAL_CANID,CANID_register);

}


//--------------------------------------------------------------------------
// Init'ing instances

//
// Sets up the drawtween for a specific bodypart
//

DrawTween *alloc_tween(SLONG type, SLONG part) {
	SLONG chunk;
	DrawTween *dt;

	ASSERT(0); //better add an extra game_chunk if your going to put animals in
	return(0);

//	chunk=animal_bodies[type].start+part;
//	chunk=animal_bodies[type];
	chunk=6;// heh
	dt = alloc_draw_tween(DT_ANIMAL_PRIM);

	if (dt) {
		dt->Angle    = 0;
		dt->Roll     = 0;
		dt->Tilt     = 0;

		dt->Angle			=	0;
		dt->Roll			=	0;
		dt->Tilt			=	0;
		dt->AnimTween		=	0;
		dt->TweenStage		=	0;
		dt->NextFrame		=	NULL;
		dt->QueuedFrame		=	NULL;
/*		dt->TheChunk		=	&animal_anim_chunk[chunk];
		dt->CurrentFrame	=	animal_anim_chunk[chunk].AnimList[1];*/
/*		dt->TheChunk		=	&game_chunk[6];
		dt->CurrentFrame	=	game_chunk[6].AnimList[1];*/
		dt->TheChunk		=	&game_chunk[chunk];
		dt->CurrentFrame	=	game_chunk[chunk].AnimList[1];
		if(dt->CurrentFrame) {
			dt->NextFrame	=	dt->CurrentFrame->NextFrame;
		}
		dt->FrameIndex		=	0;
		dt->Flags			=	0;


	}

	return dt;

}

//
// Creates a new animal of the given type.
//

Thing *alloc_animal(UBYTE type)
{		 

	SLONG i;

	Thing    *p_thing;
	Animal   *p_animal;
	DrawTween *dt;

	THING_INDEX t_index;
	SLONG       a_index;

	ASSERT(WITHIN(type, 1, ANIMAL_NUMBER - 1));
	ASSERT(0);
	return(0);

	//
	// Look for an unused animal structure.
	//

	for (i = 1; i < MAX_ANIMALS; i++)
	{
		if (ANIMALS[i].AnimalType == ANIMAL_NONE)
		{
			a_index = i;
			
			goto found_animal;
		}
	}

	//
	// There are no spare animal structures.
	//

	TRACE("Run out of animal structures.");

	return NULL;

  found_animal:

	//
	// Find the drawtween structures for the animal.
	//
	dt = alloc_tween(type,0);
	if (dt == NULL) {
		// Could not allocate a drawtween structure.
		TRACE("Run out of drawtween structures.");

		// Free up the animal structure we allocated.
		TO_ANIMAL(a_index)->AnimalType = ANIMAL_NONE;

		return NULL;
	}
//	TO_ANIMAL(a_index)->dts[0]=dt; // backup? :P actually animal_get_drawtween needs it

	t_index = alloc_primary_thing(CLASS_ANIMAL);

	if (t_index)
	{
		p_thing  = TO_THING(t_index);
		p_animal = TO_ANIMAL(a_index);

		//
		// Link the animal to the thing.
		//

		p_thing->Class        = CLASS_ANIMAL;
		p_thing->Genus.Animal = p_animal;
		p_animal->AnimalType  = type;

		//
		// Set the draw type and the drawtween stuff.
		//

		p_thing->DrawType  = DT_ANIMAL_PRIM;
//		p_thing->DrawType  = DT_ANIM_PRIM;
		p_thing->Draw.Tweened = dt;
		
		return p_thing;
	}
	else
	{
		//
		// Free up the animal structure and the
		// drawtween structure we allocated.
		//

		TO_ANIMAL(a_index)->AnimalType = ANIMAL_NONE;

		free_draw_tween(dt);
//		for (i=0;i<animal_bodies[type].size;i++)
//		  free_draw_tween(TO_ANIMAL(a_index)->dts[i]);
	}
}

void free_animal(Thing *p_thing) {
	SLONG i;
	Animal *animal = ANIMAL_get_animal(p_thing);
	ASSERT(0);
	return;

	//
	// Free all those drawtweens...
	//

	free_draw_tween(p_thing->Draw.Tweened);
//	for (i=0;i<animal_bodies[animal->AnimalType].size;i++)
//	  free_draw_tween(animal->dts[i]);

	
	//
	// Free the animal structure and the thing structure.
	//

	animal->AnimalType  = ANIMAL_NONE;
	ANIMAL_COUNT       -= 1;

	free_thing(p_thing);
}


Thing *ANIMAL_create(GameCoord pos, UBYTE type)
{
	Thing *p_thing = alloc_animal(type);
	return(0);

	if (p_thing != NULL)
	{
		p_thing->WorldPos = pos;

		add_thing_to_map(p_thing);

		//
		// Make it initialise itself.
		//

		set_state_function(p_thing, STATE_INIT);
	}

	return p_thing;
}

//
//  do animation
//

UBYTE ANIMAL_animatetween(DrawTween *draw_info) {
	SLONG tween_step;
	ASSERT(0);

//	tween_step=256/(draw_info->CurrentFrame->TweenStep+1);
	tween_step=draw_info->CurrentFrame->TweenStep<<1;

	tween_step = (tween_step*TICK_RATIO)>>TICK_SHIFT;
	if(tween_step==0)
		tween_step=1;
	draw_info->AnimTween += tween_step; //256/(draw_info->CurrentFrame->TweenStep+1);

	if(draw_info->AnimTween>256) {
		draw_info->AnimTween-=256;

SLONG advance_keyframe(DrawTween *draw_info);

		return advance_keyframe(draw_info); // 1 = anim over
	}
	return 0;
}

UBYTE ANIMAL_animate(Thing *animal) {
	UBYTE	     i,j=0;
	ASSERT(0);
//	for (i=0;i<animal_bodies[animal->Genus.Animal->AnimalType].size;i++)
//		j|=ANIMAL_animatetween(animal->Genus.Animal->dts[i]);
//	j=ANIMAL_animatetween(animal->Genus.Animal->dts[0]);
	j=ANIMAL_animatetween(animal->Draw.Tweened);
	return j;
}


//
// Change the animation
//

void	ANIMAL_set_anim(Thing *thing, SLONG anim) {
	Animal *animal = ANIMAL_get_animal(thing);
	UBYTE i;
	SLONG chunk;
	DrawTween *dt;
	ASSERT(0);
	return;

//	chunk=animal_bodies[animal->AnimalType].start;
	chunk=6;
//	for (i=0;i<animal_bodies[animal->AnimalType].size;i++)
	i=0;
	{
//		dt=animal->dts[i];
		dt=thing->Draw.Tweened;
		dt->AnimTween	 = 0;
		dt->NextFrame	 = NULL;
		dt->QueuedFrame	 = NULL;
//		dt->CurrentFrame	 = animal_anim_chunk[chunk].AnimList[anim];
		dt->CurrentFrame	 = game_chunk[chunk].AnimList[anim];
		dt->CurrentAnim	 = anim;
		dt->FrameIndex	 = 0;

		if (dt->CurrentFrame)
			dt->NextFrame = dt->CurrentFrame->NextFrame;
		chunk++;
	}

}

Animal *ANIMAL_get_animal(struct Thing *animal_thing)
{
	Animal *animal;

	ASSERT(WITHIN(animal_thing, TO_THING(1),  TO_THING(MAX_THINGS)));
	ASSERT(animal_thing->Class == CLASS_ANIMAL);

	animal = animal_thing->Genus.Animal;

	ASSERT(WITHIN(animal, TO_ANIMAL(1), TO_ANIMAL(MAX_ANIMALS - 1)));

	return animal;
}
/*
DrawTween *ANIMAL_get_drawtween(Animal *animal) {
	return animal->dts[0];
}
*/
/*
SLONG ANIMAL_body_size(Animal *animal) {
	return animal_bodies[animal->AnimalType].size;
}
*/


/*
extern void FIGURE_rotate_obj(SLONG xangle,SLONG yangle,SLONG zangle, Matrix33 *r3) ;
extern void FIGURE_draw_prim_tween(SLONG prim, SLONG x, SLONG y, SLONG z, SLONG tween, struct GameKeyFrameElement *anim_info, struct GameKeyFrameElement *anim_info_next, struct Matrix33 *rot_mat,
		SLONG off_dx, SLONG off_dy, SLONG off_dz, ULONG colour, ULONG specular);
-> jcl <- don't use FIGURE_draw_prim_tween() here - it's changed...
void ANIM_obj_draw_diddled(Thing *p_thing,DrawTween *dt,float xd,float yd,float zd)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG i;

	ULONG colour;
	ULONG specular;

	Matrix33 r_matrix;
	float matrix[9];

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;


	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		return;
	}

	//
	// The offset to keep the locked limb in the same place.
	//

	dx = 0;
	dy = 0;
	dz = 0;

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		(dt->Roll+2048)&2047,
	   &r_matrix);

	for (i=0;i<9;i++) matrix[i]=r_matrix.M[i&3][i>>2];
	MATRIX_MUL(matrix,xd,yd,zd);

	//
	// What colour do we draw the figure?
	//

	NIGHT_get_d3d_colour(
		NIGHT_get_light_at(
			(p_thing->WorldPos.X >> 8),
			(p_thing->WorldPos.Y >> 8),
			(p_thing->WorldPos.Z >> 8)),
	   &colour,
	   &specular);
	
	colour   &= ~POLY_colour_restrict;
	specular &= ~POLY_colour_restrict;


	//
	// Draw each body part.
	//

//	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = i; //dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];

		FIGURE_draw_prim_tween(
			start_object + object_offset,
			(p_thing->WorldPos.X >> 8)+xd,
			(p_thing->WorldPos.Y >> 8)+yd,
			(p_thing->WorldPos.Z >> 8)+zd,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			colour,
			specular);
	}
}

*/



// --------------------------------------------------------------------------
// Some convenient recentring stuff i had nowhere else convenient to shove

/*
struct	PrimPoint prim_points[MAX_PRIM_POINTS];

struct	PrimFace4 prim_faces4[MAX_PRIM_FACES3];

struct	PrimFace3 prim_faces3[MAX_PRIM_FACES4];

struct	PrimObject	prim_objects[MAX_PRIM_OBJECTS];

struct	PrimMultiObject	prim_multi_objects[MAX_PRIM_MOBJECTS];

*/


/*
GameCoord GetChunkCentre(GameKeyFrameChunk *chunk) {
	SLONG i,j,k,counter;
	GameCoord ctr = {0,0,0};
	PrimMultiObject *pmo;
	PrimObject *po;
	GameKeyFrameElement	*element; 

	counter=0;
//	for (i=0;(i<10)&&chunk->MultiObject[i];i++) {
		pmo=&prim_multi_objects[chunk->MultiObject[0]];
		for (j=pmo->StartObject;j<pmo->EndObject;j++) {
		  po=&prim_objects[j];
		  for (k=po->StartPoint;k<po->EndPoint;k++) {
			  ctr.X+=prim_points[k].X;
			  ctr.Y+=prim_points[k].Y;
			  ctr.Z+=prim_points[k].Z;
			  counter++;
//			  ctr.X*=0.5; ctr.Y*=0.5; ctr.Z*=0.5;
		  }
		}
//	}
	ctr.X/=counter; ctr.Y/=counter; ctr.Z/=counter;

	element=chunk->TheElements;

	ctr.X+=element->OffsetX;
	ctr.Y+=element->OffsetY;
	ctr.Z+=element->OffsetZ;


	return ctr;

}

SetChunkCentre(GameKeyFrameChunk *chunk, GameCoord centre) {
	SLONG i,j,k;
	PrimMultiObject *pmo;
	PrimObject *po;
	GameKeyFrameElement	*element; 
//	return;
	centre.Z=64;
//	for (i=0;(i<10)&&chunk->MultiObject[i];i++) {
		pmo=&prim_multi_objects[chunk->MultiObject[0]];
		for (j=pmo->StartObject;j<pmo->EndObject;j++) 
		{
		  po=&prim_objects[j];

		  for (k=po->StartPoint;k<po->EndPoint;k++) 
		  {
			  prim_points[k].X-=centre.X;
			  prim_points[k].Y-=centre.Y;
			  prim_points[k].Z-=centre.Z;
		  }
		}
//	}

	element=chunk->TheElements;
	for (i=0;i<chunk->MaxElements;i++) {
		element->OffsetX-=centre.X;
		element->OffsetY-=centre.Y;
		element->OffsetZ-=centre.Z;
		element++;
	}

	
}
*/



#endif //#ifndef TARGET_DC



