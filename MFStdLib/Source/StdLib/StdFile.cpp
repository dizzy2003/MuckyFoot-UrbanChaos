// StdFile.cpp
// Guy Simmons, 18th December 1997.

#include	<MFStdLib.h>

#define MAX_LENGTH_OF_BASE_PATH 64
CBYTE cBasePath[MAX_LENGTH_OF_BASE_PATH+1];

#ifdef DEBUG
int m_iNumOpenFiles_FileOpen = 0;
int m_iNumOpenFiles_MF_Fopen = 0;
char pcPrevFilenameOpened[256];
#endif

//---------------------------------------------------------------

#define MAX_LENGTH_OF_FULL_NAME (MAX_LENGTH_OF_BASE_PATH+128)
CBYTE cTempFilename[MAX_LENGTH_OF_FULL_NAME+1];

CBYTE *MakeFullPathName ( const CBYTE *cFilename )
{
	strcpy ( cTempFilename, cBasePath );
	ASSERT ( strlen ( cFilename ) < ( MAX_LENGTH_OF_FULL_NAME - MAX_LENGTH_OF_BASE_PATH ) );
	strcat ( cTempFilename, cFilename );
	return ( cTempFilename );
}

//---------------------------------------------------------------

BOOL	FileExists( CBYTE *file_name)
{
	file_name = MakeFullPathName ( file_name );

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
		file_name = MakeFullPathName ( file_name );

    	result	=	CreateFile	(
									file_name,
									(GENERIC_READ),
									(FILE_SHARE_READ),
									NULL,
									OPEN_EXISTING,
									0,
									NULL
    	                   		);
		if(result==INVALID_HANDLE_VALUE)
		{
			result	=	FILE_OPEN_ERROR;
		}
		else
		{
#ifdef DEBUG
			m_iNumOpenFiles_FileOpen++;
			if ( m_iNumOpenFiles_FileOpen > 1 )
			{
				TRACE ( "FileOpen nested %i\n", m_iNumOpenFiles_FileOpen );
			}
			strncpy ( pcPrevFilenameOpened, file_name, 256 );
#endif
		}
	}
	return	result;
}

//---------------------------------------------------------------

void	FileClose(MFFileHandle file_handle)
{
#ifdef DEBUG
	if ( m_iNumOpenFiles_FileOpen > 1 )
	{
		TRACE ( "FileClose nested %i\n", m_iNumOpenFiles_FileOpen );
	}
	m_iNumOpenFiles_FileOpen--;
#endif
	CloseHandle(file_handle);
}

//---------------------------------------------------------------

MFFileHandle	FileCreate(CBYTE *file_name,BOOL overwrite)
{
	DWORD			creation_mode;
	MFFileHandle	result;

	file_name = MakeFullPathName ( file_name );

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
								0,//(FILE_SHARE_READ|FILE_SHARE_WRITE),
								NULL,
								creation_mode,
								FILE_ATTRIBUTE_NORMAL,
								NULL
	                   		);
	if(result==INVALID_HANDLE_VALUE)
		result	=	FILE_CREATION_ERROR;

	return	result;
}

//---------------------------------------------------------------

void	FileDelete(CBYTE *file_name)
{
	file_name = MakeFullPathName ( file_name );
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

//---------------------------------------------------------------

void			FileSetBasePath(CBYTE *path_name)
{
	ASSERT ( strlen ( path_name ) < MAX_LENGTH_OF_BASE_PATH - 1 );
	strncpy ( cBasePath, path_name, MAX_LENGTH_OF_BASE_PATH - 1 );
	// Add a trailing slash if need be.
	CBYTE *pch = cBasePath;
	if ( *pch != '\0' )
	{
		while ( *++pch != '\0' ){}
		pch--;
		if ( *pch != '\\' )
		{
			*pch++ = '\\';
			*pch = '\0';
		}
	}
}

//---------------------------------------------------------------

// Do NOT mix and match MF_FOpen/MF_FClose with the above - they don't mingle.
#ifdef TARGET_DC
static TCHAR pchTcharVersion[100];
//static TCHAR pchTcharMode[5];
#endif
FILE *MF_Fopen ( const char *file_name, const char *mode )
{
	if ( !FileExists ( (char *)file_name ) )
	{
		return NULL;
	}
	file_name = MakeFullPathName ( file_name );
#ifdef TARGET_DC

	// Apparently fopen causes a memory leak.

#if 1
	ASSERT ( mode[0] == 'r' );
	ASSERT ( mode[2] == '\0' );
	TCHAR *pchTcharMode;
	if ( mode[1] == 't' )
	{
		pchTcharMode = TEXT("rt");
	}
	else
	{
		ASSERT ( mode[1] == 'b' );
		pchTcharMode = TEXT("rb");
	}
#else
	pc1 = mode;
	ASSERT ( strlen ( mode) < 3 );
	pc2 = pchTcharMode;
	while ( *pc1 != '\0' )
	{
		*pc2++ = (TCHAR)( *pc1++ );
	}
	*pc2++ = TEXT('\0');
#endif

	const char *pc1 = file_name;
	ASSERT ( strlen ( file_name) < 80 );
	TCHAR *pc2 = pchTcharVersion;
	while ( *pc1 != '\0' )
	{
		*pc2++ = (TCHAR)( *pc1++ );
	}
	*pc2++ = TEXT('\0');

	FILE *res = _wfopen ( pchTcharVersion, pchTcharMode );
#ifdef DEBUG
	if ( res != NULL )
	{
		m_iNumOpenFiles_MF_Fopen++;
		if ( m_iNumOpenFiles_MF_Fopen > 1 )
		{
			TRACE ( "MF_Fopen nested %i\n", m_iNumOpenFiles_MF_Fopen );
		}
		strncpy ( pcPrevFilenameOpened, file_name, 256 );
	}
#endif
	return ( res );
#else
	return ( fopen ( (char *)file_name, (char *)mode ) );
#endif
}

//---------------------------------------------------------------

int MF_Fclose( FILE *stream )
{
#ifdef DEBUG
	if ( m_iNumOpenFiles_MF_Fopen > 1 )
	{
		TRACE ( "MF_Fclose nested %i\n", m_iNumOpenFiles_MF_Fopen );
	}
	m_iNumOpenFiles_MF_Fopen--;
#endif
	return ( fclose ( stream ) );
}

//---------------------------------------------------------------



