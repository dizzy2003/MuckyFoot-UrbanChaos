//
// System variables.
//

#include "always.h"
#include "mem.h"
#include "ml.h"
#include "st.h"
#include "sysvar.h"
#include "vm.h"

//
// The names of the system fields.
//

CBYTE *SYSVAR_field[SYSVAR_FIELD_NUMBER] =
{
	".x",
	".y",
	".z",
	".rhw",
	".colour",
	".specular",
	".u",
	".v"
};

void SYSVAR_init()
{
	SLONG i;

	for (i = 0; i < SYSVAR_FIELD_NUMBER; i++)
	{
		ST_add(ST_TABLE_SYSTEM, SYSVAR_field[i], i, 0);
	}
}

