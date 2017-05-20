// StdFile.h
// Guy Simmons, 18th December 1997.

#ifndef	STD_FILE_H
#define	STD_FILE_H

//---------------------------------------------------------------

typedef	HANDLE		MFFileHandle;

#define	FILE_OPEN_ERROR			((MFFileHandle)-100)
#define	FILE_CLOSE_ERROR		((MFFileHandle)-101)
#define	FILE_CREATION_ERROR		((MFFileHandle)-102)
#define	FILE_SIZE_ERROR			((SLONG)-103)
#define	FILE_READ_ERROR			((SLONG)-104)
#define	FILE_WRITE_ERROR		((SLONG)-105)
#define	FILE_SEEK_ERROR			((SLONG)-106)
#define	FILE_LOAD_AT_ERROR		((SLONG)-107)

#define	SEEK_MODE_BEGINNING		0
#define	SEEK_MODE_CURRENT		1
#define	SEEK_MODE_END			2

BOOL			FileExists(CBYTE *file_name);
MFFileHandle	FileOpen(CBYTE *file_name);
void			FileClose(MFFileHandle file_handle);
MFFileHandle	FileCreate(CBYTE *file_name,BOOL overwrite);
void			FileDelete(CBYTE *file_name);
SLONG			FileSize(MFFileHandle file_handle);
SLONG			FileRead(MFFileHandle file_handle,void *buffer,ULONG size);
SLONG			FileWrite(MFFileHandle file_handle,void *buffer,ULONG size);
SLONG			FileSeek(MFFileHandle file_handle,const int mode,SLONG offset);
SLONG			FileLoadAt(CBYTE *file_name,void *buffer);
void			FileSetBasePath(CBYTE *path_name);



// Plug'n'play replacements for fopen/fclose.
// Except they do something cunning on the DC.
// These can be used with sscanf() and so on,
// whereas the above can't.
FILE *MF_Fopen ( const char *file_name, const char *mode );
int MF_Fclose( FILE *stream );


//---------------------------------------------------------------

#endif
