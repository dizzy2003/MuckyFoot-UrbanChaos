// FileClump.h
//
// File clump class - contains multiple files

#ifndef FILECLUMP_H
#define FILECLUMP_H

// FileClump
//
// a bunch of files stored in a metafile

class FileClump
{
public:
	FileClump(const char* clumpfn, ULONG max_id, bool readonly);
	~FileClump();

	bool	Exists(ULONG id);								// sees if a file exists

	UBYTE*	Read(ULONG id);									// read a whole file
	bool	Write(void* buffer, size_t nbytes, ULONG id);	// write a whole file

private:
	FILE*			ClumpFD;	// FILE* for the clump, may be NULL if the open failed
	ULONG			MaxID;		// maximum ID
	size_t*			Offsets;	// MaxID offsets
	size_t*			Lengths;	// MaxID lengths
	size_t			NextOffset;	// next offset for writing
	bool			ReadOnly;	// read-only flag
};

#endif	// FILECLUMP_H
