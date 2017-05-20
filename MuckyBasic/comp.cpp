#include "always.h"
#include "cg.h"
#include "lex.h"
#include "parse.h"
#include "st.h"
#include "vm.h"

#define COMP_MAX_PROGRAM (512 * 1024)		// Huge buffer!

CBYTE COMP_program[COMP_MAX_PROGRAM];

SLONG COMP_do(CBYTE *fname_input, CBYTE *fname_output)
{
	//
	// Load program.
	//

	FILE *handle = fopen(fname_input, "rb");

	if (!handle)
	{
		//
		// Could not open input file.
		//

		return FALSE;
	}

	//
	// Load in the source.
	//

	SLONG bytes_read;

	bytes_read = fread(COMP_program, sizeof(CBYTE), COMP_MAX_PROGRAM, handle);

	if (bytes_read == 0)
	{
		//
		// No data read?
		//

		return FALSE;
	}

	if (bytes_read >= COMP_MAX_PROGRAM)
	{
		//
		// Our source buffer isn't large enough!
		//

		return FALSE;
	}

	ST_init();

	PARSE_do(COMP_program);

	CG_do(fname_output);

	return TRUE;
}
