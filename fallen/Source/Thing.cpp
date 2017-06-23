// Thing.cpp
// Guy Simmons, 14th October 1997.

#include	"Game.h"
#include	"statedef.h"
#include	"cnet.h"
#ifndef	PSX
#include	"fallen/ddlibrary/headers/net.h"
#endif
#include	"pap.h"
#include	"pcom.h"
#include	"eway.h"
#include	"interfac.h"
#include	"mfx.h"
#include	"gamemenu.h"

#include "memory.h"
//extern	ULONG	get_hardware_input(UWORD type);

UWORD	*thing_class_head;

THING_INDEX THING_array[THING_ARRAY_SIZE];
SLONG	tick_tock_unclipped=0;

#ifndef PSX
extern BOOL allow_debug_keys;
#endif

UWORD	class_priority[]=
{


	CLASS_CAMERA	,
//	CLASS_PLAYER	,
	CLASS_BARREL	,
	CLASS_BARREL	,
	CLASS_PLAT		,
	CLASS_PROJECTILE,
	CLASS_ANIMAL	,
	CLASS_SWITCH	,
	CLASS_VEHICLE	,
	CLASS_SPECIAL	,
	CLASS_ANIM_PRIM	,
	CLASS_CHOPPER	,
	CLASS_PYRO    	,
	CLASS_BIKE		,
	CLASS_BAT		,
	0
};

//---------------------------------------------------------------
#ifndef PSX
void	init_things(void)
{
	SWORD			c0,c1;


	memset((UBYTE*)THINGS,0,sizeof(Thing)*MAX_THINGS);

	for(c0=1;c0<MAX_PRIMARY_THINGS;c0++)
	{
		TO_THING(c0)->LinkParent	=	c0-1;
		TO_THING(c0)->LinkChild		=	c0+1;
		TO_THING(c0)->NextLink		=	0;
	}
	TO_THING(c0)->LinkParent	=	c0-1;
	PRIMARY_USED				=	0;
	PRIMARY_UNUSED				=	1;
	PRIMARY_COUNT				=	0;

	c1	=	(++c0);
	for(;c0<(MAX_THINGS-1);c0++)
	{
		TO_THING(c0)->LinkParent	=	c0-1;
		TO_THING(c0)->LinkChild		=	c0+1;
		TO_THING(c0)->NextLink		=	0;

	}
	TO_THING(c1)->LinkParent	=	0;
	TO_THING(c0)->LinkParent	=	c0-1;
	SECONDARY_USED				=	0;
	SECONDARY_UNUSED			=	c1;
	SECONDARY_COUNT				=	0;

	memset((UBYTE*)thing_class_head,0,CLASS_END*2);
}
#endif
//---------------------------------------------------------------

THING_INDEX	alloc_primary_thing(UWORD thing_class)
{
	THING_INDEX			new_thing;


	new_thing	=	PRIMARY_UNUSED;
	if(new_thing)
	{
		PRIMARY_UNUSED							=	TO_THING(PRIMARY_UNUSED)->LinkChild;
		TO_THING(PRIMARY_UNUSED)->LinkParent	=	0;
		TO_THING(PRIMARY_USED)->LinkParent		=	new_thing;
		TO_THING(new_thing)->LinkChild			=	PRIMARY_USED;
		TO_THING(new_thing)->LinkParent			=	0;
		PRIMARY_USED							=	new_thing;
		PRIMARY_COUNT++;
		TO_THING(new_thing)->NextLink			=	thing_class_head[thing_class];
		thing_class_head[thing_class]=new_thing;
	}
	else
	{
		ASSERT(0);	// failed to allocate primary thing
	}
	return	new_thing;
}

//---------------------------------------------------------------

void	free_primary_thing(THING_INDEX thing)
{
	UWORD	index;
	UWORD	prev;
	if(PRIMARY_USED==thing)
		PRIMARY_USED									=	TO_THING(thing)->LinkChild;
	else
		TO_THING(TO_THING(thing)->LinkParent)->LinkChild=	TO_THING(thing)->LinkChild;


	index=thing_class_head[TO_THING(thing)->Class];
	prev=0;
	while(index)
	{
		if(index==thing)
		{
			if(prev==0)
			{
				thing_class_head[TO_THING(thing)->Class]=TO_THING(index)->NextLink;
			}
			else
			{
				TO_THING(prev)->NextLink=TO_THING(thing)->NextLink;
			}
			TO_THING(index)->NextLink=0;
			break;
		}

		prev=index;
		index=TO_THING(index)->NextLink;
	}


	TO_THING(TO_THING(thing)->LinkChild)->LinkParent=	TO_THING(thing)->LinkParent;
	TO_THING(PRIMARY_UNUSED)->LinkParent			=	thing;

	TO_THING(thing)->LinkChild						=	PRIMARY_UNUSED;
	TO_THING(thing)->LinkParent						=	0;
	TO_THING(thing)->Class                          =	CLASS_NONE;
	PRIMARY_UNUSED									=	thing;
	PRIMARY_COUNT--;


#if I_WAS_MAKING_SURE_OF_IT

	{
		Thing *p_thing;
		Thing *p_parent;
		Thing *p_child;

		p_thing = TO_THING(thing);

		p_child  = (p_thing->LinkChild)  ? TO_THING(p_thing->LinkChild)  : NULL;
		p_parent = (p_thing->LinkParent) ? TO_THING(p_thing->LinkParent) : NULL;

		if (PRIMARY_USED == thing)
		{
			ASSERT(p_parent == NULL);

			PRIMARY_USED = p_thing->Child;
		}
		else
		{
		}
	}
#endif
}

//---------------------------------------------------------------

