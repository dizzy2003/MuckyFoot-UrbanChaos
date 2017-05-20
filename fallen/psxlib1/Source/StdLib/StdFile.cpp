// StdFile.cpp
// Guy Simmons, 18th December 1997.

#include	<MFStdLib.h>


//---------------------------------------------------------------

BOOL	FileExists(CBYTE *file_name)
{
	if(GetFileAttributes(file_name)==0xffffffff)
		return	FALSE;
	else
		return	TRUE;
}

//---------------------------------------------------------------

MFFileHandle	FileOpen(CBYTE *file_name)
{
	MFFileHandle	result	=	FILE_OPEN_ERROR;


	if(FileExists(file_name))
	{
    	result	=	CreateFile	(
									file_name,
									(GENERIC_READ|GENERIC_WRITE),
									(FILE_SHARE_READ|FILE_SHARE_WRITE),
									NULL,
									OPEN_EXISTING,
									0,
									NULL
    	                   		);
		if(result==INVALID_HANDLE_VALUE)
			result	=	FILE_OPEN_ERROR;
	}
	return	result;
}

//---------------------------------------------------------------

void	FileClose(MFFileHandle file_handle)
{
	CloseHandle(file_handle);
}

//---------------------------------------------------------------

MFFileHandle	FileCreate(CBYTE *file_name,BOOL overwrite)
{
	DWORD			creation_mode;
	MFFileHandle	result;


	if(overwrite)
	{
		creation_mode	=	CREATE_ALWAYS;
	}
	else
	{
		creation_mode	=	CREATE_NEW;
	}
	result	=	CreateFile	(
								file_name,
								(GENERIC_READ|GENERIC_WRITE),
								(FILE_SHARE_READ|FILE_SHARE_WRITE),
								NULL,
								creation_mode,
								0,
								NULL
	                   		);
	if(result==INVALID_HANDLE_VALUE)
		result	=	FILE_CREATION_ERROR;

	return	result;
}

//---------------------------------------------------------------

void	FileDelete(CBYTE *file_name)
{
	DeleteFile(file_name);
}

//---------------------------------------------------------------

SLONG	FileSize(MFFileHandle file_handle)
{
	DWORD	result;


	result	=	GetFileSize(file_handle,NULL);
	if(result==0xffffffff)
		return	FILE_SIZE_ERROR;
	else
		return	(SLONG)result;
}

//---------------------------------------------------------------

SLONG	FileRead(MFFileHandle file_handle,void *buffer,ULONG size)
{
	SLONG	bytes_read;


	if(ReadFile(file_handle,buffer,size,(LPDWORD)&bytes_read,NULL)==FALSE)
		return	FILE_READ_ERROR;
	else
		return	bytes_read;
}

//---------------------------------------------------------------

SLONG	FileWrite(MFFileHandle file_handle,void *buffer,ULONG size)
{
	SLONG	bytes_written;


	if(WriteFile(file_handle,buffer,size,(LPDWORD)&bytes_written,NULL)==FALSE)
		return	FILE_WRITE_ERROR;
	else
		return	bytes_written;
}

//---------------------------------------------------------------

SLONG	FileSeek(MFFileHandle file_handle,const int mode,SLONG offset)
{
	DWORD		method;


	switch(mode)
	{
		case	SEEK_MODE_BEGINNING:
			method	=	FILE_BEGIN;
			break;
		case	SEEK_MODE_CURRENT:
			method	=	FILE_CURRENT;
			break;
		case	SEEK_MODE_END:
			method	=	FILE_END;
			break;
	}
	if(SetFilePointer(file_handle,offset,NULL,method)==0xffffffff)
		return	FILE_SEEK_ERROR;
	else
		return	0;
}

//---------------------------------------------------------------

SLONG	FileLoadAt(CBYTE *file_name,void *buffer)
{
	SLONG			size;
	MFFileHandle	handle;

	
	handle	=	FileOpen(file_name);
	if(handle!=FILE_OPEN_ERROR)
	{
		size	=	FileSize(handle);
		if(size>0)
		{
			if(FileRead(handle,buffer,size)==size)
			{
				FileClose(handle);
				return	size;
			}
		}
		FileClose(handle);
	}
	return	FILE_LOAD_AT_ERROR;
}

//---------------------------------------------------------------


void TraceText(CBYTE *fmt, ...)
{
	//
	// Work out the real message.
	//

	CBYTE   message[512];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(message, fmt, ap);
	va_end  (ap);

	OutputDebugString(message);
}
