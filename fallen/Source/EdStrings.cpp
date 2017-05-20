//	EdStrings.cpp
//	Guy Simmons, 5th August 1998.

#include	<MFStdLib.h>


//---------------------------------------------------------------

TCHAR	*wtype_strings[]	=
{
	"(Waypoint)",
	"Create Player",
	"Create Enemies",
	"Create Vehicle",
	"Create Item",
	"Create Creature",
	"Create Camera",
	"Create Camera Target",
	"Map Exit",
	"Camera Waypoint",
	"Camera Target Waypoint",
	"Display Message",
	"Trigger Sound Effect",
	"Trigger Visual Effect",
	"Trigger Cut Scene",
	"Teleport Entrance",
	"Teleport Target",
	"End game (lose)",
	"Shout",
	"Activate Prim",
	"Create Trap",
	"Adjust Enemy",
	"Link Movable Platform",
	"Create Bomb",
	"Burn Prim",
	"End game (win)",
	"Navigation Beacon",
	"Spot FX",
	"Create Barrel",
	"Kill Waypoint",
	"Create Treasure",
	"Bonus Points",
	"Group life",
	"Group death",
	"Conversation",
	"Interesting!",
	"Increase Counter",
	"Dynamic Light FX",
	"Go Here, Do This",
	"Transfer player",
	"Autosave checkpoint",
	"Make prim searchable",
	"Lock/unlock vehicle",
	"Group reset",
	"Count-up driving game counter",
	"Reset counter",
	"Create mist",
	"Adjust enemy flags",
	"Stall car",
	"Extend timer",
	"Move thing",
	"Make person pee",
	"Cone penalties on",
	"Flash road sign",
	"Warehouse Ambience",
	"No floors",
	"Shake camera",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wtrigger_strings[]	=
{
	"Game starts",
	"Dependency",
	"Player enters radius",
	"Door opens",
	"Tripwire",
	"Pressure Pad",
	"Electric Fence",
	"Water Level",
	"Security Camera",
	"Switch",
	"Anim Prim",
	"Timer",
	"Shout (all)",
	"Boolean AND",
	"Boolean OR",
	"Item held",
	"Item seen",
	"Killed or arrested",
	"Shout (any)",
	"Countdown timer",
	"Enemy enters radius",
	"Visible countdown timer",
	"Player enters cuboid",
	"Target 50% damaged",
	"Group dead",
	"Enemy sees person",
	"Player 'uses' person",
	"Player 'uses' in radius",
	"Prim Damaged",
	"Person arrested",
	"Conversation finished",
	"Counter <a> Reaches <b>",
	"Killed not arrested",
	"Crime rate above",
	"Crime rate below",
	"Person is murderer",
	"Person in vehicle",
	"Thing stop dir/radius",
	"Player carries person",
	"Specific item held",
	"Random",
	"Player fires a gun",
	"Darci has grabbed someone",
	"Darci punches and kicks",
	"Radius and direction",
	"!"
};

//---------------------------------------------------------------

TCHAR	*on_trigger_strings[]	=
{
	"Activate once only- stay active all game",
	"Activate whenever triggered",
	"Stay active for a time then go inactive",
	"Activate once only- inactive for rest of the game ",
	"!"
};

//---------------------------------------------------------------

TCHAR	*door_actions[]	=
{
	"Unlock Door",
	"Lock Door",
	"!"
};

TCHAR	*tripwire_actions[]	=
{
	"None",
	"!"
};

TCHAR	*ppad_actions[]	=
{
	"None",
	"!"
};

TCHAR	*efence_actions[]	=
{
	"None",
	"!"
};

TCHAR	*wlevel_actions[]	=
{
	"None",
	"!"
};

TCHAR	*scamera_actions[]	=
{
	"None",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wplayer_strings[]	=
{
	"Darci",
	"Roper",
	"Cop",
	"Gang",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wenemy_strings[]	=
/*{
	"Civillian",
	"Gang Member",
	"Cult Member",
	"Cop",
	"Soldier",
	"!"
};*/
/*
{
	"Civillian",
	"Civillian (clown)",
	"Civillian (tramp)",
	"Civillian (office)",
	"Civillian (industry)",
	"Gang (unarmed)",
	"Gang (baseball)",
	"Gang (knife)",
	"Gang (pistol)",
	"Gang captain",
	"Gang boss",
	"Cop (unarmed)",
	"Cop (armed)",
	"Cop (swat)",
	"Security Guard",
	"Army (soldier)",
	"Army (captain)",
	"Army (guard)",
	"Cult (unarmed)",
	"Cult (captain)",
	"Cult (boss)",
	"Darci",
	"Roper",
	"!",
};
*/

{
	"Civillian",
	"Civillian with balloon",
	"Prostutite",
	"Prostitute fat ugly",
	"Workman",
	"Gang rasta",
	"Gang red",
	"Gang grey",
	"Gang rasta with pistol",
	"Gang red   with shotgun",
	"Gang grey  with AK47",
	"Cop",
	"Cop with pistol",
	"Cop with shotgun",
	"Cop with AK47",
	"Hostage",
	"Workman with grenade",
	"Tramp",
	"M.I.B 1",
	"M.I.B 2",
	"M.I.B 3",
	"Darci",
	"Roper",
	"!"
};


//---------------------------------------------------------------

TCHAR	*wenemy_ai_strings[]	=
{
	"Nothing",
	"Civillian",
	"Guard an area",
	"Assassin",
	"Boss/Captain",
	"Cop",
	"Violent youth (Chris)",
	"Guard a door",
	"Bodyguard",
	"Driver",
	"Bomb-disposer",
	"Biker",
	"Fight-test dummy",
	"Bully",
	"Cop driver",
	"Suicide",
	"Flee player",
	"Group Genocide",
	"M.I.B.",
	"Summoner",
	"Hypochondriac",
	"Shoot dead assasin",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wenemy_move_strings[]	=
{
	"Stand Still",
	"Patrol Waypts (in order)",
	"Patrol Waypts (randomly)",
	"Wander",
	"Follow",
	"Warm hands",
	"Follow on see",
	"Dance",
	"Hands up",
	"Tied up",
	"!"
};


//---------------------------------------------------------------

TCHAR	*wvehicle_strings[]	=
{
	"Car",
	"Van",
	"Taxi",
	"Helicopter",
	"Motorbike",
	"Police car",
	"Ambulance",
	"Meatwagon",
	"Jeep",
	"Sedan",
	"Wild Cat van",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wvehicle_behaviour_strings[]	=
{
	"Player drives",
	"Patrol waypoints",
	"Guard area",
	"Track target",
	"!"
};
//---------------------------------------------------------------

TCHAR	*wvehicle_key_strings[]	=
{
	"Unlocked",
	"Red",
	"Blue",
	"Green",
	"Black",
	"White",
	"Locked",
	"!"
};
//---------------------------------------------------------------

TCHAR *witem_strings[] =
{
	"Key",
	"Pistol",
	"Health",
	"Shotgun",
	"Knife",
	"AK47",
	"Mine",
	"Baseball bat",
	"Barrel",
	"Pistol ammo",
	"Shotgun ammo",
	"AK47 ammo",
	"Keycard",
	"File",
	"Floppy disk",
	"Crowbar",
	"Gasmask",
	"Wrench",
	"Video",
	"Gloves",
	"WeedAway",
	"Grenade",
	"Explosives",
	"Silencer",
	"Wire Cutters",
	"!"
};

TCHAR *witem_flag_strings[] =
{
	"Item follows person",
	"Item hidden in prim",
	"!"
};

TCHAR	*witem_strings_old[]	=	
{
	"Knife",
	"Baseball Bat",
	"Sledge Hammer",
	"Machete",
	"Iron Bar",
	"Fireman's Axe",
	"Hand grenade HE",
	"Incendiary Grenade",
	"Smoke Grenade",
	"Dynamite",
	"Crossbow",
	"Colt.45",
	"Uzi 9mm",
	"Heckler And Koch MP5",
	"Rifles",
	"Semi Auto Cartridge Rifle(Dragunov SVD)",
	"Sniper Conversion Semi Auto Cartridge Rifle",
	"Full Auto Kalashnikov AK-48",
	"Full Auto M16A2",
	"Sawn Off double barrel (Burst cone)",
	"Pump Action(Direct fire)",
	"M61 Light MG",
	"Chain Gun",
	"Anti Tank Weapon",
	"Anti tank rifle",
	"Grenade Launchers",
	"M78 GL with HE charges",
	"Flamethrower",
	"Home made small flamer kit",
	"Police Issue Flak Jacket",
	"Petrol Lighter",
	"Spray can",
	"Red Key Card",
	"Blue Key Card",
	"Green Key Card",
	"Crow Bar",
	"Lock picking tools",
	"Portable scanner",
	"Clip Ammunition",
	"Fuel Ammunition",
	"Medical Kit",
	"Stim pill",
	"Grappling hook",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wsfx_strings[]	=	
{
	"None",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wvfx_strings[]	=	
{
	"Flare",
	"Fire dome",
	"Shockwave",
	"Smoke trails",
	"Bonfire",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wspotfx_strings[]	=	
{
	"Water fountain",
	"Water drip",
	"Dark Smokestack Smoke",
	"Light Chimney Smoke",
	"!"
};


//---------------------------------------------------------------

TCHAR	*wcscenes_strings[]	=
{
	"None",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wmessage_strings[]	=
{
	"None",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wcreature_strings[]	=
{
	"Bat",
	"Gargoyle",
	"Balrog",
	"Bane",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wtraptype_strings[]	=

{
	"Steam Jet",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wlitetype_strings[]	=
{
	"Flashing on/off",
	"Flashing two-tone",
	"Disco",
	"Flame",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wtrapaxis_strings[]	=
{
	"Forward",
	"Up",
	"Down",
	"!"
};

//---------------------------------------------------------------

TCHAR	*showlines_strings[]	=
{
	"Always",
	"Selected Depencencies",
	"Selected All",
	"Never",
	"!"
};

//---------------------------------------------------------------

TCHAR *wcammove_strings[] =
{
	"Normal",
	"Smooth",
	"Wobbly",
	"!"
};

//---------------------------------------------------------------

TCHAR *wcamtype_strings[] =
{
	"Normal",
	"Securitycam",
	"Camcorder",
	"News Camera",
	"Targetting Scope",
	"!"
};

//---------------------------------------------------------------

TCHAR *wcamtarg_strings[] =
{
	"Normal",
	"Attached",
	"Nearest living thing",
	"!"
};

//---------------------------------------------------------------

TCHAR *wactivate_strings[] =
{
	"Door",
	"Electric Fence",
	"Security Camera",
	"Anim Prim",
	"!"
};

//---------------------------------------------------------------

TCHAR *wenemy_flag_strings[] =
{
	"Lazy",
	"Dilligent",
	"Gang",
	"Fight Back",
	"Just kill player",
	"Robotic",
	"Restricted Movement",
	"Only player kills",

	"Blue Zone",
	"Cyan Zone",
	"Yellow Zone",
	"Magenta Zone",
	"Invulnerable",
	"Guilty",
	"Fake wandering",
	"Can be carried",
	"!"
};

//---------------------------------------------------------------

TCHAR *wenemy_ability_strings[] =
{
	"Default",
	"1 (poo)", 
	"2", "3", "4", "5", "6", "7",
	"8 (average)",
	"9", "10", "11", "12", "13", "14",
	"15 (badass)",
	"!"
};

//---------------------------------------------------------------

TCHAR *wplatform_flag_strings[] =
{
	"Lock to axis",
	"Lock rotation",
	"Is a rocket",
	"!"
};

//---------------------------------------------------------------
// This one doesn't have a ! because it's not used the "normal" way...

TCHAR *colour_strings[] =
{
	"Black",
	"White",
	"Red",
	"Yellow",
	"Green",
	"Light Blue",
	"Blue",
	"Purple",
	"Mark's Pink",
	"Burnt Umber",
	"Deep Purple",
	"Soylent Green",
	"Terracotta",
	"Mint",
	"Rat's piss"
};


//---------------------------------------------------------------

TCHAR	*wweaponitem_strings[]	=	
{
	"Key",				
	"Gun",
	"Health",
	"Bomb",
	"Shotgun",
	"Knife",
	"Explosives",
	"Grenade",
	"AK47",
	"Mine",
	"Thermodroid",
	"Baseball bat",
	"Ammo pistol",
	"Ammo shotgun",
	"Ammo AK47",
	"Keycard",
	"File",
	"Floppy disk",
	"Crowbar",
	"Gasmask",
	"Wrench",
	"Video",
	"Gloves",
	"Weedaway",
	"!"
};

TCHAR	*wweaponitem_strings_old[]	=	
{
	"Knife",
	"Baseball Bat",
	"Sledge Hammer",
	"Machete",
	"Iron Bar",
	"Fireman's Axe",
	"Hand grenade HE",
	"Incendiary Grenade",
	"Smoke Grenade",
	"Dynamite",
	"Crossbow",
	"Colt.45",
	"Uzi 9mm",
	"Heckler And Koch MP5",
	"Rifles",
	"Semi Auto Cartridge Rifle(Dragunov SVD)",
	"Sniper Conversion Semi Auto Cartridge Rifle",
	"Full Auto Kalashnikov AK-48",
	"Full Auto M16A2",
	"Sawn Off double barrel (Burst cone)",
	"Pump Action(Direct fire)",
	"M61 Light MG",
	"Chain Gun",
	"Anti Tank Weapon",
	"Anti tank rifle",
	"Grenade Launchers",
	"M78 GL with HE charges",
	"Flamethrower",
	"Home made small flamer kit",
	"!"
};

TCHAR	*wotheritem_strings[]	=	
{
	"Police Issue Flak Jacket",
	"Petrol Lighter",
	"Spray can",
	"Red Key Card",
	"Blue Key Card",
	"Green Key Card",
	"Crow Bar",
	"Lock picking tools",
	"Portable scanner",
	"Clip Ammunition",
	"Fuel Ammunition",
	"Medical Kit",
	"Stim pill",
	"Grappling hook",
	"!"
};

//---------------------------------------------------------------

TCHAR	*witemcontainer_strings[] =
{
	"None (just lying around)",
	"Box 1",
	"Box 2",
	"Toolbox",
	"Bag",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wbombtype_strings[] =
{
	"Dynamite Stick",
	"Egg Timer 'n' Wires",
	"Hi Tech LED",
	"!"
};

//---------------------------------------------------------------

TCHAR	*wfire_strings[] =
{
	"Flickering flames",
	"Bonfires all over",
	"Thick flames",
	"Thick smoke",
	"(Static)",
	"!"
};


//---------------------------------------------------------------
// This one also doesn't have a ! because it's not used the "normal" way...

TCHAR	*zonetype_strings[]	=
{
	"Indoors",
	"Reverb",
	"No wander",
	"Blue Zone",
	"Cyan Zone",
	"Yellow Zone",
	"Magenta Zone",
	"No go",
};

//---------------------------------------------------------------

TCHAR *wbarrel_type_strings[] =
{
	"Barrel",
	"Traffic cone",
	"Bin",
	"Burning barrel",
	"Burning bin",
	"Oil drum",
	"LOX drum",
	"!"
};

//---------------------------------------------------------------

TCHAR *wwarefx_strings[] =
{
	"Silence",
	"Police HQ",
	"Restaurant",
	"Hi-tech/Office",
	"Club Music",
	"!"
};