THING_INDEX	alloc_secondary_thing(UWORD thing_class)
{
	THING_INDEX			new_thing;
//	ASSERT(0);


	new_thing	=	SECONDARY_UNUSED;
	if(new_thing)
	{
		SECONDARY_UNUSED						=	TO_THING(SECONDARY_UNUSED)->LinkChild;
		TO_THING(SECONDARY_UNUSED)->LinkParent	=	0;
		TO_THING(SECONDARY_USED)->LinkParent	=	new_thing;
		TO_THING(new_thing)->LinkChild			=	SECONDARY_USED;
		TO_THING(new_thing)->LinkParent			=	0;
		SECONDARY_USED							=	new_thing;
		SECONDARY_COUNT++;
	}
	else
	{
		ASSERT(0);	// failed to allocate secondary thing
	}
	return	new_thing;
}

//---------------------------------------------------------------

void	free_secondary_thing(THING_INDEX thing)
{
	UWORD	index,prev;
//	ASSERT(0);

	index=thing_class_head[TO_THING(thing)->Class];
	prev=0;
	while(index)
	{
		if(index==thing)
		{
			if(prev==0)
			{
				thing_class_head[TO_THING(thing)->Class]=TO_THING(index)->NextLink;
			}
			else
			{
				TO_THING(prev)->NextLink=TO_THING(thing)->NextLink;
			}
			TO_THING(index)->NextLink=0;
			break;
		}

		prev=index;
		index=TO_THING(index)->NextLink;
	}

	if(SECONDARY_USED==thing)
		SECONDARY_USED									=	TO_THING(thing)->LinkChild;
	else
		TO_THING(TO_THING(thing)->LinkParent)->LinkChild=	TO_THING(thing)->LinkChild;

	TO_THING(TO_THING(thing)->LinkChild)->LinkParent=	TO_THING(thing)->LinkParent;
	TO_THING(SECONDARY_UNUSED)->LinkParent			=	thing;
	TO_THING(thing)->LinkChild						=	SECONDARY_UNUSED;
	TO_THING(thing)->LinkParent						=	0;
	TO_THING(thing)->Class                          =	CLASS_NONE;
	SECONDARY_UNUSED								=	thing;
	SECONDARY_COUNT--;
}

//---------------------------------------------------------------

void	add_thing_to_map(Thing *t_thing)
{
	SLONG mx;
	SLONG mz;

	PAP_Lo *pl;

	if(!(t_thing->Flags&FLAGS_ON_MAPWHO))				// Does thing currently exist on map?
	{
		mx = t_thing->WorldPos.X >> (8 + PAP_SHIFT_LO);
		mz = t_thing->WorldPos.Z >> (8 + PAP_SHIFT_LO);

		if(ON_PAP_LO(mx,mz))
		{
			pl = &PAP_2LO(mx, mz);

			t_thing->Parent = 0;
			t_thing->Child  = pl->MapWho;

			if (t_thing->Child > 0)
			{
				TO_THING(t_thing->Child)->Parent = THING_NUMBER(t_thing);
			}

			pl->MapWho = THING_NUMBER(t_thing);

			t_thing->Flags |= FLAGS_ON_MAPWHO;
		}
	}

	/*


	ULONG		mappos;


	if(!(t_thing->Flags&FLAGS_ON_MAPWHO))				// Does thing currently exist on map?
	{
		mappos			=	MAP_INDEX(t_thing->WorldPos.X>>(ELE_SHIFT+8),t_thing->WorldPos.Z>>(ELE_SHIFT+8));

		t_thing->Parent	=	0;
		t_thing->Child	=	MAP_WHO(mappos);
		if(t_thing->Child>0)
		{
			TO_THING(t_thing->Child)->Parent	=	THING_NUMBER(t_thing);
		}
		MAP_WHO(mappos)	=	THING_NUMBER(t_thing);
		t_thing->Flags	|=	FLAGS_ON_MAPWHO;
	}

	*/
}

//---------------------------------------------------------------

void	remove_thing_from_map(Thing *t_thing)
{
	SLONG mx;
	SLONG mz;

	PAP_Lo *pl;

	if (t_thing->Flags&FLAGS_ON_MAPWHO)				// Does thing currently exist on map?
	{
		mx = t_thing->WorldPos.X >> (8 + PAP_SHIFT_LO);
		mz = t_thing->WorldPos.Z >> (8 + PAP_SHIFT_LO);

		if(ON_PAP_LO(mx,mz))
		{
			pl = &PAP_2LO(mx, mz);


			if (t_thing->Parent)
			{
				TO_THING(t_thing->Parent)->Child = t_thing->Child;
			}
			else
			{
				pl->MapWho = t_thing->Child;
			}

			if (t_thing->Child > 0)
			{
				TO_THING(t_thing->Child)->Parent = t_thing->Parent;
			}

			t_thing->Flags &= ~FLAGS_ON_MAPWHO;
		}
	}
}

//---------------------------------------------------------------

