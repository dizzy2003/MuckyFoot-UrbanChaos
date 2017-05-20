#ifndef	GUNS_H
#define	GUNS_H

#define	MAX_GUN_AIM	3000   //(3 seconds)








//
// Finds a target based on dist and angle (for pistol)
//

THING_INDEX find_target    (Thing *p_person);
THING_INDEX find_target_new(Thing *p_person);


//
// Much longer range. Greater limit on angle.
//

THING_INDEX find_snipe_target(Thing *p_person);





#endif