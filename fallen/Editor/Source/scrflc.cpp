//	Hello


//**************************************|************************************

#include	"Editor.hpp"


//**************************************|************************************





static	struct	Animation						animation[1];

int	ScreenWidth=320;
int	ScreenHeight=200;
//**************************************|************************************

SLONG	anim_record()
{
	SBYTE	file_name[128];
	MFFileHandle	fpz;
	SLONG	anim_number=0;

//	if (Display.ScreenMode == MODE_MCGA)
	{
//		for (anim_number = 0; anim_number < 10000; anim_number++)
		{
			//sprintf(file_name, DIR_FLICS"game%04d.flc", anim_number);

			// AW180595
			sprintf((char*)file_name,"anim.flc", anim_number);

//			if ((fpz = FileCreate((CBYTE *)file_name,1) !=FILE_OPEN_ERROR)
			{
				return(anim_open(file_name, 0, 0, 320, 200, 0, PLAYBACK_MODE_RECORD ));
			}
//			else
			{
//				FileClose(fpz );
			}
		}
	}
	return(0);
}

SLONG	anim_record_frame(UBYTE *screen, UBYTE *palette)
{
	if ( (animation->PlaybackMode & PLAYBACK_MODE_RECORD))
	{
		return(anim_make_next_frame(screen, palette));
	}
	return(0);
}

SLONG	anim_stop()
{
	return(anim_close(PLAYBACK_MODE_RECORD));
}

//**************************************|************************************

SLONG	anim_open(SBYTE *file_name, SWORD xpos, SWORD ypos, SWORD width, SWORD height, SBYTE *postage_stamp, SLONG	playback )
{
	if ((animation->PlaybackMode & playback) == 0)
	{
		if (playback & PLAYBACK_MODE_RECORD)
		{
			memset(animation, 0, sizeof(struct Animation));
			animation->PlaybackMode |= playback;

			animation->LastFrame = (UBYTE *)MemAlloc(ScreenWidth * ScreenHeight * 2);
			animation->NextFrameBuffer = (UBYTE *)MemAlloc(ScreenWidth * ScreenHeight * 2);
			animation->RecordFileHandle = FileCreate((CBYTE *)file_name, 1);

			if(animation->LastFrame==0||animation->NextFrameBuffer==0||animation->RecordFileHandle==0)
				return(0);

			animation->FLCFileHeader.Size				=	sizeof(struct FLCFileHeader);
			animation->FLCFileHeader.Magic				=	0xAF12;
			animation->FLCFileHeader.NumberOfFrames		=	0;
			animation->FLCFileHeader.Width				=	ScreenWidth;
			animation->FLCFileHeader.Height				=	ScreenHeight;
			animation->FLCFileHeader.Depth				=	8;
			animation->FLCFileHeader.Flags				=	3;
			animation->FLCFileHeader.Speed				=	57;
			animation->FLCFileHeader.Reserved_0			=	0;
			animation->FLCFileHeader.Created			=	0;
			animation->FLCFileHeader.Creator			=	0x464C4942;
			animation->FLCFileHeader.Updated			=	0;
			animation->FLCFileHeader.Updater			=	0x464C4942;
			animation->FLCFileHeader.AspectX			=	6;
			animation->FLCFileHeader.AspectY			=	5;
			memset(animation->FLCFileHeader.Reserved_1, 0, sizeof(animation->FLCFileHeader.Reserved_1));
			animation->FLCFileHeader.OFrame1			=	0;
			animation->FLCFileHeader.OFrame2			=	0;
			memset(animation->FLCFileHeader.Reserved_2, 0, sizeof(animation->FLCFileHeader.Reserved_2));

			animation->Xpos								=	xpos;
			animation->Xpos								=	ypos;

			if (anim_write_data((UBYTE *)&animation->FLCFileHeader, sizeof (struct FLCFileHeader )) == 0)
			{
				FileClose(animation->RecordFileHandle );
				return(0);
			}

			animation->FrameSizeMaximum					=	ScreenHeight * ScreenWidth + 1024;
			animation->FrameNumber						=	0;

			memset(animation->Palette, -1, 256 * 3);
		}
		if (playback & PLAYBACK_MODE_PLAY)
		{
/*
			animation->PlaybackMode |= playback;
			if ((animation->NextFrameBuffer = (UBYTE *)MyAlloc(ScreenWidth * ScreenHeight * 2)) == 0 ||
				(animation->PlayFileHandle = MyOpen((CBYTE *)file_name, MODE_READONLY)) == -1)
			{
				return(0);
			}
			if (anim_read_data((UBYTE *)&animation->FLCFileHeader, sizeof(struct FLCFileHeader )) == 0)
			{
				FileClose(animation->PlayFileHandle );
				return(0);
			}
			if (anim_read_data((UBYTE *)&animation->FLCPrefixChunk, sizeof(struct FLCPrefixChunk)) == 0)
			{
				FileClose(animation->PlayFileHandle );
				return(0);
			}
			if (animation->FLCPrefixChunk.Type == 0xF100)
			{
				if (anim_read_data((UBYTE *)animation->NextFrameBuffer, animation->FLCPrefixChunk.Size - sizeof(struct FLCPrefixChunk)) == 0)
				{
					FileClose(animation->PlayFileHandle );
					return(0);
				}
			}
			else
			{
				anim_read_data(0, -sizeof(struct FLCPrefixChunk));
			}
			animation->FrameNumber = 0;
*/
		}
		return(1);
	}
	return(0);
}