void	move_thing_on_map(Thing *t_thing,GameCoord *new_position)
{
	SLONG cur_mx = t_thing->WorldPos.X >> (8 + PAP_SHIFT_LO);
	SLONG cur_mz = t_thing->WorldPos.Z >> (8 + PAP_SHIFT_LO);

	SLONG new_mx = new_position->X >> (8 + PAP_SHIFT_LO);
	SLONG new_mz = new_position->Z >> (8 + PAP_SHIFT_LO);
/*
	if (t_thing->Class == CLASS_PERSON)
	{
		if(t_thing->Genus.Person->PlayerID)
		{
			SLONG	dx,dz;
			dx=abs(new_position->X-t_thing->WorldPos.X)>>16;
			dz=abs(new_position->Z-t_thing->WorldPos.Z)>>16;

			ASSERT(QDIST2(dx,dz)<1);


		}
	}
*/

	//
	// Vehicles are allowed to drive off the map!
	//



	if (t_thing->Class != CLASS_VEHICLE)
	{
		SATURATE(new_position->X,0,(128<<16)-1);
		SATURATE(new_position->Z,0,(128<<16)-1);

		ASSERT(new_position->X>=0);
		ASSERT(new_position->X>>16<128);
		ASSERT(new_position->Z>=0);
		ASSERT(new_position->Z>>16<128);
	}

	if (cur_mx != new_mx ||
		cur_mz != new_mz)
	{



		//
		// The thing is moving to a new mapsquare.
		//

		remove_thing_from_map(t_thing);

		t_thing->WorldPos = *new_position;

		add_thing_to_map(t_thing);

	}
	else
	{
		t_thing->WorldPos = *new_position;
	}

	/*

	if(MAP_INDEX(t_thing->WorldPos.X>>(ELE_SHIFT+8),t_thing->WorldPos.Z>>(ELE_SHIFT+8)) != MAP_INDEX(new_position->X>>(ELE_SHIFT+8),new_position->Z>>(ELE_SHIFT+8)))
	{
		remove_thing_from_map(t_thing);
		t_thing->WorldPos	=	*new_position;
		add_thing_to_map(t_thing);
//		return	1;
	}
	else
	{
		t_thing->WorldPos	=	*new_position;
//		return	0;
	}

	*/
}
void	move_thing_on_map_dxdydz(Thing *t_thing,SLONG dx,SLONG dy,SLONG dz)
{
	GameCoord new_position;
	new_position.X=t_thing->WorldPos.X+dx;
	new_position.Y=t_thing->WorldPos.Y+dy;
	new_position.Z=t_thing->WorldPos.Z+dz;

	move_thing_on_map(t_thing,&new_position);


}

//---------------------------------------------------------------
#ifndef PSX
void	log_primary_used_list(void)
{
	THING_INDEX		thing;


	DebugText("\nPRIMARY USED\n");
	thing	=	PRIMARY_USED;
	while(thing)
	{
		DebugText("Thing - %ld\nThing->Parent - %ld\nThing->Child - %ld\n",thing,TO_THING(thing)->LinkParent,TO_THING(thing)->LinkChild);
		thing	=	TO_THING(thing)->LinkChild;
	}
}

//---------------------------------------------------------------


void	log_primary_unused_list(void)
{
	THING_INDEX		thing;


	DebugText("\nPRIMARY UNUSED\n");
	thing	=	PRIMARY_UNUSED;
	while(thing)
	{
		DebugText("Thing - %ld\nThing->Parent - %ld\nThing->Child - %ld\n",thing,TO_THING(thing)->LinkParent,TO_THING(thing)->LinkChild);
		thing	=	TO_THING(thing)->LinkChild;
	}
}

//---------------------------------------------------------------

void	log_secondary_used_list(void)
{
	THING_INDEX		thing;


	DebugText("\nSECONDARY USED\n");
	thing	=	SECONDARY_USED;
	while(thing)
	{
		DebugText("Thing - %ld\nThing->Parent - %ld\nThing->Child - %ld\n",thing,TO_THING(thing)->LinkParent,TO_THING(thing)->LinkChild);
		thing	=	TO_THING(thing)->LinkChild;
	}
}

//---------------------------------------------------------------

void	log_secondary_unused_list(void)
{
	THING_INDEX		thing;


	DebugText("\nSECONDARY UNUSED\n");
	thing	=	SECONDARY_UNUSED;
	while(thing)
	{
		DebugText("Thing - %ld\nThing->Parent - %ld\nThing->Child - %ld\n",thing,TO_THING(thing)->LinkParent,TO_THING(thing)->LinkChild);
		thing	=	TO_THING(thing)->LinkChild;
	}
}

//---------------------------------------------------------------
void	Time(struct MFTime *the_time);

void	wait_ticks(SLONG wait)
{
	struct MFTime 	the_time;

	SLONG	tick_reqd;
	Time(&the_time);
	tick_reqd=the_time.Ticks+wait;
	while(the_time.Ticks<tick_reqd)
	{
		Time(&the_time);
	}
}
#endif
//
// If you want to try something out do it here rather than game.h
//
#undef NORMAL_TICK_TOCK
#define	NORMAL_TICK_TOCK	(1000/20)

struct	NET_packet
{
	ULONG	Input;
	SLONG	Check1;
};

extern	UWORD		controls;

#ifndef	PSX

MFFileHandle	playback_file;
MFFileHandle	verifier_file;

UBYTE	input_type[]={1,2,0,0,0,0,0,0};


