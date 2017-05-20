/*
** MCARD.cpp
**
** Memory card handler for PSX Save games
*/

#include <MFStdLib.h>

#include <libmcrd.h>

#include "psxeng.h"
#include "mcard.h"

SLONG MCARD_status;

DIRENTRY MCARD_file;

void MCARD_Init()
{
	MCARD_status=MCARD_NOT_SCANNED;
//	MemCardStart();
}

void MCARD_Final()
{
//	MemCardStop();
}

SLONG MCARD_Scan()
{
	MCARD_status=MemCardAccept(0x00);
	if (MCARD_status==0)
		return MCARD_status=MCARD_NOT_AVAILABLE;

	return MCARD_status=MCARD_SCANNING;
}

SLONG MCARD_Status()
{
	SLONG cmd,result;

	switch(MemCardSync(1,&cmd,&result))
	{
	case 0:
		return MCARD_status;
		break;
	case -1:
	case 1:
		switch(MCARD_status)
		{
		case	MCARD_NOT_SCANNED:
		case	MCARD_NEW_CARD:
			if (MemCardAccept(0x00)==0)
				return MCARD_status=MCARD_NOT_AVAILABLE;
			else
				return MCARD_status=MCARD_SCANNING;
			break;
		case	MCARD_SCANNING:
			return MCARD_status=result;
		case	MCARD_READING:
			return MCARD_status=result+MCARD_READ_OFFSET;
		case	MCARD_WRITING:
			return MCARD_status=result+MCARD_WRITE_OFFSET;
			break;
		case	MCARD_NOT_AVAILABLE:
			return MCARD_status;
		default:
			return MCARD_status;
		}
	}
	return 0;
}

SLONG MCARD_Format()
{
	MCARD_status=MCARD_Scan();
	while(MCARD_status==MCARD_SCANNING)
		MCARD_status=MCARD_Status();

	if (MCARD_status==MCARD_NOT_FORMATTED)
		MCARD_status=MemCardFormat(0x00)+MCARD_FORMAT_OFFSET;
	else
		MCARD_status=MCARD_FORMAT_NEW_CARD;
	return MCARD_status;
}

SLONG MCARD_ReadFile(char *fname,UBYTE *addr,SLONG len)
{					  
	MCARD_status=MCARD_Scan();
	while(MCARD_status==MCARD_SCANNING)
		MCARD_status=MCARD_Status();
	if (MCARD_status==MCARD_OKAY)
	{
		if (MemCardReadFile(0x00,fname,(ULONG*)addr,0,len)+MCARD_READ_OFFSET)
			return MCARD_status=MCARD_READING;
	}
	else
		return MCARD_status=MCARD_READ_NEW_CARD;

	return MCARD_status=MCARD_READ_ERROR;
}

SLONG MCARD_WriteFile(char *fname,UBYTE *addr,SLONG len)
{
	if (MemCardWriteFile(0x00,fname,(ULONG*)addr,0,len)+MCARD_WRITE_OFFSET)
		return MCARD_status=MCARD_WRITING;

	return MCARD_status=MCARD_WRITE_ERROR;
}

SLONG MCARD_FindFile(char *fname)
{
	SLONG files;

	MCARD_status=MemCardGetDirentry(0x00,fname,&MCARD_file,&files,0,1)+MCARD_FIND_OFFSET;

	if (MCARD_status!=MCARD_FIND_OKAY)
		return MCARD_status;

	if (files==0)
		return MCARD_status=MCARD_FIND_NOT_FOUND;
	else
		return MCARD_status=MCARD_FIND_OKAY;
}

SLONG MCARD_CreateFile(char *fname,SLONG blocks)
{
	MCARD_status=MCARD_Scan();
	while (MCARD_status==MCARD_SCANNING)
		MCARD_status=MCARD_Status();

	if (MCARD_status==MCARD_OKAY)
		MCARD_status=MemCardCreateFile(0x00,fname,blocks)+MCARD_NEW_OFFSET;
	else
		MCARD_status=MCARD_WRITE_NEW_CARD;

	return MCARD_status;
}

struct DIRENTRY MCARD_dir[15];

SLONG MCARD_FindSpace(SLONG blocks)
{
	SLONG files,size;
	SLONG i;

	MCARD_status=MemCardGetDirentry(0x00,"*",MCARD_dir,&files,0,15);

	size=0;

	for(i=0;i<files;i++)
		size+=(MCARD_dir[i].size+8191)>>13;

	if (size+blocks>=15)
		return MCARD_status=MCARD_SPACE_FULL;

	return MCARD_status=MCARD_SPACE_OKAY;
}