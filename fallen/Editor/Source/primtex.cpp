//
// Handles the allocation of prims and prim textures on n:\
//

#include	"Editor.hpp"
#include <windows.h>
#include <MFStdLib.h>
#include "primtex.h"





SLONG PRIMTEX_get_number(CBYTE *fname)
{
	SLONG page_number;
	SLONG line_number;
	SLONG match;
	SLONG i;
	SLONG a;

	FILE *handle_tga;
	FILE *handle_txt;
	FILE *handle_pge;

	CBYTE fullname_tga[MAX_PATH];
	CBYTE fullname_txt[MAX_PATH];
	CBYTE fullname_pge[MAX_PATH];
	CBYTE line        [MAX_PATH];

	CBYTE *ch;

	//
	// Work out the full path of this file on n:\
	//

	sprintf(fullname_tga, "u:\\urbanchaos\\textures\\shared\\MaxTex\\%s", fname);
	sprintf(fullname_txt, "u:\\urbanchaos\\textures\\shared\\MaxTex\\%s", fname);

	//
	// Change the fullname_txt file to have .txt at the end of it!
	//

	for (ch = fullname_txt; *ch; ch++);

	ch[-3] = 't';
	ch[-2] = 'x';
	ch[-1] = 't';

	//
	// Does this texture exist?
	//

	handle_tga = fopen(fullname_tga, "rb");

	if (handle_tga == NULL)
	{
		//
		// Could not find this texture- just use the question mark texture.
		//

		return PRIMTEX_PAGENUMBER_QMARK;
	}

	fclose(handle_tga);

	//
	// This tga does exist. Is it already allocated a page number?
	//

	handle_txt = fopen(fullname_txt, "rb");

	if (handle_txt)
	{
		page_number = PRIMTEX_PAGENUMBER_QMARK;

		while(fgets(line, MAX_PATH, handle_txt))
		{
			match = sscanf(line, "Page number: %d", &a);

			if (match == 1)
			{
				page_number = a;

				break;
			}
		}

		fclose(handle_txt);

		return page_number;
	}

	//
	// We have not allocated this texture file a page number yet. ARGH! Go through
	// the pages in the n:\urbanchaos\textures\shared\prims directory to find the
	// first unused slot.
	//
	
	page_number = PRIMTEX_PAGENUMBER_QMARK;

	for (i = 1; i < 256; i++)	// Page 0 is reserved for qmark.tga
	{
		sprintf(fullname_pge, "u:\\urbanchaos\\textures\\shared\\prims\\tex%03d.tga", i);

		handle_pge = fopen(fullname_pge, "rb");

		if (!handle_pge)
		{
			page_number = i;

			break;
		}

		fclose(handle_pge);
	}

	if (page_number == PRIMTEX_PAGENUMBER_QMARK)
	{
		//
		// No spare textures!
		//

		return PRIMTEX_PAGENUMBER_QMARK;
	}

	//
	// Make a note of where we have put this texture.
	//

	handle_txt = fopen(fullname_txt, "wb");

	if (!handle_txt)
	{
		//
		// Oh dear. What up here?
		//

		return PRIMTEX_PAGENUMBER_QMARK;
	}

	fprintf(handle_txt, "Page number: %d\n", page_number);
	fclose (handle_txt);

	//
	// Copy this tga to the new filename.
	//

	CopyFile(
		fullname_tga,
		fullname_pge,
		TRUE);		// TRUE => don't overwrite an existing file- doesn't matter because fullname_pga doesn't exist.

	return page_number;
}