//**************************************|************************************

SLONG	anim_close(SLONG playback)
{
	if ((animation->PlaybackMode & playback) && animation->RecordFileHandle != 0)
	{
		FileSeek(animation->RecordFileHandle, SEEK_MODE_BEGINNING,0);
		animation->FLCFileHeader.NumberOfFrames--;
		FileWrite(animation->RecordFileHandle, (UBYTE *)&animation->FLCFileHeader, sizeof (struct FLCFileHeader ));
		FileClose(animation->RecordFileHandle );

		animation->RecordFileHandle = 0;
		animation->PlaybackMode = 0;
		return(1 );
	}
	if ((playback & PLAYBACK_MODE_PLAY) && animation->PlayFileHandle != 0)
	{
		FileClose(animation->PlayFileHandle );
		animation->PlayFileHandle = 0;
		animation->PlaybackMode = 0;
		return(1 );
	}
	return(0);
}

//**************************************|************************************

SLONG	anim_make_next_frame(UBYTE *screen, UBYTE *palette )
{
	struct	FLCFrameChunk		*FLCFrameChunk;
	struct	FLCFrameDataChunk	*FLCFrameDataChunk;
	SLONG						index;
	UBYTE						*NextFrameBufferPointer;
	SLONG						FLI_BRUN_size;
	SLONG						FLI_SS2_size;
	SLONG						FLI_LC_size;

	animation->NextFrameBufferPointer.UByte	=	animation->NextFrameBuffer;
	memset(animation->NextFrameBufferPointer.UByte, 0, ScreenWidth * ScreenHeight + 32 * 1024);

	animation->FLCFrameChunk.Size		=	0;
	animation->FLCFrameChunk.Type		=	0xF1FA;
	animation->FLCFrameChunk.Chunks		=	0;
	memset(&animation->FLCFrameChunk.Reserved_0, 0, sizeof(animation->FLCFrameChunk.Reserved_0 ));

	FLCFrameChunk = animation->NextFrameBufferPointer.FLCFrameChunk;
	anim_store_data((UBYTE *)&animation->FLCFrameChunk, sizeof(struct FLCFrameChunk));

	animation->FLCFrameDataChunk.Size		=	0;
	animation->FLCFrameDataChunk.Type		=	0;

	FLCFrameDataChunk = animation->NextFrameBufferPointer.FLCFrameDataChunk;
	anim_store_data((UBYTE *)&animation->FLCFrameDataChunk, sizeof(struct FLCFrameDataChunk));

	if (animation->FrameNumber == 0)
	{
		animation->FLCFileHeader.OFrame1 = animation->FLCFileHeader.Size;
	}
	else if (animation->FrameNumber == 1)
	{
		animation->FLCFileHeader.OFrame2 = animation->FLCFileHeader.Size;
	}

	if (anim_make_FLI_COLOUR256(palette))
	{
		FLCFrameChunk->Chunks++;
		FLCFrameDataChunk->Type = FLI_COLOUR256;
		FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;

		animation->FLCFrameDataChunk.Size		=	0;
		animation->FLCFrameDataChunk.Type		=	0;
		FLCFrameDataChunk = animation->NextFrameBufferPointer.FLCFrameDataChunk;
		anim_store_data((UBYTE *)&animation->FLCFrameDataChunk, sizeof(struct FLCFrameDataChunk));
	}

	if (animation->FrameNumber == 0)
	{
		if (anim_make_FLI_BRUN(screen) == 0)
		{
			if (anim_make_FLI_COPY(screen))
			{
				FLCFrameChunk->Chunks++;
				FLCFrameDataChunk->Type = FLI_COPY;
				FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;
			}
		}
		else
		{
			FLCFrameChunk->Chunks++;
			FLCFrameDataChunk->Type = FLI_BRUN;
			FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;
		}
	}
	else
	{

		NextFrameBufferPointer = animation->NextFrameBufferPointer.UByte;
		FLI_BRUN_size = anim_make_FLI_BRUN(screen);
		memset(NextFrameBufferPointer, 0, FLI_BRUN_size);
		animation->NextFrameBufferPointer.UByte = NextFrameBufferPointer;
		FLI_SS2_size = anim_make_FLI_SS2(screen, animation->LastFrame);
		memset(NextFrameBufferPointer, 0, FLI_SS2_size);
		animation->NextFrameBufferPointer.UByte = NextFrameBufferPointer;
		FLI_LC_size = anim_make_FLI_LC(screen, animation->LastFrame);

		if (FLI_LC_size < FLI_SS2_size && FLI_LC_size < FLI_BRUN_size)
		{
			FLCFrameChunk->Chunks++;
			FLCFrameDataChunk->Type = FLI_LC;
			FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;
		}
 		else if (FLI_SS2_size < FLI_BRUN_size)
		{
			memset(NextFrameBufferPointer, 0, FLI_LC_size);
			animation->NextFrameBufferPointer.UByte = NextFrameBufferPointer;
			anim_make_FLI_SS2(screen, animation->LastFrame);
			FLCFrameChunk->Chunks++;
			FLCFrameDataChunk->Type = FLI_SS2;
			FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;
		}
		else if (FLI_BRUN_size < animation->FLCFileHeader.Width * animation->FLCFileHeader.Height + sizeof(struct FLCFrameChunk))
		{
			memset(NextFrameBufferPointer, 0, FLI_LC_size);
			animation->NextFrameBufferPointer.UByte = NextFrameBufferPointer;
			anim_make_FLI_BRUN(screen );
			FLCFrameChunk->Chunks++;
			FLCFrameDataChunk->Type = FLI_BRUN;
			FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;
		}
		else
		{
			memset(NextFrameBufferPointer, 0, FLI_LC_size);
			animation->NextFrameBufferPointer.UByte = NextFrameBufferPointer;
			anim_make_FLI_COPY(screen );
			FLCFrameChunk->Chunks++;
			FLCFrameDataChunk->Type = FLI_COPY;
			FLCFrameDataChunk->Size = animation->NextFrameBufferPointer.UByte - (UBYTE *)FLCFrameDataChunk;
		}
	}

	FLCFrameChunk->Size = animation->NextFrameBufferPointer.UByte - animation->NextFrameBuffer;
	if (anim_write_data(animation->NextFrameBuffer, animation->NextFrameBufferPointer.UByte - animation->NextFrameBuffer) == 0)
	{
		return(0);
	}
	else
	{

	}

	memcpy(animation->LastFrame, screen, ScreenWidth * ScreenHeight );
	memcpy(animation->Palette, palette, 256 * 3);
	animation->FrameNumber++;
	animation->FLCFileHeader.NumberOfFrames++;
	animation->FLCFileHeader.Size += animation->NextFrameBufferPointer.UByte - animation->NextFrameBuffer;

	return(1);
}

