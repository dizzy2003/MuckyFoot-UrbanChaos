//
// Looks at all the textures loaded and packs similar ones
// onto 256x256s...
//

#include <ddlib.h>
#include "game.h"
#include "tga.h"
#include "texture.h"


#ifndef TARGET_DC


//
// From elev.cpp...
//

extern CBYTE ELEV_fname_level[];


//
// From texture.cpp
//

extern D3DTexture TEXTURE_texture[];


//
// The current texture we are creating.
//

TGA_Pixel PACK_tga[256 * 256];


//
// Temporary buffer for a texture we've loaded in.
//

TGA_Pixel PACK_buffer[64 * 64];


//
// The files packed onto our current texture.
//

CBYTE PACK_fname[9][256];
CBYTE PACK_fname_dir[256];
SLONG PACK_fname_upto;

SLONG PACK_texture_upto;

CBYTE PACK_output_directory[256];
CBYTE PACK_fname_array[256];

FILE *PACK_fname_array_handle;




#define MAX_NUM_PACKED_PAGES 100

int iNumPackedPages;
D3DPage pagePacking[MAX_NUM_PACKED_PAGES];


//
// Outputs the current packing texture.
//

UBYTE save_out_the_vqs;

#if 0
void PACK_output()
{
	SLONG i;

	CBYTE texture_filename[256];

	sprintf(texture_filename, "%s\\PackedTexture%03d.tga", PACK_output_directory, PACK_texture_upto);

	//
	// Got a whole new texture.
	//

	TGA_save(texture_filename, 256, 256, (TGA_Pixel *) PACK_tga, FALSE);


	//
	// If you want to run this bit of code then do a search for 'save_out_the_vqs'
	// in d3dtexture.cpp and re-enable if (save_out_the_vqs)
	//

	save_out_the_vqs = TRUE;

	{
		D3DTexture *pTex;

		#define DO_DC_CONVERT(name) pTex = MFnew<D3DTexture>(); pTex->LoadTextureTGA ( (name), -1, TRUE ); MFdelete ( pTex )

		DO_DC_CONVERT(texture_filename);
	}

	save_out_the_vqs = FALSE;
	

	//
	// Write out the array.
	//

	fprintf(PACK_fname_array_handle, "\t\"***64_3:");

	//
	// Now output the directory containing the texture. Becuase this is 'C' code we
	// are generating, each '\' must be written out as '\\'.
	//

	CBYTE *ch;

	for (ch = PACK_fname_dir; *ch; ch++)
	{
		if (*ch == '\\')
		{
			fprintf(PACK_fname_array_handle, "\\\\");
		}
		else
		{
			fprintf(PACK_fname_array_handle, "%c", *ch);
		}
	}

	//
	// Write in the rest of the line.
	//

	fprintf(PACK_fname_array_handle, "\\\\\", \"");

	for (ch = texture_filename; *ch; ch++)
	{
		if (*ch == '\\')
		{
			fprintf(PACK_fname_array_handle, "\\\\");
		}
		else
		{
			fprintf(PACK_fname_array_handle, "%c", *ch);
		}
	}

	fprintf(PACK_fname_array_handle, "\",\n");

	for (i = 0; i < 9; i++)
	{
		fprintf(PACK_fname_array_handle, "\t\"%s\",\n", PACK_fname[i]);
	}

	fprintf(PACK_fname_array_handle, "\n");

	PACK_texture_upto += 1;
	PACK_fname_upto    = 0;

	memset(PACK_fname,      0, sizeof(PACK_fname    ));
	memset(PACK_fname_dir,  0, sizeof(PACK_fname_dir));

	memset(PACK_tga, 0, sizeof(PACK_tga));
}
#endif


void read_string ( FILE *handle, char *string )
{
	char *pch = string;
	while ( TRUE )
	{
		int character = fgetc ( handle );
		// Silly person - didn't finish your file properly.
		ASSERT ( character != EOF );
		if ( ( character == '\0' ) || ( character == '\n' ) || ( character == EOF ) )
		{
			*pch = '\0';
			break;
		}
		*pch++ = (char)character;
	}
}

char *copy_string ( char *input )
{
	char *temp = (char *)MemAlloc ( strlen ( input ) + 1 );
	ASSERT ( temp != NULL );
	strcpy ( temp, input );
	return temp;
}

