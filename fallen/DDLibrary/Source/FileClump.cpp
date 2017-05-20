// FileClump.cpp
//
// metafile class

#include "DDLib.h"
#include "FileClump.h"

// FileClump
//
// create a file clump

FileClump::FileClump(const char* clumpfn, ULONG max_id, bool readonly)
{
	// basic init
	ClumpFD = NULL;
	MaxID = max_id;
	Offsets = NULL;
	Lengths = NULL;
	NextOffset = 0;
	ReadOnly = readonly;

	if (ReadOnly)
	{
		if (ClumpFD = MF_Fopen(clumpfn, "rb"))
		{
			// read header
			fread(&MaxID, sizeof(ULONG), 1, ClumpFD);
			ASSERT(MaxID == max_id);
			Offsets = new size_t[MaxID];
			Lengths = new size_t[MaxID];
			fread(Offsets, sizeof(size_t), MaxID, ClumpFD);
			fread(Lengths, sizeof(size_t), MaxID, ClumpFD);
		}
		else
		{
			MaxID = 0;
		}
	}
	else
	{
		if (ClumpFD = MF_Fopen(clumpfn, "wb"))
		{
			// write dummy header
			fwrite(&MaxID, sizeof(ULONG), 1, ClumpFD);
			Offsets = new size_t[MaxID];
			Lengths = new size_t[MaxID];
			for (ULONG ii = 0; ii < MaxID; ii++)
			{
				Offsets[ii] = Lengths[ii] = 0;
			}
			fwrite(Offsets, sizeof(size_t), MaxID, ClumpFD);
			fwrite(Lengths, sizeof(size_t), MaxID, ClumpFD);
			NextOffset = sizeof(ULONG) + 2 * MaxID * sizeof(size_t);
		}
		else
		{
			MaxID = 0;
		}
	}
}

// ~FileClump
//
// destroy a file clump

FileClump::~FileClump()
{
	if (ClumpFD)
	{
		if (!ReadOnly)
		{
			// write the real header
			fseek(ClumpFD, 0, SEEK_SET);
			fwrite(&MaxID, sizeof(ULONG), 1, ClumpFD);
			fwrite(Offsets, sizeof(size_t), MaxID, ClumpFD);
			fwrite(Lengths, sizeof(size_t), MaxID, ClumpFD);

			TRACE("MaxID = %8.8X\n", MaxID);
			for (ULONG ii = 0; ii < MaxID; ii++)
			{
				if (Offsets[ii])	TRACE("%8.8X Off %8.8X Len %8.8X\n", ii, Offsets[ii], Lengths[ii]);
			}
		}
		MF_Fclose(ClumpFD);
	}
	delete[] Offsets;
	delete[] Lengths;
}

// Exists
//
// does a file exist in the clump file?

bool FileClump::Exists(ULONG id)
{
	if (id >= MaxID)	return false;
	if (!Offsets[id])	return false;

	return true;
}

// Read
//
// read a file from the clump

UBYTE* FileClump::Read(ULONG id)
{
	if (id >= MaxID)	return NULL;
	if (!Offsets[id])	return NULL;

	UBYTE*	buffer = new UBYTE[Lengths[id]];
	
	if (buffer)
	{
		fseek(ClumpFD, Offsets[id], SEEK_SET);
		if (fread(buffer, 1, Lengths[id], ClumpFD) != Lengths[id])
		{
			delete[] buffer;
			buffer = NULL;
		}
	}

	return buffer;
}

// Write
//
// write a file to the clump

bool FileClump::Write(void* buffer, size_t nbytes, ULONG id)
{
	if (id >= MaxID)	return false;
	if (ReadOnly)		return false;
	if (Offsets[id])	return false;

	fseek(ClumpFD, NextOffset, SEEK_SET);
	if (fwrite(buffer, 1, nbytes, ClumpFD) != nbytes)	return false;

	Offsets[id] = NextOffset;
	Lengths[id] = nbytes;
	NextOffset += nbytes;

	return true;
}
