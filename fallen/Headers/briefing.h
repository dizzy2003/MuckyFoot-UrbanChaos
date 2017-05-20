//
// briefing.h
// mission selection, briefings
// 14 dec 98
//

#include "MFStdLib.h"

extern CBYTE BRIEFING_mission_filename[_MAX_PATH];

SBYTE BRIEFING_select(void);
SBYTE BRIEFING_next_mission(); // 0 = run out of missions

//#define OBEY_SCRIPT