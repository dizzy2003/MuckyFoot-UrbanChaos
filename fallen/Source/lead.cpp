#include "game.h"
#include "pap.h"
#include "lead.h"
#include "animate.h"
#include "ob.h"


//
// The structures.
//

LEAD_Lead  LEAD_lead[LEAD_MAX_LEADS];
SLONG      LEAD_lead_upto;

LEAD_Point LEAD_point[LEAD_MAX_POINTS];
SLONG      LEAD_point_upto;



void LEAD_init()
{
	memset(LEAD_lead,  0, sizeof(LEAD_lead));
	memset(LEAD_point, 0, sizeof(LEAD_point));

	LEAD_lead_upto  = 1;
	LEAD_point_upto = 1;
}


void LEAD_create(
		SLONG len,
		SLONG world_x,
		SLONG world_y,
		SLONG world_z)
{
	SLONG p_num;

	LEAD_Lead *ll;

	//
	// How many points do we need?
	//

	switch(len)
	{
		case LEAD_LEN_SHORT:  p_num = 8;  break;
		case LEAD_LEN_MEDIUM: p_num = 16; break;
		case LEAD_LEN_LONG:   p_num = 32; break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Do we have enough points?
	//

	if (LEAD_point_upto + p_num > LEAD_MAX_POINTS)
	{
		return;
	}

	//
	// Ran out of objects?
	//

	if (LEAD_lead_upto >= LEAD_MAX_LEADS)
	{
		return;
	}

	ll = &LEAD_lead[LEAD_lead_upto];

	ll->p_num        = p_num;
	ll->p_index      = LEAD_point_upto;
	ll->attach_x     = world_x;
	ll->attach_y     = world_y;
	ll->attach_z     = world_z;
	ll->attach_thing = NULL;


	LEAD_lead_upto  += 1;
	LEAD_point_upto += p_num;
}

void LEAD_find_end(
		UBYTE  lead_index,
		SLONG *end_x,
		SLONG *end_y,
		SLONG *end_z)
{
	SLONG pos_x;
	SLONG pos_y;
	SLONG pos_z;

	ASSERT(WITHIN(lead_index, 1, LEAD_lead_upto - 1));

	LEAD_Lead *ll;
	Thing     *p_thing;

	ll = &LEAD_lead[lead_index];

	if (ll->attach_thing)
	{
		p_thing = TO_THING(ll->attach_thing);

		switch(p_thing->Class)
		{
			case CLASS_PERSON:
				
				calc_sub_objects_position(
					p_thing,
					p_thing->Draw.Tweened->AnimTween,
					SUB_OBJECT_HEAD,
				   &pos_x,
				   &pos_y,
				   &pos_z);

				pos_x += p_thing->WorldPos.X >> 8;
				pos_y += p_thing->WorldPos.Y >> 8;
				pos_z += p_thing->WorldPos.Z >> 8;
				
				break;

			case CLASS_ANIMAL:

				pos_x = (p_thing->WorldPos.X >> 8);
				pos_y = (p_thing->WorldPos.Y >> 8) + 0x80;
				pos_z = (p_thing->WorldPos.Z >> 8);

				break;

			default:
				ASSERT(0);
				break;
		}
	}
	else
	{
		//
		// Pick a random to initialise the string in.
		//
	}
}



void LEAD_attach()
{
	SLONG i;
	SLONG j;

	SLONG ob_x;
	SLONG ob_y;
	SLONG ob_z;
	SLONG ob_yaw;
	SLONG ob_prim;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;
	SLONG score;

	SLONG end_x;
	SLONG end_y;
	SLONG end_z;

	SLONG best_thing =  NULL;
	SLONG best_score = -INFINITY;

	#define LEAD_MAX_FIND 8

	THING_INDEX find[LEAD_MAX_FIND];
	SLONG       found;

	LEAD_Lead  *ll;
	LEAD_Point *lp;
	Thing       *p_found;

	for (i = 1; i < LEAD_lead_upto; i++)
	{
		ll = &LEAD_lead[i];

		//
		// Look for a lampost or a tree to attach to.
		//

		if (OB_find_type(
				ll->attach_x,
				ll->attach_y,
				ll->attach_z,
				0x200,
				PRIM_FLAG_LAMPOST | PRIM_FLAG_TREE,
			   &ob_x,
			   &ob_y,
			   &ob_z,
			   &ob_yaw,
			   &ob_prim))
		{
			ll->attach_x = ob_x;
			ll->attach_y = ob_y + 0x60;
			ll->attach_z = ob_z;
		}
		else
		{
			//
			// We couldn't find a tree or a lampost- so just stick to the ground.
			//

			ll->attach_y = PAP_calc_map_height_at(ll->attach_x, ll->attach_z);
		}

		//
		// Find an animal or person to attach to.
		//

		found = THING_find_sphere(
					ll->attach_x,
					ll->attach_y,
					ll->attach_z,
					0x200,
					find,
					LEAD_MAX_FIND,
					(1 << CLASS_PERSON) | (1 << CLASS_ANIMAL));

		//
		// Look for the best thing to attach to.
		//

		for (j = 0; j < found; j++)
		{
			p_found = TO_THING(find[j]);

			dx = (p_found->WorldPos.X >> 8) - ll->attach_x;
			dy = (p_found->WorldPos.Y >> 8) - ll->attach_y;
			dz = (p_found->WorldPos.Z >> 8) - ll->attach_z;

			dist  = QDIST3(abs(dx),abs(dy),abs(dz));
			score = dist;

			if (p_found->Class == CLASS_ANIMAL)
			{
				//
				// It is better to attach to animals than to people!
				//

				score <<= 2;
			}

			if (score > best_score)
			{
				best_score = score;
				best_thing = find[j];
			}
		}

		ll->attach_thing = best_thing;
	}
}



