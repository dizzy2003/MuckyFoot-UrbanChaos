//
// Loads an environment from a file.
//

#ifndef _ENV_
#define _ENV_


//
// now we use standard Windows .INI files
//

#ifndef TARGET_DC

void ENV_load(CBYTE *fname);

#else

// Oh no we don't...


// Feed it the start of the env save/load block, and
// it loads the block and returns a pointer to the end of the block.
extern char *ENV_load ( char *pcData );

// Call once with pcData = NULL, and it returns the number of
// bytes it needs to save the data. Call it again with a pointer
// to the start of this data, and it will actually fill in the data
// (and return the number of bytes again).
extern int ENV_save ( char *pcData );


#endif

//
// retrieve values
//

CBYTE *ENV_get_value_string(CBYTE *name, CBYTE* section = "Game");				// returns NULL if not found - NOTE: string is in a static buffer
SLONG  ENV_get_value_number(CBYTE *name, SLONG def, CBYTE* section = "Game");	// returns def if not found

//
// store values
//

void ENV_set_value_string(CBYTE* name, CBYTE* value, CBYTE* section = "Game");
void ENV_set_value_number(CBYTE* name, SLONG value, CBYTE* section = "Game");

#endif