void	do_packets(void)
{
	ULONG	input,count=0;
	NET_Message		answer;
	NET_packet		packets[10];
	SLONG	c0;


//
// Everyone gets user input from there machines
//
	// 'controls' contains the user input word.
	{


		// Reset the user input word.
	//	controls	=	0;

		if(!CNET_network_game)
		{
			if(NO_PLAYERS>1)
			{

				for(c0=0;c0<NO_PLAYERS;c0++)
				{
					input		=	get_hardware_input(input_type[c0]); //controls;
					packets[c0].Input	=	input;
					PACKET_DATA(c0)	=	packets[c0].Input;

				}

			}
			else
			{

				//	This section is a bodge to make sure the record files are compatible between network & single player games.
#ifdef TARGET_DC
				input		=	get_hardware_input(INPUT_TYPE_JOY); //controls;
#else
				input		=	get_hardware_input(INPUT_TYPE_ALL); //controls;
#endif
				packets[PLAYER_ID].Input	=	input;

				if(GAME_STATE&GS_PLAYBACK)		//	Playback a single player game.
				{
					SLONG	check;
					//	Load in the timing stuff.
					FileRead(playback_file,&TICK_TOCK,sizeof(TICK_TOCK));
					FileRead(playback_file,&TICK_RATIO,sizeof(TICK_RATIO));
					FileRead(playback_file,&TICK_INV_RATIO,sizeof(TICK_INV_RATIO));
					FileRead(playback_file,&TICK_TOCK,sizeof(TICK_TOCK));

					//	Now load in the packets.
					for(count=0;count<(unsigned)NO_PLAYERS;count++)
					{
						if(FileRead(playback_file,&packets[count],sizeof(NET_packet)) != sizeof(NET_packet))
							GAME_STATE	&=	~GS_PLAY_GAME;
					}
				}
				else if(GAME_STATE&GS_RECORD)	//	Record a single player game.
				{
					//	Save out the timing stuff.
					FileWrite(playback_file,&TICK_TOCK,sizeof(TICK_TOCK));
					FileWrite(playback_file,&TICK_RATIO,sizeof(TICK_RATIO));
					FileWrite(playback_file,&TICK_INV_RATIO,sizeof(TICK_INV_RATIO));
					FileWrite(playback_file,&TICK_TOCK,sizeof(TICK_TOCK));

					//	Now save out the packets.
					for(count=0;count<NO_PLAYERS;count++)
					{
						FileWrite(playback_file,&packets[count],sizeof(NET_packet));
					}
				}

				PACKET_DATA(PLAYER_ID)	=	packets[PLAYER_ID].Input;
			}

			return;
		}

		input		=	get_hardware_input(INPUT_TYPE_ALL); //controls;


		LogText("NETWORK do packet  i am %d no players %d \n",PLAYER_ID,NO_PLAYERS);


		packets[PLAYER_ID].Input=input;
		packets[PLAYER_ID].Check1=0;
		for(c0=0;c0<NO_PLAYERS;c0++)
			packets[PLAYER_ID].Check1+=NET_PERSON(c0)->WorldPos.X+NET_PERSON(c0)->WorldPos.Z;


	//
	// everyone except the host sends its user input
	//
 		if(PLAYER_ID!=0)
			NET_message_send(0,&packets[PLAYER_ID],sizeof(NET_packet));

	//
	// host, loops until received all packets
	//
		if(PLAYER_ID==0 && CNET_network_game)
		{
			while(SHELL_ACTIVE && count<(unsigned)(NO_PLAYERS-1) && LastKey!=KB_ESC)
			{
				if(NET_message_waiting())
				{
					NET_message_get(&answer);
					if(answer.player_id==NET_PLAYER_SYSTEM)
					{
						MSG_add(" sysmessage %d \n",answer.system.sysmess);
					}
					else
					{
						count++;
	//					PACKET_DATA(answer.player_id)=*(UWORD*)(answer.player.data);

						packets[answer.player_id]=*(NET_packet*)(answer.player.data);
					}

				}
			}

	//
	// host sends out all the messages
	//

	//	LogText(" host has all messages, send them off

	//		NET_message_send(NET_PLAYER_ALL,&PACKET_DATA(0),NO_PLAYERS*2);
			NET_message_send(NET_PLAYER_ALL,&packets[0],NO_PLAYERS*sizeof(NET_packet));
		}

	//
	// eveyone except host waits for a packet
	//
		if(PLAYER_ID!=0 && CNET_network_game)
		{
			SLONG	got_message=0;
			while(SHELL_ACTIVE&&!got_message&&LastKey!=KB_ESC)
			{
				if(NET_message_waiting())
				{
					NET_message_get(&answer);
					if(answer.player_id==NET_PLAYER_SYSTEM)
					{
						MSG_add(" sysmessage %d \n",answer.system.sysmess);
					}
					else
					{
						got_message=1;
	//					memcpy(&PACKET_DATA(0),answer.player.data,answer.player.num_bytes);
						memcpy(&packets[0],answer.player.data,answer.player.num_bytes);
					}
				}
			}
		}
		if(0)
		for(c0=0;c0<NO_PLAYERS;c0++)
		{
			LogText(" NET player %d input %x, x %d z %d vel %d st %d ss %d \n",
				c0,
				packets[c0].Input,
				NET_PERSON(c0)->WorldPos.X,
				NET_PERSON(c0)->WorldPos.Z,
				NET_PERSON(c0)->Velocity,
				NET_PERSON(c0)->State,
				NET_PERSON(c0)->SubState);
		}
/*
		if(GAME_STATE&GS_RECORD)		//	Playback a network game.
		{
			//	Load in the timing stuff.
			FileRead(playback_file,&TICK_RATIO,sizeof(TICK_RATIO));
			FileRead(playback_file,&TICK_INV_RATIO,sizeof(TICK_INV_RATIO));
			FileRead(playback_file,&TICK_TOCK,sizeof(TICK_TOCK));

			//	Now load in the packets.
			for(count=0;count<NO_PLAYERS;count++)
			{
				if(FileRead(playback_file,&packets[count],sizeof(NET_packet)) != sizeof(NET_packet))
					GAME_STATE	&=	~GS_PLAY_GAME;
			}
		}
		else if(GAME_STATE&GS_RECORD)	//	Record a network game.
		{
			//	Save out the timing stuff.
			FileWrite(playback_file,&TICK_RATIO,sizeof(TICK_RATIO));
			FileWrite(playback_file,&TICK_INV_RATIO,sizeof(TICK_INV_RATIO));
			FileWrite(playback_file,&TICK_TOCK,sizeof(TICK_TOCK));

			//	Now save out the packets.
			for(count=0;count<NO_PLAYERS;count++)
			{
				FileWrite(playback_file,&packets[count],sizeof(NET_packet));
			}
		}
*/
		for(count=0;count<NO_PLAYERS;count++)
		{
			if(packets[count].Check1!=packets[PLAYER_ID].Check1)
			{
				MSG_add(" out of sync player %d ",PLAYER_ID);
				ASSERT(0);
			}
			PACKET_DATA(count)=packets[count].Input;
		}
	}

}

#else


struct	recorder
{
	ULONG	input;
	UWORD	tick_tock;
	UWORD	tick_ratio;
	UWORD	tick_tock_un;
	UWORD	pad;
	ULONG	s1;

};

#ifndef FS_ISO9660

#define	MAX_RECORD	5000

struct recorder	record_input[MAX_RECORD];
UWORD	record_index=0;

#else

#endif

#ifndef FS_ISO9660

//
//run off dev kit
//

void	init_record(SLONG	level)
{
	record_input[0].input=level;

	record_index=1;
}

