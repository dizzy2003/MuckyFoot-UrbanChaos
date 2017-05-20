// EdText.cpp
// Guy Simmons, 19th January 1998.

#include	"Editor.hpp"

//---------------------------------------------------------------

CBYTE	*class_text[]	=	
{
	"None",
	"Player",
	"Camera",
	"Projectile",
	"Building",
	"Person",
	"Furniture",
	"Trigger",
	"Vehicle",
	"Special"
};

//---------------------------------------------------------------

CBYTE	*genus_text[][10]	=
{
	{	"None"										},
	{	"None",	"Darci",	"Roper"					},
	{	"None",	"Tracker",	"Fixed"					},
	{	"None"										},
	{	"None"										},
	{	"None",	"BLANK", "BLANK", "Cop", "Thug", "Victim", "Mafiosa"		},
	{	"None"										},
	{	"None"										},
	{	"None",	"Player Trigger",	"Thing Trigger", "Group Trigger",	"Class Trigger"	},
	{	"None",	"Van",	"Car"						},
	{	"None", "Key"								},
	{	"None"										},
	{	"None"										}
};

//---------------------------------------------------------------

CBYTE	*condition_text[]	=
{
	"NONE",
	"THING_DEAD",
	"ALL_GROUP_DEAD",
	"PERCENT_GROUP_DEAD",
	"THING_NEAR_PLAYER",
	"GROUP_NEAR_PLAYER",
	"CLASS_NEAR_PLAYER",
	"THING_NEAR_THING",
	"GROUP_NEAR_THING",
	"CLASS_NEAR_THING",
	"CLASS_COUNT",
	"GROUP_COUNT",
	"SWITCH_TRIGGERED",
	"TIME",
	"CLIST_FULFILLED"
};

//---------------------------------------------------------------

CBYTE	*command_text[]	=
{
	"NONE",
	"ATTACK_PLAYER",
	"ATTACK_THING",
	"ATTACK_GROUP",
	"ATTACK_CLASS",
	"DEFEND_PLAYER",
	"DEFEND_THING",
	"DEFEND_GROUP",
	"DEFEND_CLASS",
	"PATROL_WAYPOINT",
	"START_TIMER",
	"WAIT_FOR_TRIGGER",
	"WAIT_FOR_CLIST",
	"FOLLOW_PLAYER"
};

//---------------------------------------------------------------

CBYTE	*s_command_text[]	=
{
	"ALWAYS",
	"UNTIL_TRIGGER",
	"UNTIL_CLIST",
	"WHILE_TRIGGER",
	"WHILE_CLIST"
};

//---------------------------------------------------------------
