//
// Image compression.
//

#ifndef _IC_
#define _IC_

#include "tga.h"


//
// Compresses the given 4x4 block of a TGA using the mad S3 texture compression
// system.  Ignores the alpha channel of course!
//

typedef struct
{
	UWORD colour1;	// 5:6:5
	UWORD colour2;	// 5:6:5
	ULONG bit;

} IC_Packet;

IC_Packet IC_pack(
			TGA_Pixel *tga, 
			SLONG      tga_width,
			SLONG      tga_height,
			SLONG      px,
			SLONG      py);


//
// Fills in a 4x4 block of the given tga with the colour data
// from a compressed IC packet.
//

void IC_unpack(
			IC_Packet  ip,
			TGA_Pixel *tga, 
			SLONG      tga_width,
			SLONG      tga_height,
			SLONG      px,
			SLONG      py);



//
// Debug test...	
//

void IC_test(void);


#endif
