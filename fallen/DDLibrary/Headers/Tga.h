//
// Loads in 32-bit RGBA uncompressed TGAs.
//

#ifndef _TGA_
#define _TGA_

#include "FileClump.h"

//
// The format of a TGA pixel.
//

typedef struct
{
	UBYTE blue;
	UBYTE green;
	UBYTE red;
	UBYTE alpha;

} TGA_Pixel;

//
// Info describing the tga.
//

typedef struct
{
	SLONG valid;
	SLONG width;
	SLONG height;
	SLONG contains_alpha;
	
} TGA_Info;

//
// If the width and height of the tga exceed the maximums, then the tga is not loaded.
//

TGA_Info TGA_load(
			const CBYTE *file,
			SLONG        max_width,
			SLONG        max_height,
			TGA_Pixel   *data,
			ULONG		id,
			BOOL		bCanShrink = TRUE);

//
// Saves out a tga.
//

void TGA_save(
		const CBYTE *file,
		SLONG        width,
		SLONG        height,
		TGA_Pixel   *data,
		SLONG        contains_alpha);	// FALSE => Save without the alpha data.


// Clump management

void OpenTGAClump(const char* clumpfn, size_t maxid, bool readonly);
bool DoesTGAExist(const char* filename, ULONG id);
void CloseTGAClump();
FileClump* GetTGAClump();

#endif