void	end_record(void)
{
	int handle;
	CBYTE	fname[50];

	record_input[record_index].input=0xffffffff;
	record_input[record_index].tick_tock=TICK_TOCK;
	record_input[record_index].tick_ratio=TICK_RATIO;
	record_index++;


	sprintf(fname,"replay%02d.gam",record_input[0].input);
	handle=PCcreat(fname,0);
	PCwrite(handle,(UBYTE*)record_input,record_index*sizeof(struct recorder));
	PCclose(handle);
}

#endif
void	init_playback(void)
{
#ifndef FS_ISO9660
	record_index=1;
	GAME_STATE=GS_PLAY_GAME;
	GAME_STATE|=GS_PLAYBACK;
#endif
}

void	do_packets(void)
{

	PACKET_DATA(0)=get_hardware_input(INPUT_TYPE_ALL); //controls
#ifndef FS_ISO9660
	if(GAME_STATE&GS_PLAYBACK)		//	Playback a single player game.
	{
		PACKET_DATA(0)=record_input[record_index].input;
		TICK_RATIO=record_input[record_index].tick_ratio;
		TICK_TOCK=record_input[record_index].tick_tock;
		tick_tock_unclipped=record_input[record_index].tick_tock_un;
		ASSERT(record_input[record_index].s1==RAND_SEED);

		if(TICK_RATIO)
			TICK_INV_RATIO = 0x10000 / TICK_RATIO;

		record_index++;

	}
	else //if(GAME_STATE&GS_RECORD)	//	Record a single player game.
	{
		record_input[record_index].input=PACKET_DATA(0);
		record_input[record_index].tick_tock=TICK_TOCK;
		record_input[record_index].tick_ratio=TICK_RATIO;
		record_input[record_index].tick_tock_un=tick_tock_unclipped;
		record_input[record_index].s1=RAND_SEED;

		record_index++;

		if(record_index>MAX_RECORD)
			record_index--;
	}
#endif
	controls=0;

}

#endif

static	UWORD slow_mo=0;

void	set_slow_motion(UWORD motion)
{
	slow_mo=motion;

}


#ifndef PSX

UWORD	class_check[]=
{
	CLASS_PLAYER	,
	CLASS_PERSON	,
	CLASS_CAMERA	,
	CLASS_BARREL	,
	CLASS_BARREL	,
	CLASS_PLAT		,
	CLASS_PROJECTILE,
	CLASS_ANIMAL	,
	CLASS_SWITCH	,
	CLASS_VEHICLE	,
	CLASS_SPECIAL	,
	CLASS_ANIM_PRIM	,
	CLASS_CHOPPER	,
	CLASS_PYRO    	,
	CLASS_BIKE		,
	CLASS_BAT		,
	0
};

void copy_important_thing_bits(const Thing* src, Thing* dst)
{
	memcpy(dst, src, sizeof(Thing));
	dst->Draw.Tweened = NULL;
	dst->Genus.Vehicle = NULL;
	dst->Flags &= ~FLAGS_HAS_ATTACHED_SOUND;
	dst->StateFn = NULL;
}

void store_thing(Thing* p_thing)
{
	Thing	temp;

	copy_important_thing_bits(p_thing, &temp);

	FileWrite(verifier_file, &temp, sizeof(temp));
}

void check_thing(Thing* p_thing)
{
	Thing	temp;

	copy_important_thing_bits(p_thing, &temp);

	Thing	file;

	FileRead(verifier_file, &file, sizeof(file));

	Thing	file2;

	copy_important_thing_bits(&file, &file2);

	if (memcmp(&file2, &temp, sizeof(file2)))
	{
		TRACE("Game turn = %d\n", GAME_TURN);
		TRACE("Thing# = %d\n", THING_NUMBER(p_thing));
		ASSERT(0);
	}
}

void for_things(void (*fn)(Thing* p_thing))
{
	int	ix = 0;

	while (class_check[ix])
	{
		UWORD	list;

		list = thing_class_head[class_check[ix]];

		while (list)
		{
			Thing*	p_thing = TO_THING(list);

			ASSERT(p_thing->Class == class_check[ix]);

			fn(p_thing);

			list = p_thing->NextLink;
		}

		ix++;
	}
}

void	store_thing_data()
{
	for_things(store_thing);
}

void	check_thing_data()
{
	for_things(check_thing);
}

#endif

SLONG REAL_TICK_RATIO = 256;

void	process_things_tick(SLONG frame_rate_independant)
{
	static	SLONG	prev_tick = 0;
	SLONG	cur_tick;

	SLONG	tick_diff;
	static	first_pass=1;

 	cur_tick=GetTickCount();
	tick_diff=cur_tick-prev_tick;
	prev_tick=cur_tick;

	if(first_pass)
	{
		tick_diff=NORMAL_TICK_TOCK;
		first_pass=0;
	}

#ifndef PSX
	if(CNET_network_game)
	{
		tick_diff=1000/20;
	}

	if(frame_rate_independant==0)
		tick_diff=1000/25;  //assume 25 fps

	if(allow_debug_keys)
	if(Keys[KB_COLON])
	{
		if(slow_mo)
			slow_mo=0;
		else
			slow_mo=32000;
		Keys[KB_COLON]=0;
	}

	// 20/50

extern	UBYTE	record_video;

	if(record_video)
		tick_diff=40;
#endif

	TICK_TOCK=tick_diff;
	TICK_RATIO=(TICK_TOCK<<TICK_SHIFT)/(NORMAL_TICK_TOCK);

	tick_tock_unclipped=TICK_TOCK;

	// smooth my ticks up
	TICK_RATIO = SmoothTicks(TICK_RATIO);

	#define MIN_TICK_RATIO ((1 << TICK_SHIFT) / 2)
	#define MAX_TICK_RATIO ((1 << TICK_SHIFT) * 3 >> 1)

	SATURATE(TICK_TOCK, NORMAL_TICK_TOCK>>1, NORMAL_TICK_TOCK*2);
	SATURATE(TICK_RATIO, MIN_TICK_RATIO, MAX_TICK_RATIO);

	if(slow_mo)
	{
		slow_mo--;
		TICK_RATIO = 32;
		TICK_TOCK  = (TICK_RATIO*NORMAL_TICK_TOCK)>>TICK_SHIFT;
	}

	//
	// Slow down the game...
	//
	REAL_TICK_RATIO = TICK_RATIO; // some things need to keep going, need to keep F.R.I.
#ifndef PSX
	if (!GAMEMENU_is_paused())
		if (GAMEMENU_slowdown_mul() != 0x100)
		{
			TICK_RATIO = TICK_RATIO * GAMEMENU_slowdown_mul() >> 8;
			TICK_TOCK  = TICK_RATIO * NORMAL_TICK_TOCK >> TICK_SHIFT;

			if (TICK_RATIO == 0)
			{
				TICK_RATIO = 1;
			}
		}
#endif
	TICK_INV_RATIO = 0x10000 / TICK_RATIO;

}
extern	SWORD	noise_count;
extern  void	process_noises(void);

