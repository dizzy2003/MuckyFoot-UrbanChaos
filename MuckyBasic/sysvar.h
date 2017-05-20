//
// System variables.
//

#ifndef _SYSVAR_
#define _SYSVAR_


//
// The fields that are used by the system and therefore constants.
//

#define SYSVAR_FIELD_X          0
#define SYSVAR_FIELD_Y          1
#define SYSVAR_FIELD_Z          2
#define SYSVAR_FIELD_RHW        3
#define SYSVAR_FIELD_COLOUR     4
#define SYSVAR_FIELD_SPECULAR   5
#define SYSVAR_FIELD_U          6
#define SYSVAR_FIELD_V          7
#define SYSVAR_FIELD_NUMBER     8


//
// Adds all the system variables to the SYSTEM symbol table.
// Call this function after calling ST_init()
//

void SYSVAR_init(void);



#endif