//**************************************|************************************

SLONG	anim_make_FLI_PSTAMP()
{
	return(0);
}

//**************************************|************************************

SLONG	anim_make_FLI_COLOUR256(UBYTE *palette)
{
	UWORD	*packet_count;
	UBYTE	*colour_count;
	UBYTE	palette_skip = 0;
	SWORD	colour_changed = 0;
	SWORD	palette_step;

	if (memcmp(palette, animation->Palette, 256 * 3 ) != 0)
	{
		packet_count = animation->NextFrameBufferPointer.UWord++;

		for (palette_step = 0; palette_step < 256; palette_step++)
		{
			if (memcmp(&palette[palette_step * 3],&animation->Palette[palette_step * 3], 3) != 0)
			{
				if (colour_changed == 0)
				{
					*animation->NextFrameBufferPointer.UByte++ = palette_skip;
					palette_skip = 0;
					colour_count = animation->NextFrameBufferPointer.UByte++;
				}
				*animation->NextFrameBufferPointer.UByte++ = palette[palette_step * 3 + 0];// * 4;
				*animation->NextFrameBufferPointer.UByte++ = palette[palette_step * 3 + 1];// * 4;
				*animation->NextFrameBufferPointer.UByte++ = palette[palette_step * 3 + 2];// * 4;
				(*colour_count)++;
				colour_changed++;
			}
			else
			{
				colour_changed = 0;
				palette_skip++;
			}
			if (colour_changed == 1)
			{
				(*packet_count)++;
			}
		}

//		animation->NextFrameBufferPointer.UByte = (UBYTE *)((ULONG)((animation->NextFrameBufferPointer.UByte + 1)) & (~1));

 		return(1 );
	}
	return(0 );
}

//**************************************|************************************

SLONG	anim_make_FLI_COLOUR(UBYTE *palette)
{
	UWORD	*packet_count;
	UBYTE	*colour_count;
	UBYTE	palette_skip = 0;
	SWORD	colour_changed = 0;
	SWORD	palette_step;
	UBYTE	*NextFrameBufferPointer = animation->NextFrameBufferPointer.UByte;

	if (memcmp(palette, animation->Palette, 256 * 3 ) != 0)
	{
		packet_count = animation->NextFrameBufferPointer.UWord++;

		for (palette_step = 0; palette_step < 256; palette_step++)
		{
			if (memcmp(&palette[palette_step * 3],&animation->Palette[palette_step * 3], 3) != 0)
			{
				if (colour_changed == 0)
				{
					*animation->NextFrameBufferPointer.UByte++ = palette_skip;
					palette_skip = 0;
					colour_count = animation->NextFrameBufferPointer.UByte++;
				}
				*animation->NextFrameBufferPointer.UByte++ = palette[palette_step * 3 + 0];
				*animation->NextFrameBufferPointer.UByte++ = palette[palette_step * 3 + 1];
				*animation->NextFrameBufferPointer.UByte++ = palette[palette_step * 3 + 2];
				(*colour_count)++;
				colour_changed++;
			}
			else
			{
				colour_changed = 0;
				palette_skip++;
			}
			if (colour_changed == 1)
			{
				(*packet_count)++;
			}
		}

//		animation->NextFrameBufferPointer.UByte = (UBYTE *)((ULONG)((animation->NextFrameBufferPointer.UByte + 1)) & (~1));

 		return(1 );
	}
	return(0 );
}