void	process_things(SLONG frame_rate_independant)
{
	Thing	*p_thing;
	Thing			*t_thing;
	THING_INDEX		current_thing;


	SLONG	count=0;
	UWORD	index=0;



//#ifdef	PSX
//extern	SWORD	sync_count;
//	if(sync_count<1)
//		sync_count=1;
//#ifdef VERSION_NTSC
//	tick_diff=1000/(60/sync_count);
//#else
//	tick_diff=1000/(50/sync_count);
//#endif
//#else

	process_things_tick(frame_rate_independant);

	//	Do the packet processing.
	do_packets();

	//
	// Process things.
	//


	//
	// The people, because they are special in that they don't use statefn
	//


	//
	// The priority list
	//

	index=0;
	while(class_priority[index])
	{
		UWORD	list;

		list=thing_class_head[class_priority[index]];
		while(list)
		{
			p_thing=TO_THING(list);
			list=p_thing->NextLink;
			ASSERT(p_thing->Class==class_priority[index]);

			if(p_thing->StateFn)
			{
#ifdef	PSX
				if((p_thing->Flags & FLAGS_IN_VIEW) ||  ((THING_NUMBER(p_thing)&0x3)==(GAME_TURN&0x3)))
#endif
				{
					p_thing->StateFn(p_thing);
					if(noise_count)
					{
						process_noises();

					}

					if (index == 1)
					{
						//
						// The first time through processing the barrels.
						//
					}
					else
					{
						p_thing->Flags	&=	~FLAGS_IN_VIEW;
					}
				}

			}

//			list = next;//p_thing->NextLink;
		}
		index++;
	}

	//
	//	Players don't care about flag_in_view
	//

	index=thing_class_head[CLASS_PLAYER];
	while(index)
	{
		p_thing=TO_THING(index);
		index =p_thing->NextLink;

		ASSERT(p_thing->Class==CLASS_PLAYER);

		if(p_thing->StateFn)
		{
			p_thing->StateFn(p_thing);
		}

		p_thing->Flags	&=	~FLAGS_IN_VIEW;

//		index =p_thing->NextLink;
	}


extern	void WMOVE_process();
	WMOVE_process();

	index=thing_class_head[CLASS_PERSON];
	while(index)
	{
		p_thing=TO_THING(index);
		index=p_thing->NextLink;
		ASSERT(p_thing->Class==CLASS_PERSON);


//		p_thing->Genus.Person->ComboHistory++; //now used left right and center for a random number // if someone only does something every 16 turns use this
		++PTIME(p_thing);

#ifdef	PSX
		//      if in view                  ||     or every 4th turn                           ||                              they are a fake person sent to attack you
		if((p_thing->Flags & FLAGS_IN_VIEW) || (((index&0x3)==(GAME_TURN&0x3)))||(p_thing->Genus.Person->PersonType==PERSON_THUG_RASTA && (p_thing->Genus.Person->Flags2 & FLAG2_PERSON_FAKE_WANDER) ) )
			/*&& !(p_thing->Genus.Person->pcom_ai       == PCOM_AI_CIV &&	p_thing->Genus.Person->pcom_ai_state == PCOM_AI_STATE_NORMAL && p_thing->Genus.Person->pcom_move == PCOM_MOVE_WANDER) )*/
#endif
		{

#ifdef	PSX_STERN_REVENGE_BUG_AND_CRAP_DRIVERS
			poo

			SLONG	dx,dz;



			dx=abs(POLY_cam_x-(p_thing->WorldPos.X>>8));
			dz=abs(POLY_cam_z-(p_thing->WorldPos.Z>>8));

			if(QDIST2(dx,dz)<(40<<8))
#endif
			{
				if(p_thing->Genus.Person->InCar)
				{
#if 0
					TRACE("index %d thing %x gt %d",index,p_thing,GAME_TURN);
#endif
				}
				general_process_person(p_thing);
				PCOM_process_person(p_thing);


				if(noise_count)
				{
					process_noises();

				}

			}
		}
		p_thing->Flags	&=	~FLAGS_IN_VIEW;

//		index =next;//p_thing->NextLink;
	}
extern	void	do_arrests(void);
	do_arrests();


}

//---------------------------------------------------------------

inline BOOL	is_class_primary(SBYTE classification)
{
//	return(TRUE);

	switch(classification)
	{
		// Secondaries.
		case	CLASS_NONE:
		case	CLASS_CAMERA:
		case	CLASS_SWITCH:
		case	CLASS_TRACK:
			return	FALSE;
		// Primaries.
		case	CLASS_PLAYER:
		case	CLASS_PROJECTILE:
		case	CLASS_BUILDING:
		case	CLASS_PERSON:
		case	CLASS_FURNITURE:
		case	CLASS_VEHICLE:
		case	CLASS_SPECIAL:
		case	CLASS_PYRO:
		case	CLASS_PLAT:
		case	CLASS_BARREL:
		case	CLASS_BIKE:
		case	CLASS_BAT:
			return	TRUE;
		default:
			ASSERT(0);
			return	FALSE;
	}
}