void PACK_do()
{
	SLONG i;
	SLONG j;

	SLONG reqd_page_type;
	SLONG page_size;
	SLONG texture_size;
	SLONG border_size;

	TGA_Info    ti;
	D3DTexture *tt;

	CBYTE level_name [256];
	CBYTE texture_dir[256];
	
	//
	// The level name without spaces and without the extension...
	//

	CBYTE *ch;
	CBYTE *just_filename;


	TRACE ( "PACKing level %s", ELEV_fname_level );



	for (ch = ELEV_fname_level; *ch; ch++);

	while(1)
	{
		ch -= 1;

		if (ch == ELEV_fname_level)
		{
			break;
		}

		if (*ch == '\\')
		{
			ch += 1;

			break;
		}
	}

	CBYTE *src;
	CBYTE *dest;

	src  = ch;
	dest = level_name;

	while(1)
	{
		if (*src == '.' || *src == '\000')
		{
			*dest = '\000';

			break;
		}

		if (*src == ' ')
		{
			src++;
		}
		else
		{
			*dest++ = *src++;
		}
	}

	char PACK_text_array[100];
	FILE *PACK_text_array_handle;

	sprintf(PACK_output_directory, "PackedTextures\\%s", level_name);
	sprintf(PACK_fname_array, "%s\\array_%s.cpp", PACK_output_directory, level_name);
	sprintf(PACK_text_array, "%s\\text_%s.txt", PACK_output_directory, level_name);

	//
	// Create the directory we will be outputting into.
	//

	CreateDirectory(PACK_output_directory, NULL);

	//
	// Start off the array...
	//

	PACK_fname_array_handle = fopen(PACK_fname_array, "wb");
	if (!PACK_fname_array_handle)
	{
		ASSERT ( FALSE );
		return;
	}
	PACK_text_array_handle = fopen(PACK_text_array, "wt");
	if (!PACK_text_array_handle)
	{
		ASSERT ( FALSE );
		return;
	}

	fprintf(PACK_fname_array_handle, "//\n");
	fprintf(PACK_fname_array_handle, "// Packing array for \"%s\"\n", ELEV_fname_level);
	fprintf(PACK_fname_array_handle, "//\n");
	fprintf(PACK_fname_array_handle, "\n");
	fprintf(PACK_fname_array_handle, "CBYTE *packed_array_%s[] = \n", level_name);
	fprintf(PACK_fname_array_handle, "{\n");

	//
	// Go through all the textures.
	//



	// Read in any hand-done ones first & construct the corresponding textures.
	// The filename is int he same directory, called "hand_done_[levname].txt".
	// It has the same format as the included ones, except one-per-line,
	// and there is no explicit texture name - that's auto-generated.
	//
	// ***64_4:server\\textures\\extras\\DC\\
	// button_A.tga
	// button_B.tga
	// button_C.tga
	// button_X.tga
	// button_Y.tga
	// button_Z.tga
	// button_LEFT.tga
	// button_RIGHT.tga
	// button_PADLEFT.tga
	// button_PADRIGHT.tga
	// button_PADDOWN.tga
	// button_PADUP.tga

	iNumPackedPages = 0;

	// Find the list.

	char PACK_file_in[100];
	sprintf(PACK_file_in, "%s\\hand_done_%s.txt", PACK_output_directory, level_name);

	//
	// Start off the array...
	//

	FILE *PACK_file_in_handle = fopen(PACK_file_in, "rt");
	if (PACK_file_in_handle)
	{
		// Scan my text list, initialising the D3DPages as we go.
		int iPageType = 0;
		int iPagePos = 0;
		int iSafetyTimeout = 1000;

		char pcCurString[500];
		// Read the first string.
		read_string ( PACK_file_in_handle, pcCurString );
		while ( TRUE )
		{

			iSafetyTimeout--;
			if ( iSafetyTimeout <= 0 )
			{
				// Oops.
				ASSERT ( FALSE );
				break;
			}

			// This should be a command.
			//char *pcCurString = *ppcList;
			ASSERT ( ( pcCurString[0] == '*' ) && ( pcCurString[1] == '*' ) && ( pcCurString[2] == '*' ) );
			if ( 0 == stricmp ( pcCurString+3, "END" ) )
			{
				// End of list.
				break;
			}

			// Find the page type
			if ( 0 == strncmp ( pcCurString, "***64_4:", 8 ) )
			{
				iPageType = D3DPAGE_64_4X4;
			}
			else if ( 0 == strncmp ( pcCurString, "***32_4:", 8 ) )
			{
				iPageType = D3DPAGE_32_4X4;
			}
			else if ( 0 == strncmp ( pcCurString, "***64_3:", 8 ) )
			{
				iPageType = D3DPAGE_64_3X3;
			}
			else if ( 0 == strncmp ( pcCurString, "***32_3:", 8 ) )
			{
				iPageType = D3DPAGE_32_3X3;
			}
			else
			{
				ASSERT ( FALSE );
			}

			// Set up the texture page.
			pagePacking[iNumPackedPages].bPageType = (UBYTE)iPageType;
			pagePacking[iNumPackedPages].bNumTextures = 0;
			pagePacking[iNumPackedPages].pcDirectory = copy_string ( pcCurString + 8 );

			// Name it.
			char pcTemp[100];
			sprintf ( pcTemp, "%s\\PackedTexture%03d.tga", PACK_output_directory, iNumPackedPages);
			pagePacking[iNumPackedPages].pcFilename = copy_string ( pcTemp );

			//pagePacking[iNumPackedPages].ppcTextureList = ppcList;
			pagePacking[iNumPackedPages].pTex = NULL;

			// See how many textures there are.
			pagePacking[iNumPackedPages].ppcTextureList = (char **)MemAlloc ( 16 * sizeof ( char * ) );
			for ( int iTemp = 0; iTemp < 16; iTemp++ )
			{
				pagePacking[iNumPackedPages].ppcTextureList[iTemp] = NULL;
			}

			read_string ( PACK_file_in_handle, pcCurString );
			while ( pcCurString[0] != '*' )
			{
				if ( pcCurString[0] == '\0' )
				{
					// Empty slot.
					pagePacking[iNumPackedPages].ppcTextureList[pagePacking[iNumPackedPages].bNumTextures] = NULL;
				}
				else
				{
					// Make sure it hasn't already been added.
					for ( int i = 0; i <= iNumPackedPages; i++ )
					{
						if ( 0 == stricmp ( pagePacking[i].pcDirectory, pagePacking[iNumPackedPages].pcDirectory ) )
						{
							for ( int j = 0; j < pagePacking[i].bNumTextures; j++ )
							{
								if ( ( pagePacking[i].ppcTextureList[j] != NULL ) &&
									 ( 0 == stricmp ( pagePacking[i].ppcTextureList[j], pcCurString ) ) )
								{
									// Silly person.
									ASSERT ( FALSE );
									// Skip it.
									goto try_the_next_texture_coz_this_is_not_needed;
								}
							}
						}
					}
					// Also make sure we actually need it for this level.
					{
						bool bFound = FALSE;
						for (i = 0; i < 512; i++)
						{
							tt = &TEXTURE_texture[i];

							if (tt->Type == D3DTEXTURE_TYPE_UNUSED)
							{
								continue;
							}
							if (POLY_page_flag[i])
							{
								// We can't pack this page because it's a funny one.
								continue;
							}

							// Extract directory name.
							strcpy(texture_dir, tt->texture_name);

							for (ch = texture_dir; *ch; ch++);
							while(ch > texture_dir && *ch != '\\') {ch--;}

							if (*ch == '\\')
							{
								*ch = '\000';
							}

							just_filename = ch + 1;

							if ( 0 == stricmp ( just_filename, pcCurString ) )
							{
								// Right name.

								SLONG strlen_texture_dir  = strlen(texture_dir);
								SLONG strlen_pagepack_dir = strlen(pagePacking[iNumPackedPages].pcDirectory);

								if (strlen_pagepack_dir == strlen_texture_dir + 1)
								{
									if ( 0 == strnicmp ( texture_dir, pagePacking[iNumPackedPages].pcDirectory,  strlen_texture_dir ) )
									{
										// And same directory.
										bFound = TRUE;
										break;
									}
								}
							}
						}
						if ( !bFound )
						{
							// No, this doesn't need to be loaded.
							ASSERT ( FALSE );
							goto try_the_next_texture_coz_this_is_not_needed;
						}
					}

					pagePacking[iNumPackedPages].ppcTextureList[pagePacking[iNumPackedPages].bNumTextures] = copy_string ( pcCurString );
				}
				//pagePacking[iNumPackedPages].pTextures[pagePacking[iNumPackedPages].bNumTextures] = NULL;
				pagePacking[iNumPackedPages].bNumTextures++;
try_the_next_texture_coz_this_is_not_needed:
				read_string ( PACK_file_in_handle, pcCurString );
			}

			switch ( iPageType )
			{
			case D3DPAGE_64_4X4:
			case D3DPAGE_32_4X4:
				ASSERT ( pagePacking[iNumPackedPages].bNumTextures <= 16 );
				break;
			case D3DPAGE_64_3X3:
			case D3DPAGE_32_3X3:
				ASSERT ( pagePacking[iNumPackedPages].bNumTextures <= 9 );
				break;
			default:
				ASSERT ( FALSE );
				break;
			}

			iNumPackedPages++;
		}

		fclose(PACK_file_in_handle);
	}



	PACK_fname_upto   = 0;
	PACK_texture_upto = 0;

	memset(PACK_fname,      0, sizeof(PACK_fname    ));
	memset(PACK_fname_dir,  0, sizeof(PACK_fname_dir));

	for (i = 0; i < 64 * 22; i++)
	{
		tt = &TEXTURE_texture[i];

		if (tt->Type == D3DTEXTURE_TYPE_UNUSED)
		{
			continue;
		}

		if (POLY_page_flag[i])
		{
			// We can't pack this page because it's a funny one.
			continue;
		}


		// Extract directory name.
		strcpy(texture_dir, tt->texture_name);

		for (ch = texture_dir; *ch; ch++);
		while(ch > texture_dir && *ch != '\\') {ch--;}

		if (*ch == '\\')
		{
			*ch = '\000';
		}

		just_filename = ch + 1;


		// See if it's in a texture page already
		bool bFound = FALSE;
		for ( int iPageNum = 0; iPageNum < iNumPackedPages; iPageNum++ )
		{
			//
			// The texture_dir doesn't have a final backslash whereas
			// the packPacking directory does...
			//

			SLONG strlen_texture_dir  = strlen(texture_dir);
			SLONG strlen_pagepack_dir = strlen(pagePacking[iPageNum].pcDirectory);

			if (strlen_pagepack_dir == strlen_texture_dir + 1)
			{
				if ( strnicmp ( texture_dir, pagePacking[iPageNum].pcDirectory, strlen_texture_dir) == 0 )
				{
					for ( int iTexNum = 0; iTexNum < pagePacking[iPageNum].bNumTextures; iTexNum++ )
					{
						if ( ( pagePacking[iPageNum].ppcTextureList[iTexNum] != NULL ) &&
							 ( 0 == stricmp ( pagePacking[iPageNum].ppcTextureList[iTexNum], just_filename ) ) )
						{
							bFound = TRUE;
							break;
						}
					}
				}
			}

			if ( bFound )
			{
				break;
			}
		}
		if ( bFound )
		{
			// Already got it.
			continue;
		}

		//
		// What size of texture do we have and which page type
		// do we need?
		//

		texture_size = tt->size;

		if (texture_size == 64)
		{
			if (strstr(texture_dir, "people"))
			{
				//
				// This is a people page, pack it 4x4
				//

				reqd_page_type = D3DPAGE_64_4X4;
			}
			else
			{
				//
				// Put world textures on a 3x3 because of wrapping...
				//

				reqd_page_type = D3DPAGE_64_3X3;

				if (i >= 64 * 8)
				{
					//
					// Try squeezing in the prims!
					//

					reqd_page_type = D3DPAGE_64_4X4;
				}
			}
		}
		else
		if (texture_size == 32)
		{
			if (strstr(texture_dir, "people"))
			{
				//
				// This is a people page, pack it 4x4
				//

				reqd_page_type = D3DPAGE_32_4X4;
			}
			else
			{
				//
				// Put world textures on a 3x3 because of wrapping...
				//

				reqd_page_type = D3DPAGE_32_3X3;

				if (i >= 64 * 8)
				{
					//
					// Try squeezing in the prims!
					//

					reqd_page_type = D3DPAGE_32_4X4;
				}
			}
		}
		else
		{
			//
			// We don't support funny sizes...
			//

			continue;
		}

		ASSERT(reqd_page_type != 0);

		// OK, try to pack it into an existing page in the right directory with a gap.
		bFound = FALSE;
		for ( iPageNum = 0; iPageNum < iNumPackedPages; iPageNum++ )
		{
			// Only auto-pack into 3x3s
			if ( pagePacking[iPageNum].bPageType == reqd_page_type )
			{
				//
				// The texture_dir doesn't have a final backslash whereas
				// the packPacking directory does...
				//

				SLONG strlen_texture_dir  = strlen(texture_dir);
				SLONG strlen_pagepack_dir = strlen(pagePacking[iPageNum].pcDirectory);

				if (strlen_pagepack_dir == strlen_texture_dir + 1)
				{
					if ( strnicmp ( texture_dir, pagePacking[iPageNum].pcDirectory, strlen_texture_dir ) == 0 )
					{
						//
						// What's the max number of textures this pagepage can hold?
						//

						SLONG max_textures_in_this_page;

						if (reqd_page_type == D3DPAGE_32_3X3 ||
							reqd_page_type == D3DPAGE_64_3X3)
						{
							max_textures_in_this_page = 9;
						}
						else
						{
							max_textures_in_this_page = 16;
						}

						// Has it got a gap?

						if ( pagePacking[iPageNum].bNumTextures < max_textures_in_this_page )
						{
							// Yes, there's a gap.
							bFound = TRUE;
							break;
						}
					}
				}
			}
		}

		if ( !bFound )
		{
			// Make a new page then.
			iPageNum = iNumPackedPages;
			// Set up the texture page.
			pagePacking[iNumPackedPages].bPageType = reqd_page_type;
			pagePacking[iNumPackedPages].bNumTextures = 0;
			char pcTemp[100];
			strcpy ( pcTemp, texture_dir );
			strcat ( pcTemp, "\\" );
			pagePacking[iNumPackedPages].pcDirectory = copy_string ( pcTemp );

			//pagePacking[iNumPackedPages].ppcTextureList = ppcList;
			pagePacking[iNumPackedPages].pTex = NULL;

			// Allocate space for the source texture names.
			pagePacking[iNumPackedPages].ppcTextureList = (char **)MemAlloc ( 16 * sizeof ( char * ) );

			// Name it.
			sprintf ( pcTemp, "%s\\PackedTexture%03d.tga", PACK_output_directory, iNumPackedPages);
			pagePacking[iNumPackedPages].pcFilename = copy_string ( pcTemp );


			iNumPackedPages++;
		}

		// Add to this page.
		ASSERT ( pagePacking[iPageNum].bNumTextures < 16 );
		pagePacking[iPageNum].ppcTextureList[pagePacking[iPageNum].bNumTextures] = copy_string ( just_filename  );
		pagePacking[iPageNum].bNumTextures++;

	}


	// OK, so we've compiled our pages. Now write them out.
	
	for ( int iPageNum = 0; iPageNum < iNumPackedPages; iPageNum++ )
	{
		// First write the cpp file data.
		switch ( pagePacking[iPageNum].bPageType )
		{
			case D3DPAGE_64_4X4:
				fprintf ( PACK_fname_array_handle, "\"***64_4:" );
				fprintf ( PACK_text_array_handle, "***64_4:" );
				break;
			case D3DPAGE_32_4X4:
				fprintf ( PACK_fname_array_handle, "\"***32_4:" );
				fprintf ( PACK_text_array_handle, "***32_4:" );
				break;
			case D3DPAGE_64_3X3:
				fprintf ( PACK_fname_array_handle, "\"***64_3:" );
				fprintf ( PACK_text_array_handle, "***64_3:" );
				break;
			case D3DPAGE_32_3X3:
				fprintf ( PACK_fname_array_handle, "\"***32_3:" );
				fprintf ( PACK_text_array_handle, "***32_3:" );
				break;
			default:
				ASSERT ( FALSE );
				break;
		}

		//
		// Now output the directory containing the texture. Becuase this is 'C' code we
		// are generating, each '\' must be written out as '\\'.
		//

		CBYTE *ch;
		for (ch = pagePacking[iPageNum].pcDirectory; *ch; ch++)
		{
			if (*ch == '\\')
			{
				fprintf(PACK_fname_array_handle, "\\\\");
				fprintf(PACK_text_array_handle, "\\");
			}
			else
			{
				fprintf(PACK_fname_array_handle, "%c", *ch);
				fprintf(PACK_text_array_handle, "%c", *ch);
			}
		}

		//
		// Write in the rest of the line.
		//
		fprintf(PACK_fname_array_handle, "\", \"");
		fprintf(PACK_text_array_handle, "   ");

		for (ch = pagePacking[iPageNum].pcFilename; *ch; ch++)
		{
			if (*ch == '\\')
			{
				fprintf(PACK_fname_array_handle, "\\\\");
				fprintf(PACK_text_array_handle, "\\");
			}
			else
			{
				fprintf(PACK_fname_array_handle, "%c", *ch);
				fprintf(PACK_text_array_handle, "%c", *ch);
			}
		}

		fprintf(PACK_fname_array_handle, "\",\n");
		fprintf(PACK_text_array_handle, "\n");

		// Now write out the texture names.

		for (i = 0; i < pagePacking[iPageNum].bNumTextures; i++)
		{
			if ( pagePacking[iPageNum].ppcTextureList[i] == NULL )
			{
				fprintf(PACK_fname_array_handle, "\t\"\",\n" );
				fprintf(PACK_text_array_handle, "\n" );
			}
			else
			{
				fprintf(PACK_fname_array_handle, "\t\"%s\",\n", pagePacking[iPageNum].ppcTextureList[i]);
				fprintf(PACK_text_array_handle, "%s\n", pagePacking[iPageNum].ppcTextureList[i]);
			}
		}

		fprintf(PACK_fname_array_handle, "\n");

		//
		// How big are the page, the individual textures
		// and the border?
		//

		switch(pagePacking[iPageNum].bPageType)
		{
			case D3DPAGE_64_4X4: page_size = 256; break;
			case D3DPAGE_32_4X4: page_size = 128; break;
			case D3DPAGE_64_3X3: page_size = 256; break;
			case D3DPAGE_32_3X3: page_size = 128; break;

			default:
				ASSERT(0);
				break;
		}

		texture_size = page_size    >> 2;
		border_size  = texture_size >> 2;


		#define PLOT_PIX(fx,fy,tx,ty) if (WITHIN((tx), 0, page_size - 1) && WITHIN((ty), 0, page_size - 1) && WITHIN((fx), 0, page_size - 1) && WITHIN((fy), 0, page_size - 1)) PACK_tga[(tx) + (ty) * page_size] = PACK_tga[(fx) + (fy) * page_size]

		// Clear the page - don't want old data confusing the VQ rout & chewing palette entries.
		for ( int iY = 0; iY < 256; iY++ )
		{
			for ( int iX = 0; iX < 256; iX++ )
			{
				PACK_tga[iX + iY * 256].alpha = 0xff;
				PACK_tga[iX + iY * 256].blue  = 0;
				PACK_tga[iX + iY * 256].green = 0;
				PACK_tga[iX + iY * 256].red   = 0;
			}
		}

		// And now construct this page's texture.
		for ( int iTexNum = 0; iTexNum < pagePacking[iPageNum].bNumTextures; iTexNum++)
		{
			if ( pagePacking[iPageNum].ppcTextureList[iTexNum] == NULL )
			{
				// Empty slot.
				continue;
			}

			// Load up the tga.
			char pcTemp[100];
			strcpy ( pcTemp, pagePacking[iPageNum].pcDirectory );
			strcat ( pcTemp, pagePacking[iPageNum].ppcTextureList[iTexNum] );
			ti = TGA_load ( pcTemp, 64, 64, (TGA_Pixel *) PACK_buffer, 0, 0);
			if (!ti.valid)
			{
				// Nads.
				ASSERT ( FALSE );
			}

			ASSERT(ti.width  == texture_size);
			ASSERT(ti.height == texture_size);

			if (ti.valid)
			{

				//
				// Copy over the texture to the right place.
				//

				SLONG x;
				SLONG y;

				SLONG fx;
				SLONG fy;

				SLONG tx;
				SLONG ty;

				SLONG base_x;
				SLONG base_y;

				//
				// What is the base (x,y) for this position?
				//

				switch ( pagePacking[iPageNum].bPageType )
				{
				case D3DPAGE_64_4X4:
					base_x = (iTexNum % 4) * 64;
					base_y = (iTexNum / 4) * 64;
					break;
				case D3DPAGE_32_4X4:
					base_x = (iTexNum % 4) * 32;
					base_y = (iTexNum / 4) * 32;
					break;
				case D3DPAGE_64_3X3:
					base_x = (iTexNum % 3) * 96;
					base_y = (iTexNum / 3) * 96;
					break;
				case D3DPAGE_32_3X3:
					base_x = (iTexNum % 3) * 48;
					base_y = (iTexNum / 3) * 48;
					break;
				default:
					ASSERT ( FALSE );
					break;
				}

				//
				// Copy over the main body of the texture.
				//

				for (x = 0; x < texture_size; x++)
				{
					for (y = 0; y < texture_size; y++)
					{
						fx = x;
						fy = y;

						tx = base_x + x;
						ty = base_y + y;

						PACK_tga[tx + ty * page_size] = PACK_buffer[fx + fy * texture_size];
					}
				}
			}
		}

		if ( pagePacking[iPageNum].bPageType == D3DPAGE_64_3X3 ||
			 pagePacking[iPageNum].bPageType == D3DPAGE_32_3X3)
		{

			for ( int iTexNum = 0; iTexNum < pagePacking[iPageNum].bNumTextures; iTexNum++)
			{

				SLONG fx;
				SLONG fy;

				SLONG tx;
				SLONG ty;

				SLONG base_x;
				SLONG base_y;

				switch (pagePacking[iPageNum].bPageType)
				{
					case D3DPAGE_64_3X3:
						base_x = (iTexNum % 3) * 96;
						base_y = (iTexNum / 3) * 96;
						break;

					case D3DPAGE_32_3X3:
						base_x = (iTexNum % 3) * 48;
						base_y = (iTexNum / 3) * 48;
						break;

					default:
						ASSERT(0);
						break;
				}

				// Now do the edges so we effectively have clamping...
				for ( int iBorder = 1; iBorder <= border_size; iBorder++ )
				{
					for (j = 0; j < texture_size; j++)
					{
						//
						// Top...
						//

						fx = base_x + j;
						fy = base_y;

						tx = base_x + j;
						ty = base_y - iBorder;

						PLOT_PIX ( fx, fy, tx, ty );

						//
						// Bottom...
						//

						fx = base_x + j;
						fy = base_y + texture_size - 1;

						tx = base_x + j;
						ty = base_y + texture_size - 1 + iBorder;

						PLOT_PIX ( fx, fy, tx, ty );
						
						//
						// Left...
						//

						fx = base_x;
						fy = base_y + j;

						tx = base_x - iBorder;
						ty = base_y + j;

						PLOT_PIX ( fx, fy, tx, ty );

						//
						// Right...
						//

						fx = base_x + texture_size - 1;
						fy = base_y + j;

						tx = base_x + texture_size - 1 + iBorder;
						ty = base_y + j;

						PLOT_PIX ( fx, fy, tx, ty );
					}
				}


				for ( int iBordX = 1; iBordX <= border_size; iBordX++ )
				{
					for ( int iBordY = 1; iBordY <= border_size; iBordY++ )
					{
						//
						// Now do the corners.
						//

						//
						// Top left...
						//

						fx = base_x;
						fy = base_y;

						tx = base_x - iBordX;
						ty = base_y - iBordY;

						PLOT_PIX ( fx, fy, tx, ty );

						//
						// Bottom right...
						//

						fx = base_x + texture_size - 1;
						fy = base_y + texture_size - 1;

						tx = base_x + texture_size - 1 + iBordX;
						ty = base_y + texture_size - 1 + iBordY;

						PLOT_PIX ( fx, fy, tx, ty );
						
						//
						// Top right...
						//

						fx = base_x + texture_size - 1;
						fy = base_y;

						tx = base_x + texture_size - 1 + iBordX;
						ty = base_y - iBordY;

						PLOT_PIX ( fx, fy, tx, ty );

						//
						// Bottom left...
						//

						fx = base_x;
						fy = base_y + texture_size - 1;

						tx = base_x - iBordX;
						ty = base_y + texture_size - 1 + iBordY;

						PLOT_PIX ( fx, fy, tx, ty );
					}
				}
			}
		}
		else
		{
			// 4x4s often have gaps - fill them in too, but in a more cunning way.
			// Look for empty spaces.
			for ( int iTexNum = 0; iTexNum < 16; iTexNum++ )
			{
				if ( pagePacking[iPageNum].ppcTextureList[iTexNum] == NULL )
				{
					// Found a gap.
					// What surroundings does it have?
					bool bHasU = FALSE;
					bool bHasD = FALSE;
					bool bHasL = FALSE;
					bool bHasR = FALSE;
					if ( iTexNum >= 4 )
					{
						if ( pagePacking[iPageNum].ppcTextureList[iTexNum-4] != NULL )
						{
							bHasU = TRUE;
						}
					}
					if ( iTexNum <= 11 )
					{
						if ( pagePacking[iPageNum].ppcTextureList[iTexNum+4] != NULL )
						{
							bHasD = TRUE;
						}
					}
					if ( ( iTexNum % 4 ) > 0 )
					{
						if ( pagePacking[iPageNum].ppcTextureList[iTexNum-1] != NULL )
						{
							bHasL = TRUE;
						}
					}
					if ( ( iTexNum % 4 ) < 3 )
					{
						if ( pagePacking[iPageNum].ppcTextureList[iTexNum+1] != NULL )
						{
							bHasR = TRUE;
						}
					}

					SLONG base_x;
					SLONG base_y;

					switch (pagePacking[iPageNum].bPageType)
					{
						case D3DPAGE_64_4X4:
							base_x = (iTexNum % 4) * 64;
							base_y = (iTexNum / 4) * 64;
							break;

						case D3DPAGE_32_4X4:
							base_x = (iTexNum % 4) * 32;
							base_y = (iTexNum / 4) * 32;
							break;

						default:
							ASSERT(0);
							break;
					}

					SLONG diag_size = texture_size >> 1;


					// Do top-left corner.
					if ( bHasU )
					{
						if ( bHasL )
						{
							// Has both - fill both, diagonal join.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									if ( iPixY > iPixX )
									{
										// Copy from left.
										PLOT_PIX ( base_x - 1, base_y + iPixY, base_x + iPixX, base_y + iPixY );
									}
									else
									{
										// Copy from top.
										PLOT_PIX ( base_x + iPixX, base_y - 1, base_x + iPixX, base_y + iPixY );
									}
								}
							}
						}
						else
						{
							// Has top - fill from that.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from top.
									PLOT_PIX ( base_x + iPixX, base_y - 1, base_x + iPixX, base_y + iPixY );
								}
							}
						}
					}
					else
					{
						if ( bHasL )
						{
							// Has left.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from left.
									PLOT_PIX ( base_x - 1, base_y + iPixY, base_x + iPixX, base_y + iPixY );
								}
							}
						}
						else
						{
							// Has neither - copy from top-left.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									PLOT_PIX ( base_x - 1, base_y - 1, base_x + iPixX, base_y + iPixY );
								}
							}
						}
					}

					// Do top-right corner.
					if ( bHasU )
					{
						if ( bHasR )
						{
							// Has both - fill both, diagonal join.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									if ( iPixY > iPixX )
									{
										// Copy from right.
										PLOT_PIX ( base_x + texture_size, base_y + iPixY, base_x + texture_size - 1 - iPixX, base_y + iPixY );
									}
									else
									{
										// Copy from top.
										PLOT_PIX ( base_x + texture_size - 1 - iPixX, base_y - 1, base_x + texture_size - 1 - iPixX, base_y + iPixY );
									}
								}
							}
						}
						else
						{
							// Has top - fill from that.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from top.
									PLOT_PIX ( base_x + texture_size - 1 - iPixX, base_y - 1, base_x + texture_size - 1 - iPixX, base_y + iPixY );
								}
							}
						}
					}
					else
					{
						if ( bHasR )
						{
							// Has right.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from right.
									PLOT_PIX ( base_x + texture_size, base_y + iPixY, base_x + texture_size - 1 - iPixX, base_y + iPixY );
								}
							}
						}
						else
						{
							// Has neither - copy from top-right.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									PLOT_PIX ( base_x + texture_size, base_y - 1, base_x + texture_size - 1 - iPixX, base_y + iPixY );
								}
							}
						}
					}

					// Do bottom-left corner.
					if ( bHasD )
					{
						if ( bHasL )
						{
							// Has both - fill both, diagonal join.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									if ( iPixY > iPixX )
									{
										// Copy from left.
										PLOT_PIX ( base_x - 1, base_y + texture_size - 1 - iPixY, base_x + iPixX, base_y + texture_size - 1 - iPixY );
									}
									else
									{
										// Copy from bottom.
										PLOT_PIX ( base_x + iPixX, base_y + texture_size, base_x + iPixX, base_y + texture_size - 1 - iPixY );
									}
								}
							}
						}
						else
						{
							// Has bottom - fill from that.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from bottom.
									PLOT_PIX ( base_x + iPixX, base_y + texture_size, base_x + iPixX, base_y + texture_size - 1 - iPixY );
								}
							}
						}
					}
					else
					{
						if ( bHasL )
						{
							// Has left.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from left.
									PLOT_PIX ( base_x - 1, base_y + texture_size - 1 - iPixY, base_x + iPixX, base_y + texture_size - 1 - iPixY );
								}
							}
						}
						else
						{
							// Has neither - copy from bottom-left.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									PLOT_PIX ( base_x - 1, base_y + texture_size, base_x + iPixX, base_y + texture_size - 1 - iPixY );
								}
							}
						}
					}

					// Do bottom-right corner.
					if ( bHasD )
					{
						if ( bHasR )
						{
							// Has both - fill both, diagonal join.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									if ( iPixY > iPixX )
									{
										// Copy from right.
										PLOT_PIX ( base_x + texture_size, base_y + texture_size - 1 - iPixY, base_x + texture_size - 1 - iPixX, base_y + texture_size - 1 - iPixY );
									}
									else
									{
										// Copy from bottom.
										PLOT_PIX ( base_x + texture_size - 1 - iPixX, base_y + texture_size, base_x + texture_size - 1 - iPixX, base_y + texture_size - 1 - iPixY );
									}
								}
							}
						}
						else
						{
							// Has bottom - fill from that.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from bottom.
									PLOT_PIX ( base_x + texture_size - 1 - iPixX, base_y + texture_size, base_x + texture_size - 1 - iPixX, base_y + texture_size - 1 - iPixY );
								}
							}
						}
					}
					else
					{
						if ( bHasR )
						{
							// Has right.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									// Copy from right.
									PLOT_PIX ( base_x + texture_size, base_y + texture_size - 1 - iPixY, base_x + texture_size - 1 - iPixX, base_y + texture_size - 1 - iPixY );
								}
							}
						}
						else
						{
							// Has neither - copy from bottom-right.
							for ( int iPixX = 0; iPixX < diag_size; iPixX++ )
							{
								for ( int iPixY = 0; iPixY < diag_size; iPixY++ )
								{
									PLOT_PIX ( base_x + texture_size, base_y + texture_size, base_x + texture_size - 1 - iPixX, base_y + texture_size - 1 - iPixY );
								}
							}
						}
					}


				}
			}
		}



		// And finally write it out.


		CBYTE texture_filename[256];

		sprintf(texture_filename, "%s\\PackedTexture%03d.tga", PACK_output_directory, iPageNum );

		//
		// Got a whole new texture.
		//

		TGA_save(texture_filename, page_size, page_size, (TGA_Pixel *) PACK_tga, FALSE);


		//
		// If you want to run this bit of code then do a search for 'save_out_the_vqs'
		// in d3dtexture.cpp and re-enable if (save_out_the_vqs)
		//

		save_out_the_vqs = TRUE;

		{
			D3DTexture *pTex;

			#define DO_DC_CONVERT(name) pTex = MFnew<D3DTexture>(); pTex->LoadTextureTGA ( (name), -1, TRUE ); MFdelete ( pTex )

			DO_DC_CONVERT(texture_filename);
		}

		save_out_the_vqs = FALSE;

	}

	fprintf(PACK_fname_array_handle, "\t\"***END\"\n");
	fprintf(PACK_text_array_handle, "***END\n\n\n");
	fprintf(PACK_fname_array_handle, "};\n\n");

	fclose(PACK_fname_array_handle);
	fclose(PACK_text_array_handle);

	TRACE ( "Finished PACKing level %s", ELEV_fname_level );

}



#endif //#ifndef TARGET_DC
