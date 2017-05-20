// MFHost.cpp	-	Windows.
// Guy Simmons, 1st February 1997.

#include	<MFHeader.h>


//---------------------------------------------------------------

MFFileHandle		log_handle	=	NULL;

BOOL	SetupHost(ULONG flags)
{
	if(!SetupMemory())
		return	FALSE;
	if(!SetupKeyboard())
		return	FALSE;
	
	if(flags&H_CREATE_LOG)
	{
		log_handle	=	FileCreate("debug.log",1);
		if(log_handle==FILE_CREATION_ERROR)
			log_handle	=	NULL;			
	}

	return	TRUE;
}

//---------------------------------------------------------------

void	ResetHost(void)
{
	if(log_handle)
		FileClose(log_handle);
	ResetKeyboard();
	ResetMemory();
}

//---------------------------------------------------------------

void	LogText(CBYTE *error, ...)
{
	CBYTE 			buf[512];
	va_list 		argptr;

	if(log_handle)
	{
		va_start(argptr,error); 
		vsprintf(buf, error,argptr); 
		va_end(argptr);
		
		FileWrite(log_handle,buf,strlen(buf));
	}
}

//---------------------------------------------------------------

int MFMessage(const char *pMessage, const char *pFile, ULONG dwLine)
{
	LogText("Mucky Foot Message\n    %s\nIn   : %s\n%sLine : %u",pMessage,pFile,dwLine);

	printf("Something strange has happened!\n");
	printf("\n%s\n\nIn   : %s\nline : %u\n",pMessage, pFile, dwLine);

	printf("\n\nA - Kill Application\n I - Continue\n?");

	printf("\nAny key to continue.\n");
	while(!LastKey);

	return	FALSE;
}

//---------------------------------------------------------------

void	Time(struct MFTime *the_time)
{
/*
	SYSTEMTIME	new_time;


	GetLocalTime(&new_time);
*/
	the_time->Hours		=	0;
	the_time->Minutes	=	0;
	the_time->Seconds	=	0;
	the_time->MSeconds	=	0;
	the_time->DayOfWeek	=	0;
	the_time->Day		=	0;
	the_time->Month		=	0;
	the_time->Year		=	0;
	the_time->Ticks		=	clock();
}

//---------------------------------------------------------------