//---------------------------------------------------------------

Thing	*alloc_thing(SBYTE classification)
{
	Thing			*t_thing	=	NULL;
	THING_INDEX		new_thing;


	if(is_class_primary(classification))
		new_thing	=	alloc_primary_thing(classification);
	else
		new_thing	=	alloc_secondary_thing(classification);

	if(new_thing)
	{
		t_thing				=	TO_THING(new_thing);
		t_thing->Class		=	classification;
		t_thing->State		=	STATE_INIT;
		t_thing->Flags		=	0;
		t_thing->StateFn	=	0;
	}
	return	t_thing;
}

//---------------------------------------------------------------

void	free_thing(Thing *t_thing)
{
	if (t_thing->Flags&FLAGS_HAS_ATTACHED_SOUND) MFX_stop_attached(t_thing);
	if(is_class_primary(t_thing->Class))
		free_primary_thing(THING_NUMBER(t_thing));
	else
		free_secondary_thing(THING_NUMBER(t_thing));
}

//---------------------------------------------------------------


/*
// 'closest' is set outside of the function & defines the bounds of the check.
Thing	*nearest_class(Thing *the_thing,ULONG class_mask,ULONG *closest)
{
	ULONG			distance,
					radius;
	SLONG			cx,cz,
					min_x,max_x,
					min_z,max_z;
	Thing			*possible_nearest,
					*nearest			=	NULL;
	THING_INDEX		current_thing;


	// Find the bounding rectangle for the check.
	radius	=	Root(*closest);
	min_x	=	((the_thing->WorldPos.X>>8)-radius)>>ELE_SHIFT;
	max_x	=	((the_thing->WorldPos.X>>8)+radius)>>ELE_SHIFT;
	min_z	=	((the_thing->WorldPos.Z>>8)-radius)>>ELE_SHIFT;
	max_z	=	((the_thing->WorldPos.Z>>8)+radius)>>ELE_SHIFT;

	// Keep the rectangle withing the bounds of the map.
	in_range(min_x,0,MAP_WIDTH-1);
	in_range(max_x,0,MAP_WIDTH-1);
	in_range(min_z,0,MAP_HEIGHT-1);
	in_range(max_z,0,MAP_HEIGHT-1);

	// Search through the map.
	for(cx=min_x;cx<=max_x;cx++)
	{
		for(cz=min_z;cz<=max_z;cz++)
		{
			current_thing	=	MAP_WHO(MAP_INDEX(cx,cz));
			while(current_thing)
			{
				possible_nearest	=	TO_THING(current_thing);
				current_thing		=	possible_nearest->Child;

				// Make sure thing is a valid class.
				if(class_mask&(1<<possible_nearest->Class))
				{
					distance	=	SDIST3	(
												(possible_nearest->WorldPos.X-the_thing->WorldPos.X)>>8,
												(possible_nearest->WorldPos.Y-the_thing->WorldPos.Y)>>8,
												(possible_nearest->WorldPos.Z-the_thing->WorldPos.Z)>>8
											);
					if(distance<*closest)
					{
						// If distance==0 then chances are we're checking against ourself.
						if(distance==0 && the_thing==possible_nearest)
							continue;

						*closest	=	distance;
						nearest		=	possible_nearest;
					}
				}
			}
		}
	}
	return	nearest;
}

*/

//---------------------------------------------------------------

void THING_kill(Thing *p_thing)
{
	//
	// Remove from the map.
	//

	remove_thing_from_map(p_thing);

	switch(p_thing->Class)
	{
		case CLASS_NONE:
			break;
		case CLASS_PLAYER:
			free_person(p_thing);
			break;
		case CLASS_PROJECTILE:
			free_projectile(p_thing);
			break;
		case CLASS_BUILDING:
			ASSERT(0);
			break;
		case CLASS_PERSON:
			free_person(p_thing);
			break;
		case CLASS_FURNITURE:
		case CLASS_VEHICLE:
//			free_furniture(p_thing);
			break;
		case CLASS_ANIMAL:
#if !defined(PSX) && !defined(TARGET_DC)
			free_animal(p_thing);
#endif
			break;
		default:
			ASSERT(0);
			break;
	}
}

SLONG THING_dist_between(Thing *p_thing_a, Thing *p_thing_b)
{
	SLONG dx = abs(p_thing_b->WorldPos.X - p_thing_a->WorldPos.X >> 8);
	SLONG dy = abs(p_thing_b->WorldPos.Y - p_thing_a->WorldPos.Y >> 8);
	SLONG dz = abs(p_thing_b->WorldPos.Z - p_thing_a->WorldPos.Z >> 8);

	SLONG dist = QDIST3(dx,dy,dz);

	return dist;
}


//---------------------------------------------------------------