//**************************************|************************************

SLONG	anim_make_FLI_SS2(UBYTE *wscreen, UBYTE *last_screen)
{
	union	MultiPointer	screen;
	union	MultiPointer	back_screen;
	union	MultiPointer	lscreen;
	union	MultiPointer	back_lscreen;
	SWORD					line_count;
	SWORD					line_skip;
	UWORD					*number_of_packets;
	SWORD					*packet_type;
	SWORD					same_count;
	SWORD					width_count;
	SWORD					screen_offset;
	UBYTE	*NextFrameBufferPointer = animation->NextFrameBufferPointer.UByte;

	number_of_packets	= animation->NextFrameBufferPointer.UWord++;

	for (line_skip = 0, line_count = animation->FLCFileHeader.Height, back_screen.UByte = wscreen, back_lscreen.UByte = last_screen; line_count; line_count--, back_screen.UByte += ScreenWidth, back_lscreen.UByte += ScreenWidth)
	{
		screen.UByte = back_screen.UByte;
		lscreen.UByte = back_lscreen.UByte;

		if (line_skip == 0)
		{
			packet_type	= animation->NextFrameBufferPointer.SWord++;
			(*number_of_packets)++;
		}

		for (width_count = animation->FLCFileHeader.Width; width_count; )
		{
			for (screen_offset = 0; width_count && *(screen.UWord + screen_offset) == *(lscreen.UWord + screen_offset); width_count -= 2, screen_offset++)
			{
			}
			if (screen_offset * 2 == (animation->FLCFileHeader.Width ) )
			{
				line_skip--;
				screen.UByte += ScreenWidth;
				lscreen.UByte += ScreenWidth;
			}
			else if (width_count > 0)
			{
				if (line_skip)
				{
					*packet_type		=	line_skip;
					line_skip			=	0;
					packet_type			=	animation->NextFrameBufferPointer.SWord++;
				}

				screen_offset *= 2;
				line_skip		=	screen_offset; //could oveflow

				if (line_skip > 255)
				{
					*animation->NextFrameBufferPointer.UByte++	=	255;
					*animation->NextFrameBufferPointer.SByte++	=	0;
					(*packet_type)++;
					line_skip -= 255;
				}

				screen.UByte	+=	screen_offset;
				lscreen.UByte	+=	screen_offset;

				for (screen_offset = 0, same_count = 0 ; width_count > 2 && same_count != -127 &&
					(	*(screen.UWord + screen_offset) != *(lscreen.UWord + screen_offset) ||
						*(screen.UWord + screen_offset + 1) != *(lscreen.UWord + screen_offset + 1)) &&
						*(screen.UWord) == *(screen.UWord + screen_offset + 1); width_count -= 2, screen_offset ++, same_count-- )
				{
				}
				if (same_count)
				{
					if (same_count != -127)
					{
						same_count--;
						width_count -= 2;
					}
					*animation->NextFrameBufferPointer.UByte++	=	line_skip;
					*animation->NextFrameBufferPointer.SByte++	=	same_count;
					*animation->NextFrameBufferPointer.UWord++	=	*screen.UWord;
					screen.UWord 								-= 	same_count;
					lscreen.UWord 								-= 	same_count;
					line_skip									=	0;
					(*packet_type)++;
				}
				else
				{
					if (width_count == 2)
					{
						same_count++;
						width_count-=2;
					}
					else
					{
						for (screen_offset = 0, same_count = 0 ; width_count && same_count != 127 &&
							*(screen.UWord + screen_offset) != *(lscreen.UWord + screen_offset) &&
							(*(screen.UWord + screen_offset ) != *(screen.UWord + screen_offset + 1) ||
							*(screen.UWord + screen_offset) != *(screen.UWord + screen_offset + 2)); width_count -= 2, screen_offset ++, same_count++ )
						{
						}
					}
					if (same_count)
					{
						*animation->NextFrameBufferPointer.SByte++	=	line_skip;
						*animation->NextFrameBufferPointer.SByte++	=	same_count;
						memcpy(animation->NextFrameBufferPointer.UByte, screen.UByte, same_count * 2);
						animation->NextFrameBufferPointer.UWord		+=	same_count;
						screen.UWord 								+= 	same_count;
						lscreen.UWord 								+= 	same_count;
						line_skip									=	0;
						(*packet_type)++;
					}
				}
			}
		}


		if (width_count == 1)
		{
			//special case

		}
	}

	if (line_skip == -animation->FLCFileHeader.Height)
	{
		*number_of_packets = 1;
		*packet_type =	1;
		*animation->NextFrameBufferPointer.UByte++	=	0;
		*animation->NextFrameBufferPointer.SByte++	=	0;

	}
	else if (line_skip)
	{
		animation->NextFrameBufferPointer.SWord--;
		(*number_of_packets)--;
	}

	animation->NextFrameBufferPointer.UByte = (UBYTE *)((ULONG)((animation->NextFrameBufferPointer.UByte + 1)) & (~1));
	return(animation->NextFrameBufferPointer.UByte - NextFrameBufferPointer );
}

