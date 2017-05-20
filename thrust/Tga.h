//
// Loads in 32-bit RGBA uncompressed TGAs.
//

#ifndef _TGA_
#define _TGA_


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

#define TGA_FLAG_CONTAINS_ALPHA	(1 << 0)
#define TGA_FLAG_ONE_BIT_ALPHA  (1 << 1)		// Alpha is only 255 or 0
#define TGA_FLAG_GRAYSCALE		(1 << 2)		// For all pixels r == g == b.

typedef struct
{
	SLONG valid;
	SLONG width;
	SLONG height;
	ULONG flag;
	
} TGA_Info;

//
// If the width and height of the tga exceed the maximums, then the tga is not loaded.
//

TGA_Info TGA_load(
			const CBYTE *file,
			SLONG        max_width,
			SLONG        max_height,
			TGA_Pixel   *data);


//
// Saves out a tga.
//

void TGA_save(
		const CBYTE *file,
		SLONG        width,
		SLONG        height,
		TGA_Pixel   *data,
		SLONG        contains_alpha);	// FALSE => Save without the alpha data.


#endif
