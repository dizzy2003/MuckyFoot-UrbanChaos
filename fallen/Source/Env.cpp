//
// Loads an environment from a file.
//

#include "game.h"
#include <MFStdLib.h>
#include "env.h"

#if 0

//
// The heap on which the environment variables are stored.
//

#define ENV_HEAP_SIZE 16384

CBYTE  ENV_heap[ENV_HEAP_SIZE];
CBYTE *ENV_heap_upto;

//
// The environment variables.
//

#define ENV_MAX_VARS 512

typedef struct
{
	CBYTE *name;
	CBYTE *val;

} ENV_Var;

ENV_Var ENV_var[ENV_MAX_VARS];
SLONG   ENV_var_upto;




void ENV_load(CBYTE *fname)
{
	SLONG  i;
	CBYTE *ch;

	CBYTE *heap_name;
	CBYTE *heap_val;

#ifndef PSX
	FILE *handle;
	
	#define ENV_MAX_LINE_LENGTH 256

	CBYTE line[ENV_MAX_LINE_LENGTH];
	CBYTE name[ENV_MAX_LINE_LENGTH];
	CBYTE val [ENV_MAX_LINE_LENGTH];

	//
	// Clear the old environment.
	//

	ENV_heap_upto = ENV_heap;
	ENV_var_upto  = 0;

	//
	// Load in the new environment.
	//

	handle = MF_Fopen(fname, "rb");

	if (handle == NULL)
	{
		return;
	}
	
	while(fgets(line, ENV_MAX_LINE_LENGTH, handle))
	{
		if (line[0] == '#')
		{
			//
			// This is a comment line?
			//

			continue;
		}

		//
		// Look for a name.
		//

		for (ch = line, i = 0; *ch && (isalnum(*ch) || *ch == '_'); ch++, i++)
		{
			ASSERT(i < ENV_MAX_LINE_LENGTH);

			name[i] = *ch;
		}

		name[i] = '\000';

		if (name[0] == '\000')
		{
			//
			// We didn't find a valid name.
			//
			
			continue;
		}

		//
		// Look for an equals sign.
		//

		while(*ch && *ch != '=') {ch++;}

		if (*ch == '\000')
		{
			//
			// We didn't find an equals sign.
			//

			continue;
		}

		ch += 1;

		//
		// Look for the start of the value.
		//

		while(*ch && isspace(*ch)) {ch++;}

		if (*ch == '\000')
		{
			//
			// We didn't find the start of the value.
			//

			continue;
		}

		//
		// Find the value.
		//

		i = 0;

		while(1)
		{
			ASSERT(i < ENV_MAX_LINE_LENGTH);

			if (*ch == '\000' ||
				*ch == '\n'   ||
				*ch == '\r'   ||
				*ch == '\f'   ||
				iscntrl(*ch))
			{
				val[i++] = '\000';
				break;
			}
			else
			{
				val[i++] = *ch;
			}

			ch++;
		}

		//
		// Store the name/value pair on the heap.
		//

		heap_name = ENV_heap_upto;

		ch = name;

		do
		{
			ASSERT(WITHIN(ENV_heap_upto, &ENV_heap[0], &ENV_heap[ENV_HEAP_SIZE - 1]));

		   *ENV_heap_upto++ = *ch;
		}
		while(*ch++);

		heap_val = ENV_heap_upto;
		
		ch = val;

		do
		{
			ASSERT(WITHIN(ENV_heap_upto, &ENV_heap[0], &ENV_heap[ENV_HEAP_SIZE - 1]));

		   *ENV_heap_upto++ = *ch;
		}
		while(*ch++);
	
		//
		// Create a new environment variable.
		//

		ASSERT(WITHIN(ENV_var_upto, 0, ENV_MAX_VARS - 1));

		TRACE("ENV: %s = %s\n", heap_name, heap_val);

		ENV_var[ENV_var_upto].name = heap_name;
		ENV_var[ENV_var_upto].val  = heap_val;

		ENV_var_upto += 1;
	}

	MF_Fclose(handle);

#endif
}

#ifndef PSX
SLONG ENV_cmp_insensitive(CBYTE *str1, CBYTE *str2)
{
	while (1)
	{
		if (toupper(*str1) < toupper(*str2)) {return -1;}
		if (toupper(*str1) > toupper(*str2)) {return +1;}

		if (*str1 == 0) return 0;

		str1 += 1;
		str2 += 1;
	}
}
#endif

CBYTE *ENV_get_value_string(CBYTE *name)
{
	SLONG i;

#ifndef PSX
	//
	// Search backwards so that you get the last value assigned to a name.
	//

	for (i = ENV_var_upto - 1; i >= 0; i--)
	{
		if (ENV_cmp_insensitive(ENV_var[i].name, name) == 0)
		{
			return ENV_var[i].val;
		}
	}

	//
	// Name not found.
	//
#endif
	return NULL;
}

SLONG  ENV_get_value_number(CBYTE *name, SLONG def)
{
	CBYTE *str;
	SLONG  val;
#ifndef PSX
	//
	// Find the string...
	//

	str = ENV_get_value_string(name);

	if (str == NULL)
	{
		return def;
	}
	else
	{
		val = atol(str);

		TRACE("val = %d\n", val);

		return val;
	}
#else
	return 0;
#endif
}

#endif