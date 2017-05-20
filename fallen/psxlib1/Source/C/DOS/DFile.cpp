// DFile.cpp
// Guy Simmons, 10th February 1997.

#include	<MFHeader.h>


//---------------------------------------------------------------

BOOL	FileExists(CBYTE *file_name)
{
	if(_access(file_name,0)==-1)
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
		result	=	sopen	(
								file_name,
								O_RDWR|O_BINARY,
								SH_DENYNO
							);
		if(result==-1)
			result	=	FILE_OPEN_ERROR;
	}
	return	result;
}

//---------------------------------------------------------------

void	FileClose(MFFileHandle file_handle)
{
	close(file_handle);
}

//---------------------------------------------------------------

MFFileHandle	FileCreate(CBYTE *file_name,BOOL overwrite)
{
	MFFileHandle	result;


	if(overwrite==FALSE && FileExists(file_name))
	{
		return	result	=	FILE_CREATION_ERROR;
	}
	else
	{
		result	=	sopen	(
								file_name,
								O_CREAT|O_TRUNC|O_RDWR|O_BINARY,
								SH_DENYNO,
								S_IREAD|S_IWRITE
							);
		if(result==-1)
			result	=	FILE_CREATION_ERROR;
	}
	return	result;
}

//---------------------------------------------------------------

void	FileDelete(CBYTE *file_name)
{
	remove(file_name);
}

//---------------------------------------------------------------

SLONG	FileSize(MFFileHandle file_handle)
{
	SLONG		result;


	result	=	filelength(file_handle);
	if(result==-1L)
		return	FILE_SIZE_ERROR;
	else
		return	result;
}

//---------------------------------------------------------------

SLONG	FileRead(MFFileHandle file_handle,void *buffer,ULONG size)
{
	SLONG	bytes_read;


	bytes_read	=	read(file_handle,buffer,size);
	if(bytes_read<0)
		return	FILE_READ_ERROR;
	else
		return	bytes_read;
}

//---------------------------------------------------------------

SLONG	FileWrite(MFFileHandle file_handle,void *buffer,ULONG size)
{
	SLONG	bytes_written;


	bytes_written	=	write(file_handle,buffer,size);
	if(bytes_written<0)
		return	FILE_WRITE_ERROR;
	else
		return	bytes_written;
}

//---------------------------------------------------------------

SLONG	FileSeek(MFFileHandle file_handle,enum SeekModes mode,SLONG offset)
{
	int		method;


	switch(mode)
	{
		case	SEEK_MODE_BEGINNING:
			method	=	SEEK_SET;
			break;
		case	SEEK_MODE_CURRENT:
			method	=	SEEK_CUR;
			break;
		case	SEEK_MODE_END:
			method	=	SEEK_END;
			break;
	}
	if(lseek(file_handle,offset,method)==-1L)
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
