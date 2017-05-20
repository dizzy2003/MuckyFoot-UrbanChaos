//
// Loads in 32-bit RGBA uncompressed TGAs.
//

#include "always.h"
#include "tga.h"



TGA_Info TGA_load(
			const CBYTE *file,
			SLONG        max_width,
			SLONG        max_height,
			TGA_Pixel   *data)
{
	SLONG i;

	UBYTE red;
	UBYTE green;
	UBYTE blue;

	SLONG tga_width;
	SLONG tga_height;
	SLONG tga_pixel_depth;
	SLONG tga_image_type;
	SLONG tga_id_length;

	UBYTE header[18];
	UBYTE rubbish;
	UBYTE no_alpha;

	FILE *handle;

	TGA_Info ans;

	//
	// Open the file.
	//

	handle = fopen(file, "rb");

	if (handle == NULL)
	{
		TRACE("Could not open TGA file %s", file);
		ans.valid = FALSE;
		return ans;
	}

	//
	// Read the header.
	//

	if (fread(header, sizeof(UBYTE), 18, handle) != 18) goto file_error;

	//
	// Extract info from the header.
	//

	tga_id_length   = header[0x0];
	tga_image_type  = header[0x2];
	tga_width       = header[0xc] + header[0xd] * 256;
	tga_height      = header[0xe] + header[0xf] * 256;
	tga_pixel_depth = header[0x10];

	//
	// Is this a valid tga file?
	//

	ans.valid  = FALSE;
	ans.width  = tga_width;
	ans.height = tga_height;
	ans.flag   = 0;

	if (tga_image_type != 2)
	{
		TRACE("Tga must be true colour and uncompressed.\n");
		fclose(handle);
		return ans;
	}

	if (tga_pixel_depth != 32 && tga_pixel_depth != 24)
	{
		TRACE("Tga must be 32-bit or 24-bit (24-bit + 8-bit alpha channel)\n");
		fclose(handle);
		return ans;
	}
	
	if (tga_width  > max_width ||
		tga_height > max_height)
	{
		TRACE("Invalid dimensions:\n\tWanted <= %d x %d\n\tGot       %d x %d\n", max_width, max_height, tga_width, tga_height);
		fclose(handle);
		return ans;
	}

	//
	// The tga file is valid...
	//

	ans.valid = TRUE;

	//
	// Skip past the image identification field.
	//
	
	for (i = 0; i < tga_id_length; i++)
	{
		if (fread(&rubbish, sizeof(UBYTE), 1, handle) != 1) goto file_error;
	}

	//
	// Load in the data.
	//

	if (tga_pixel_depth == 32)
	{
		if (fread(data, sizeof(TGA_Pixel), tga_width * tga_height, handle) != tga_width * tga_height) goto file_error;

		no_alpha = FALSE;
	}
	else
	{
		//
		// We have to load a pixel in at a time to add the NULL alpha channel.
		//

		for (i = 0; i < tga_width * tga_height; i++)
		{
			if (fread(&blue,  sizeof(UBYTE), 1, handle) != 1) goto file_error;
			if (fread(&green, sizeof(UBYTE), 1, handle) != 1) goto file_error;
			if (fread(&red,   sizeof(UBYTE), 1, handle) != 1) goto file_error;
			
			data[i].red   = red;
			data[i].green = green;
			data[i].blue  = blue;
			data[i].alpha = 255;
		}

		no_alpha = TRUE;
	}

	fclose(handle);

	//
	// Loaded in the tga. Sets the flags- is it grayscale?
	//

	if (!no_alpha)
	{
		ans.flag |= TGA_FLAG_ONE_BIT_ALPHA;

		for (i = 0; i < tga_width * tga_height; i++)
		{
			if (data[i].alpha != 255)
			{
				//
				// Found alpha channel data.
				//

				ans.flag |= TGA_FLAG_CONTAINS_ALPHA;

				if (ans.flag != 0)
				{
					ans.flag &= ~TGA_FLAG_ONE_BIT_ALPHA;

					break;
				}
			}
		}

		if (!(ans.flag & TGA_FLAG_CONTAINS_ALPHA))
		{
			ans.flag &= ~TGA_FLAG_ONE_BIT_ALPHA;
		}
	}

	//
	// Is it grayscale?
	//

	ans.flag |= TGA_FLAG_GRAYSCALE;

	for (i = 0; i < tga_width * tga_height; i++)
	{
		if (data[i].red   != data[i].green ||
			data[i].red   != data[i].blue  ||
			data[i].green != data[i].blue)
		{
			ans.flag &= ~TGA_FLAG_GRAYSCALE;

			break;
		}
	}

	return ans;

  file_error:;

	//
	// Error!
	//

	TRACE("File error loading TGA file %s\n", file);
	fclose(handle);
	ans.valid = FALSE;

	return ans;
}








UBYTE TGA_header[18] =
{
	0, 0, 2, 0,
	0, 0, 0, 0, 
	0, 0, 0, 0, 
	0, 1,	// Width  LSB:MSB
	0, 1, 	// Height LSB:MSB
	24,		// Pixel depth
	0
};

void TGA_save(
		const CBYTE *file,
		SLONG        width,
		SLONG        height,
		TGA_Pixel   *data,
		SLONG contains_alpha)
{
	SLONG x;
	SLONG y;

	SLONG num_pixels;
	UBYTE header[18];
	SLONG bpp;

	FILE *handle;

	handle = fopen(file, "wb");

	if (handle == NULL)
	{
		TRACE("Cannot open TGA file %s\n", file);

		return;
	}

	//
	// Create the header.
	//

	SLONG i;

	for (i = 0; i < 18; i++)
	{
		header[i] = TGA_header[i];
	}

	header[0xc] = width  & 0xff;
	header[0xd] = width  >> 8;
	header[0xe] = height & 0xff;
	header[0xf] = height >> 8;

	header[0x10] = (contains_alpha) ? 32 : 24;

	//
	// Write out the header.
	//

	fwrite(&header, sizeof(UBYTE), 18, handle);

	//
	// Write out the pixel data.
	//

	for (y = width - 1; y >=     0; y--)
	for (x =         0; x <  width; x++)
	{
		if (contains_alpha)
		{
			fwrite(&data[x + y * width].alpha, sizeof(UBYTE), 1, handle);
		}

		fwrite(&data[x + y * width].blue,  sizeof(UBYTE), 1, handle);
		fwrite(&data[x + y * width].green, sizeof(UBYTE), 1, handle);
		fwrite(&data[x + y * width].red,   sizeof(UBYTE), 1, handle);
	}

	fclose(handle);
}
