/*
** wadstr.h
**
** Contains string tables as used by wad_menu for multiple languages
*/

#define W_(x) Wad_Str[(x)+1]

char *Wad_Str[4096];

#define WAD_SEL_LEVEL	0
#define WAD_SEL_CAN		1
#define WAD_LOADING		2
#define WAD_AUD_MONO	3
#define WAD_AUD_STEREO	4
#define WAD_PAD_NEWTARG	5
#define WAD_PAD_LEFT	6
#define WAD_PAD_RIGHT	7
#define WAD_PAD_BACK	8
#define WAD_PAD_KICK	9
#define WAD_PAD_PUNCH	10
#define WAD_PAD_ACTION	11
#define WAD_PAD_JUMP	12
#define WAD_PAD_FPMODE	13
#define WAD_PAD_RUN		14
#define WAD_CAM_LEFT	15
#define WAD_CAM_RIGHT	16
#define WAD_PAD_INV		17
#define WAD_PAD_PAUSE	18
#define WAD_CFG_NUMBER	19
#define WAD_CFG_FREE	20
#define WAD_VIB_ON		21
#define WAD_VIB_OFF		22
#define WAD_CFG_PRESS	23
#define WAD_PAD_CONFIG	24
#define WAD_SCR_ADJUST1 25
#define WAD_SCR_ADJUST2 26
#define WAD_SEL_SELECT	27

#define WAD_MENU_FXVOL	28
#define WAD_MENU_MUSVOL	29
#define WAD_MENU_RETURN	30

#define WAD_MENU_BRIGHT 31
#define WAD_MENU_POS	32

#define WAD_LANG_ENG	33
#define WAD_LANG_FRENCH	34
#define WAD_LANG_SPAN	35
#define WAD_LANG_ITAL	36

#define WAD_MENU_AUDIO	37
#define WAD_MENU_VIDEO	38
#define WAD_MENU_PAD	39
#define WAD_MENU_LANG	40

#define WAD_MENU_NEW	41
#define WAD_MENU_LOAD	42
#define WAD_MENU_CONFIG	43

#define WAD_MENU_TIMS	44

#define WAD_PAD_SNAP	45

#define WAD_MEM_CHECK	46
#define WAD_MEM_LOAD	47
#define WAD_MEM_SAVE	48
#define WAD_MEM_SURE	49
#define WAD_MEM_YORN	50
#define WAD_MEM_NOFORM	51
#define WAD_MEM_FORMAT	52
#define WAD_MEM_EXISTS  53
#define WAD_MEM_NOSAVE	54
#define WAD_MEM_NOCARD	55
#define WAD_MEM_ERROR1	56
#define WAD_MEM_ERROR2	57

#define WAD_MEM_SAVEYN	58
#define WAD_MEM_LOADYN	59

#define WAD_MEM_DONTREM	60

#define WAD_MEM_SAVEOK	61
#define WAD_MEM_LOADOK	62

#define WAD_MEM_RECHECK 63
#define WAD_MEM_EXIT	64

#define WAD_MEM_DOFORM	65

#define WAD_MENU_SAVE	66
#define WAD_MENU_CONTINUE 67
#define WAD_MENU_ENDGAME 68

#define WAD_MEM_FULL	69

#define WAD_MENU_MAINTITLE 70
#define WAD_MENU_EOLTITLE 71

#define WAD_PAD_START	72
#define WAD_PAD_SELECT  73

#define WAD_MEM_SAVEYN2	74
#define WAD_MEM_SAVEYN3	75

#define WAD_CHEAT_ON	76
#define WAD_CHEAT_OFF	77

#define WAD_MISS_KEYS	78

#define WAD_CTRL_NONE	79
#define WAD_CTRL_UNSUP	80
#define WAD_MEM_CHANGE  81

#define WAD_MENU_SPEECH 82
#define WAD_MENU_CREDITS 83
#define WAD_CAN_CANCEL	84
#define WAD_MENU_VOLUME 85
#define WAD_CODE_CODE	86
#define WAD_CODE_CON	87
#define WAD_CODE_BEATEN 88

#define WAD_MAX_ITEMS	89