//**************************************|************************************

SLONG	anim_make_FLI_LC(UBYTE *wscreen, UBYTE *last_screen)
{
	union	MultiPointer	screen;
	union	MultiPointer	back_screen;
	union	MultiPointer	lscreen;
	union	MultiPointer	back_lscreen;
	SWORD					line_count;
	SWORD					line_skip;
	UWORD					*number_of_packets;
	UWORD					number_of_lines;
	SWORD					*packet_type;
	UBYTE					*packet_count;
	SWORD					same_count;
	SWORD					width_count;
	SWORD					screen_offset;
	UWORD					*y_line_skip;
	UBYTE	*NextFrameBufferPointer = animation->NextFrameBufferPointer.UByte;

	for (line_count = animation->FLCFileHeader.Height, back_screen.UByte = wscreen, back_lscreen.UByte = last_screen; line_count; line_count--, back_screen.UByte += ScreenWidth, back_lscreen.UByte += ScreenWidth)
	{
		screen.UByte = back_screen.UByte;
		lscreen.UByte = back_lscreen.UByte;

		for (width_count = animation->FLCFileHeader.Width, screen_offset = 0; width_count && *(screen.UByte + screen_offset) == *(lscreen.UByte + screen_offset); width_count --, screen_offset++)
		{
		}
		if (screen_offset != animation->FLCFileHeader.Width )
		{
			break;
		}
	}

	*animation->NextFrameBufferPointer.UWord 	=	animation->FLCFileHeader.Height - line_count;

	if (line_count != 0)
	{
		for (line_count = animation->FLCFileHeader.Height, back_screen.UByte = &wscreen[(animation->FLCFileHeader.Height - 1) * animation->FLCFileHeader.Width], back_lscreen.UByte = &last_screen[(animation->FLCFileHeader.Height - 1) * animation->FLCFileHeader.Width]; line_count; line_count--, back_screen.UByte -= ScreenWidth, back_lscreen.UByte -= ScreenWidth)
		{
			screen.UByte = back_screen.UByte;
			lscreen.UByte = back_lscreen.UByte;

			for (width_count = animation->FLCFileHeader.Width, screen_offset = 0; width_count && *(screen.UByte + screen_offset) == *(lscreen.UByte + screen_offset); width_count --, screen_offset++)
			{
			}
			if (screen_offset != animation->FLCFileHeader.Width )
			{
				break;
			}
		}

		back_screen.UByte		=	&wscreen[*animation->NextFrameBufferPointer.UWord * animation->FLCFileHeader.Width];
		back_lscreen.UByte		=	&last_screen[*animation->NextFrameBufferPointer.UWord * animation->FLCFileHeader.Width];

		number_of_lines								=	line_count - *animation->NextFrameBufferPointer.UWord;
		animation->NextFrameBufferPointer.UWord++;
		*animation->NextFrameBufferPointer.UWord++ 	=	number_of_lines;

		for (line_count = number_of_lines; line_count; line_count--, back_screen.UByte += ScreenWidth, back_lscreen.UByte += ScreenWidth)
		{
			screen.UByte = back_screen.UByte;
			lscreen.UByte = back_lscreen.UByte;

			packet_count = animation->NextFrameBufferPointer.UByte++;

			for (width_count = animation->FLCFileHeader.Width; width_count; )
			{
				for (screen_offset = 0; width_count && *(screen.UByte + screen_offset) == *(lscreen.UByte + screen_offset); width_count --, screen_offset++)
				{
				}
				if (screen_offset == animation->FLCFileHeader.Width)
				{
				}
				else if (width_count > 0)
				{

					line_skip	=	screen_offset; //could oveflow
					if (line_skip > 255)
					{
						*animation->NextFrameBufferPointer.UByte++	=	255;
						*animation->NextFrameBufferPointer.SByte++	=	0;
						(*packet_count)++;
						line_skip -= 255;
					}

					screen.UByte	+=	screen_offset;
					lscreen.UByte	+=	screen_offset;

					for (screen_offset = 0, same_count = 0 ; width_count > 1 && same_count != -127 &&
						(	*(screen.UByte + screen_offset) != *(lscreen.UByte + screen_offset) ||
							*(screen.UByte + screen_offset + 1) != *(lscreen.UByte + screen_offset + 1) ||
							*(screen.UByte + screen_offset + 2) != *(lscreen.UByte + screen_offset + 2)) &&

							*(screen.UByte) == *(screen.UByte + screen_offset + 1); width_count --, screen_offset ++, same_count-- )
					{
					}
					if (same_count)
					{
						if (same_count != -127)
						{
	 						same_count--;
							width_count--;
						}
						*animation->NextFrameBufferPointer.UByte++	=	line_skip;
						*animation->NextFrameBufferPointer.SByte++	=	same_count;
						*animation->NextFrameBufferPointer.UByte++	=	*screen.UByte;
						screen.UByte 								-= 	same_count;
						lscreen.UByte 								-= 	same_count;
						(*packet_count)++;
					}
					else
					{
						if (width_count == 1)
						{
							same_count++;
							width_count--;
						}
						else
						{
							for (same_count = screen_offset = 0; width_count && same_count != 127 &&
								(	*(screen.UByte + screen_offset) != *(lscreen.UByte + screen_offset) ||
									*(screen.UByte + screen_offset + 1) != *(lscreen.UByte + screen_offset + 1) ||
									*(screen.UByte + screen_offset + 2) != *(lscreen.UByte + screen_offset + 2)) &&

								(	*(screen.UByte + screen_offset ) != *(screen.UByte + screen_offset + 1) ||
									*(screen.UByte + screen_offset ) != *(screen.UByte + screen_offset + 2) ||
									*(screen.UByte + screen_offset ) != *(screen.UByte + screen_offset + 3));
								width_count --, screen_offset ++, same_count++ )
							{
							}
						}
						if (same_count)
						{
							*animation->NextFrameBufferPointer.SByte++	=	line_skip;
							*animation->NextFrameBufferPointer.SByte++	=	same_count;
							memcpy(animation->NextFrameBufferPointer.UByte, screen.UByte, same_count);
							animation->NextFrameBufferPointer.UByte		+=	same_count;
							screen.UByte 								+= 	same_count;
							lscreen.UByte 								+= 	same_count;
							line_skip									=	0;
							(*packet_count)++;
						}
					}
				}
			}
		}
	}
	else //if (line_count == 0)
	{
		*animation->NextFrameBufferPointer.UWord++ = 0;
		*animation->NextFrameBufferPointer.UWord++ = 1;
		*animation->NextFrameBufferPointer.UByte++ = 0;
	}

	animation->NextFrameBufferPointer.UByte = (UBYTE *)((ULONG)((animation->NextFrameBufferPointer.UByte + 1)) & (~1));
	return(animation->NextFrameBufferPointer.UByte - NextFrameBufferPointer );
}

