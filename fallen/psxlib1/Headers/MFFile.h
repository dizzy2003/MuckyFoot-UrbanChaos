// MFFile.h
// Guy Simmons, 10th February 1997.

#ifndef _MF_FILE_H_
#define _MF_FILE_H_

#ifdef	_MF_WINDOWS
typedef	HANDLE		MFFileHandle;
#else
typedef	SLONG		MFFileHandle;
#endif

#define	FILE_OPEN_ERROR			((MFFileHandle)FileOpenError)
#define	FILE_CLOSE_ERROR		((MFFileHandle)FileCloseError)
#define	FILE_CREATION_ERROR		((MFFileHandle)FileCreationError)
#define	FILE_SIZE_ERROR			((SLONG)FileSizeError)
#define	FILE_READ_ERROR			((SLONG)FileReadError)
#define	FILE_WRITE_ERROR		((SLONG)FileWriteError)
#define	FILE_SEEK_ERROR			((SLONG)FileSeekError)
#define	FILE_LOAD_AT_ERROR		((SLONG)FileLoadAtError)

enum	SeekModes
{
	SEEK_MODE_BEGINNING,
	SEEK_MODE_CURRENT,
	SEEK_MODE_END
};

BOOL			FileExists(CBYTE *file_name);
MFFileHandle	FileOpen(CBYTE *file_name);
void			FileClose(MFFileHandle file_handle);
MFFileHandle	FileCreate(CBYTE *file_name,BOOL overwrite);
void			FileDelete(CBYTE *file_name);
SLONG			FileSize(MFFileHandle file_handle);
SLONG			FileRead(MFFileHandle file_handle,void *buffer,ULONG size);
SLONG			FileWrite(MFFileHandle file_handle,void *buffer,ULONG size);
SLONG			FileSeek(MFFileHandle file_handle,enum SeekModes mode,SLONG offset);
SLONG			FileLoadAt(CBYTE *file_name,void *buffer);


#endif