UBYTE	hit_player=0;
//
// if classes & 1<<31  then its really find sphere
//
SLONG THING_find_sphere(SLONG pos_x, SLONG pos_y, SLONG pos_z, SLONG radius, THING_INDEX *array, SLONG array_size, ULONG classes)
{
	UBYTE mx;
	UBYTE mz;

	SWORD x1;
	SWORD z1;
	SWORD x2;
	SWORD z2;

	SWORD dx;
	SWORD dy;
	SWORD dz;

	SLONG dist;

	SWORD array_upto;

	THING_INDEX t_index;
	Thing      *p_thing;

	//
	// Find the bounding rectangle in which to search.
	//

	x1 = pos_x - radius >> PAP_SHIFT_LO;
	z1 = pos_z - radius >> PAP_SHIFT_LO;

	x2 = pos_x + radius >> PAP_SHIFT_LO;
	z2 = pos_z + radius >> PAP_SHIFT_LO;

	//
	// Make sure the bounding rectangle is on the map.
	//

	SATURATE(x1, 0, PAP_SIZE_LO - 1);
	SATURATE(x2, 0, PAP_SIZE_LO - 1);
	SATURATE(z1, 0, PAP_SIZE_LO - 1);
	SATURATE(z2, 0, PAP_SIZE_LO - 1);

	//
	// Clear the array...
	//

	array_upto = 0;

	//
	// Look for thinge in the bounding rectangle.
	//

	for (mx = x1; mx <= x2; mx++)
	{
		for (mz = z1; mz <= z2; mz++)
		{
			t_index = PAP_2LO(mx,mz).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				//
				// Do we care about this thing?
				//

				if (classes & (1 << p_thing->Class))
				{
					//
					// How far is this thing from the epi-centre?
					//

					dx = abs((p_thing->WorldPos.X >> 8) - pos_x);
					if(classes&1<<31)
						dy=0;
					else
						dy = abs((p_thing->WorldPos.Y >> 8) - pos_y);
					dz = abs((p_thing->WorldPos.Z >> 8) - pos_z);

					dist = QDIST3(dx, dy, dz);

					if (dist <= radius)
					{
						//
						// Found a thing.
						//
						if((!hit_player)&& p_thing->Class==CLASS_PERSON && p_thing->Genus.Person->PlayerID && EWAY_stop_player_moving())
						{
							//
							// can't find player in cut scene
							//
						}
						else
						{
							if (array_upto < array_size)
							{
								array[array_upto++] = t_index;
							}
							else
							{
								//TRACE("THING_find found too many things for array size.");

								return array_upto;
							}
						}
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	return array_upto;
}



SLONG THING_find_box(SLONG x1, SLONG z1, SLONG x2, SLONG z2, THING_INDEX *array, SLONG array_size, ULONG classes)
{
	SLONG mx;
	SLONG mz;

	SLONG array_upto;

	THING_INDEX t_index;
	Thing      *p_thing;

	//
	// Make sure the bounding rectangle is on the map.
	//

	x1 >>= PAP_SHIFT_LO;
	z1 >>= PAP_SHIFT_LO;
	x2 >>= PAP_SHIFT_LO;
	z2 >>= PAP_SHIFT_LO;

	SATURATE(x1, 0, PAP_SIZE_LO - 1);
	SATURATE(x2, 0, PAP_SIZE_LO - 1);
	SATURATE(z1, 0, PAP_SIZE_LO - 1);
	SATURATE(z2, 0, PAP_SIZE_LO - 1);

	//
	// Clear the array...
	//

	array_upto = 0;

	//
	// Look for thinge in the bounding rectangle.
	//

	for (mx = x1; mx <= x2; mx++)
	{
		for (mz = z1; mz <= z2; mz++)
		{
			t_index = PAP_2LO(mx,mz).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				//
				// Do we care about this thing?
				//

				if (classes & (1 << p_thing->Class))
				{
					//
					// Found a thing.
					//

					if (array_upto < array_size)
					{
						array[array_upto++] = t_index;
					}
					else
					{
						TRACE("THING_find found too many things for array size.");

						return array_upto;
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	return array_upto;
}


SLONG THING_find_nearest_xyz_p(
		SLONG centre_x,
		SLONG centre_y,
		SLONG centre_z,
		SLONG radius,
		ULONG classes,
		Thing *p_person)
{
	SLONG mx;
	SLONG mz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG dx;
	SLONG dy;
	SLONG dz;

	SLONG dist;

	SLONG best_dist  = radius;
	SLONG best_thing = NULL;

	THING_INDEX t_index;
	Thing      *p_thing;

	//
	// Find the bounding rectangle in which to search.
	//

	x1 = centre_x - radius >> PAP_SHIFT_LO;
	z1 = centre_z - radius >> PAP_SHIFT_LO;

	x2 = centre_x + radius >> PAP_SHIFT_LO;
	z2 = centre_z + radius >> PAP_SHIFT_LO;

	//
	// Make sure the bounding rectangle is on the map.
	//

	SATURATE(x1, 0, PAP_SIZE_LO - 1);
	SATURATE(x2, 0, PAP_SIZE_LO - 1);
	SATURATE(z1, 0, PAP_SIZE_LO - 1);
	SATURATE(z2, 0, PAP_SIZE_LO - 1);

	//
	// Look for thinge in the bounding rectangle.
	//

	for (mx = x1; mx <= x2; mx++)
	{
		for (mz = z1; mz <= z2; mz++)
		{
			t_index = PAP_2LO(mx,mz).MapWho;

			while(t_index)
			{
				p_thing = TO_THING(t_index);

				//
				// Do we care about this thing?
				//

				if ((classes & (1 << p_thing->Class)) && p_thing!=p_person)
				{
					//
					// How far is this thing from the epi-centre?
					//

					dx = abs((p_thing->WorldPos.X >> 8) - centre_x);
					dy = abs((p_thing->WorldPos.Y >> 8) - centre_y);
					dz = abs((p_thing->WorldPos.Z >> 8) - centre_z);

					dist = QDIST3(dx, dy, dz);

					if (dist < best_dist)
					{
						//
						// Found a thing.
						//

						best_dist  = dist;
						best_thing = t_index;
					}
				}

				t_index = p_thing->Child;
			}
		}
	}

	return best_thing;
}

SLONG THING_find_nearest(
		SLONG centre_x,
		SLONG centre_y,
		SLONG centre_z,
		SLONG radius,
		ULONG classes)
{
	return(THING_find_nearest_xyz_p(centre_x,centre_y,centre_z,radius,classes,0));

}

SLONG THING_find_nearest_person(Thing *p_person,SLONG radius,ULONG classes)
{
	return(THING_find_nearest_xyz_p(p_person->WorldPos.X>>8,p_person->WorldPos.Y>>8,p_person->WorldPos.Z>>8,radius,classes,p_person));

}