//**************************************|************************************

SLONG	anim_make_FLI_BLACK(UBYTE *wscreen)
{
	return(0);
}

//**************************************|************************************

SLONG	anim_make_FLI_BRUN(UBYTE *wscreen)
{
	union	MultiPointer	screen;
	union	MultiPointer	back_screen;
	UWORD					line_count;
	SWORD					same_count;
	SWORD					width_count;
	SWORD					screen_offset;
	UBYTE	*NextFrameBufferPointer = animation->NextFrameBufferPointer.UByte;

	for (line_count = animation->FLCFileHeader.Height, screen.UByte = wscreen; line_count; line_count-- )
	{
		animation->NextFrameBufferPointer.SByte++; //unused

		for (width_count = animation->FLCFileHeader.Width; width_count; )
		{
			for (same_count = 0, screen_offset = 1; width_count > 1 && *screen.UByte == *(screen.UByte + screen_offset) && same_count != 127; same_count++, width_count--, screen_offset++ )
			{
			}
			if (same_count)
			{
				if (same_count != 127)
				{
					same_count++;
					width_count--;
				}
				*animation->NextFrameBufferPointer.SByte++ = (SBYTE)same_count;
				*animation->NextFrameBufferPointer.UByte++ = *screen.UByte;
				screen.UByte += same_count;
			}
			else
			{
				if (width_count == 1)
				{
					same_count--;
					width_count--;
				}
				else
				{
					for (screen_offset = 0; width_count &&
						(	*(screen.UByte + screen_offset) != *(screen.UByte + screen_offset + 1) ||
							*(screen.UByte + screen_offset) != *(screen.UByte + screen_offset + 2) ||
							*(screen.UByte + screen_offset) != *(screen.UByte + screen_offset + 3)) &&
						same_count != -127; same_count--, width_count--, screen_offset++ )
					{
					}
				}
				if (same_count)
				{
					*animation->NextFrameBufferPointer.SByte++ = (SBYTE)same_count;
					memcpy(animation->NextFrameBufferPointer.SByte, screen.UByte, -same_count);
					screen.UByte							-= same_count;
					animation->NextFrameBufferPointer.SByte -= same_count;
				}
			}
		}
	}
	animation->NextFrameBufferPointer.UByte = (UBYTE *)((ULONG)((animation->NextFrameBufferPointer.UByte + 1)) & (~1));
	return(animation->NextFrameBufferPointer.UByte - NextFrameBufferPointer );
}

//**************************************|************************************

SLONG	anim_make_FLI_COPY(UBYTE *wscreen)
{
	memcpy(animation->NextFrameBufferPointer.UByte, wscreen, animation->FLCFileHeader.Width * animation->FLCFileHeader.Height);
	animation->NextFrameBufferPointer.UByte += animation->FLCFileHeader.Width * animation->FLCFileHeader.Height;

	return(1);
}

//**************************************|************************************

SLONG	anim_write_data(UBYTE *data, SLONG size)
{
	if (FileWrite(animation->RecordFileHandle, data, size) != size)
	{
		return(0);
	}
	return(1);
}

//**************************************|************************************

SLONG	anim_store_data(UBYTE *data, SLONG size)
{
	memcpy(animation->NextFrameBufferPointer.UByte, data, size);
	animation->NextFrameBufferPointer.UByte += size;
	return(1);
}

//**************************************|************************************

SLONG	anim_show_next_frame()
{
	struct	FLCFrameDataChunk	*FLCFrameDataChunk;
	UBYTE						last_palette[256 * 3];
	UBYTE						set_palette;
	SLONG						data_chunk_count;

	if (animation->FrameNumber < animation->FLCFileHeader.NumberOfFrames)
	{
		memcpy(last_palette, animation->Palette, 256 * 3);
		set_palette = 0;

		animation->NextFrameBufferPointer.UByte = animation->NextFrameBuffer;
		anim_read_data((UBYTE *)&animation->FLCFrameChunk, sizeof (struct FLCFrameChunk));
		anim_read_data((UBYTE *)animation->NextFrameBuffer, animation->FLCFrameChunk.Size - sizeof (struct FLCFrameChunk));

		if(animation->FLCFrameChunk.Type == 0xF1FA)
		{
			for (data_chunk_count = 0; data_chunk_count < animation->FLCFrameChunk.Chunks; data_chunk_count++)
			{
				FLCFrameDataChunk 							=	animation->NextFrameBufferPointer.FLCFrameDataChunk;
				animation->NextFrameBufferPointer.UByte	+=	sizeof(struct FLCFrameDataChunk);

				switch(FLCFrameDataChunk->Type)
				{
					case	FLI_COLOUR256:
						printf("Frame : %04d   FLI_COLOUR256   Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_COLOUR256();
						SetPalette(animation->Palette);
						break;

					case	FLI_SS2:
						printf("Frame : %04d   FLI_SS2         Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_SS2();
						break;

					case	FLI_COLOUR:
						printf("Frame : %04d   FLI_COLOUR      Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_COLOUR();
						SetPalette(animation->Palette);
						break;

					case	FLI_LC:
						printf("Frame : %04d   FLI_LC          Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_LC();
						break;

					case	FLI_BLACK:
						printf("Frame : %04d   FLI_BLACK       Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_BLACK();
						break;

					case	FLI_BRUN:
						printf("Frame : %04d   FLI_BRUN        Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_BRUN();
						break;

					case	FLI_COPY:
						printf("Frame : %04d   FLI_COPY        Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_COPY();
						break;

					case	FLI_PSTAMP:
						printf("Frame : %04d   FLI_PSTAMP      Datasize %05d\n", animation->FrameNumber, FLCFrameDataChunk->Size );
						anim_show_FLI_PSTAMP();
						break;

				}
				animation->NextFrameBufferPointer.UByte = (UBYTE *)(FLCFrameDataChunk) + FLCFrameDataChunk->Size;
			}
		}
		animation->FrameNumber++;
		return(1 );
	}
	return(0 );
}

//**************************************|************************************

SLONG	anim_show_FLI_PSTAMP()
{
	return(1);
}

//**************************************|************************************

SLONG	anim_show_FLI_COLOUR256()
{
	UBYTE	*palette = animation->Palette;
	UWORD	number_of_packets;
	UWORD	packet_count;
	UWORD	colour_count;

	for (packet_count = *animation->NextFrameBufferPointer.UWord++; packet_count; packet_count--)
	{
		palette += *animation->NextFrameBufferPointer.UByte++ * 3;
		if ((colour_count = *animation->NextFrameBufferPointer.UByte++) == 0)
		{
			colour_count = 256;
		}
		for (; colour_count; colour_count--, palette += 3)
		{
			palette[0] = (*animation->NextFrameBufferPointer.UByte++) / 4;
			palette[1] = (*animation->NextFrameBufferPointer.UByte++) / 4;
			palette[2] = (*animation->NextFrameBufferPointer.UByte++) / 4;
		}
	}
 	return(1 );
}

//**************************************|************************************

SLONG	anim_show_FLI_COLOUR()
{
	UBYTE	*palette = animation->Palette;
	UWORD	number_of_packets;
	UWORD	packet_count;
	UWORD	colour_count;

	for (packet_count = *animation->NextFrameBufferPointer.UWord++; packet_count; packet_count--)
	{
		palette += *animation->NextFrameBufferPointer.UByte++ * 3;
		if ((colour_count = *animation->NextFrameBufferPointer.UByte++) == 0)
		{
			colour_count = 256;
		}
		for (; colour_count; colour_count--, palette += 3)
		{
			palette[0] = *animation->NextFrameBufferPointer.UByte++;
			palette[1] = *animation->NextFrameBufferPointer.UByte++;
			palette[2] = *animation->NextFrameBufferPointer.UByte++;
		}
	}
 	return(1 );
}

//**************************************|************************************

SLONG	anim_show_FLI_SS2()
{
	union	MultiPointer	screen;
	union	MultiPointer	back_screen;
	UWORD	packet_count;
	SWORD	packet_type;
	UWORD	line_count;
	SBYTE	line_type;
	UWORD	word_data;

	for (line_count = *animation->NextFrameBufferPointer.UWord++, screen.UByte = back_screen.UByte = WorkScreen; line_count; line_count-- , back_screen.UByte += animation->FLCFileHeader.Width, screen.UByte = back_screen.UByte )
	{
		if ((packet_type = *animation->NextFrameBufferPointer.SWord++) & 0x8000)
		{
			if (packet_type & 0x4000)
			{
				back_screen.UByte += animation->FLCFileHeader.Width * (abs(packet_type)-1);
				line_count++;
			}
			else
			{
				screen.UByte[animation->FLCFileHeader.Width - 1] = (UBYTE)packet_type;
			}
		}
		else
		{
			for (; packet_type; packet_type-- )
			{
				screen.UByte += *animation->NextFrameBufferPointer.UByte++;

				if ((line_type = *animation->NextFrameBufferPointer.SByte++) < 0)
				{
					memset(screen.UByte, *animation->NextFrameBufferPointer.UWord++, sizeof(UWORD) * (-line_type));
					screen.UByte += sizeof(UWORD) * (-line_type);
				}
				else if (line_type > 0)
				{
					memcpy(screen.UByte, animation->NextFrameBufferPointer.UByte, sizeof(UWORD) * line_type);
					screen.UByte += sizeof(UWORD) * line_type;
					animation->NextFrameBufferPointer.UByte += sizeof(UWORD) * line_type;
				}
			}
		}
	}
	return(1 );
}

//**************************************|************************************

SLONG	anim_show_FLI_LC()
{
	union	MultiPointer	screen;
	union	MultiPointer	back_screen;
	UBYTE	packet_count;
	UWORD	line_count;
	UBYTE	line_skip;
	UBYTE	number_of_packets;
	SBYTE	packet_type;
	UBYTE	byte_data;

	screen.UByte = back_screen.UByte = WorkScreen + *animation->NextFrameBufferPointer.UWord++ * animation->FLCFileHeader.Width;

	for (line_count = *animation->NextFrameBufferPointer.UWord++; line_count; line_count--, back_screen.UByte += animation->FLCFileHeader.Width, screen.UByte = back_screen.UByte )
	{
		for(packet_count = *animation->NextFrameBufferPointer.UByte++ ;packet_count; packet_count--)
		{
			screen.UByte += *animation->NextFrameBufferPointer.UByte++;

			if((packet_type = *animation->NextFrameBufferPointer.SByte++) < 0)
			{
				packet_type = -packet_type;
				memset(screen.UByte, *animation->NextFrameBufferPointer.UByte++, packet_type);
				screen.UByte += packet_type;
			}
			else if (packet_type > 0)
			{
				memcpy(screen.UByte, animation->NextFrameBufferPointer.UByte, packet_type);
				screen.UByte += packet_type;
				animation->NextFrameBufferPointer.UByte += packet_type;
			}
		}
	}
	return(1);
}

//**************************************|************************************

SLONG	anim_show_FLI_BLACK()
{
	memset(WorkScreen, 0, animation->FLCFileHeader.Width * animation->FLCFileHeader.Height);
	return(1);
}

//**************************************|************************************

SLONG	anim_show_FLI_BRUN()
{
	union	MultiPointer	screen;
	union	MultiPointer	back_screen;
	SWORD	packet_type;
	UWORD	line_count;
	UWORD	hori_line_count;
	UBYTE	byte_data;

	for (line_count = animation->FLCFileHeader.Height, screen.UByte = back_screen.UByte = WorkScreen; line_count; line_count--, back_screen.UByte += animation->FLCFileHeader.Width, screen.UByte = back_screen.UByte)
	{
		animation->NextFrameBufferPointer.UByte++;
		for (hori_line_count = animation->FLCFileHeader.Width; hori_line_count; hori_line_count -= packet_type, screen.UByte += packet_type )
		{
			if ((packet_type = *animation->NextFrameBufferPointer.SByte++) < 0)
			{
				packet_type = -packet_type;
				memcpy(screen.UByte, animation->NextFrameBufferPointer.UByte, packet_type);
				animation->NextFrameBufferPointer.UByte += packet_type;
			}
			else if (packet_type > 0)
			{
				memset(screen.UByte, *animation->NextFrameBufferPointer.UByte++,packet_type);
			}
		}
	}
	return(1 );
}

//**************************************|************************************

SLONG	anim_show_FLI_COPY()
{
	memcpy(WorkScreen, animation->NextFrameBufferPointer.UByte, animation->FLCFileHeader.Width * animation->FLCFileHeader.Height);
	return(1);
}

//**************************************|************************************

SLONG	anim_read_data(UBYTE *data, SLONG size)
{
	if (data == 0)
	{
		FileSeek(animation->PlayFileHandle,  SEEK_MODE_CURRENT,size);
	}
	else if (FileRead(animation->PlayFileHandle, data, size) != size)
	{
		return(0);
	}
	return(1);
}

//**************************************|************************************


