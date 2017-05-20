// AsyncFile2
//
// asynchronous file loading
//
// the API opens files and moves blocks from the Free to the Active list; it also moves blocks from
// the Complete to the Free list.  a critical section is used to serialize access to the Active and
// Complete lists (only the API accesses the Free list) and an event is used to wake the worker thread
// when required.  To avoid using the MT C RTL the worker thread only makes WIN32 calls.
//
// If you don't understand what this means, DON'T FUCK WITH THIS CODE

#include <ddlib.h>
//#include <windows.h>
#ifndef TARGET_DC
#include <process.h>
#endif

#ifdef TARGET_DC
#include "target.h"
#endif

#include "asyncfile2.h"

static AsyncFile	File[MAX_ASYNC_FILES];

static AsyncFile	FreeList;
static AsyncFile	ActiveList;
static AsyncFile	CompleteList;

static void Unlink(AsyncFile* file);
static void FreeLink(AsyncFile* file);
static void ActiveLink(AsyncFile* file);
static void CompleteLink(AsyncFile* file);

// thread stuff

static int			KillThread;
static void*		CancelKey;

static CRITICAL_SECTION	csLock;
static HANDLE			hEvent;
static HANDLE			hThread;
static DWORD WINAPI		ThreadRun(LPVOID arg);

// amount to read per ms
static const int	BytesPerMillisecond = 128 * 1024;

// InitAsyncFile
//
// initialize

void InitAsyncFile(void)
{
	// set up structures
	memset(File, 0, sizeof(File));
	memset(&FreeList, 0, sizeof(FreeList));
	memset(&ActiveList, 0, sizeof(ActiveList));
	memset(&CompleteList, 0, sizeof(CompleteList));

	FreeList.next = FreeList.prev = &FreeList;
	ActiveList.next = ActiveList.prev = &ActiveList;
	CompleteList.next = CompleteList.prev = &CompleteList;

	for (int ii = 0; ii < MAX_ASYNC_FILES; ii++)
	{
		FreeLink(&File[ii]);
	}

	// init sync objects
	InitializeCriticalSection(&csLock);

	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	KillThread = 0;
	CancelKey = 0;

	// begin background thread
	DWORD	tid;

	hThread = CreateThread(NULL, 0, ThreadRun, NULL, 0, &tid);
}

// TermAsyncFile
//
// terminate

void TermAsyncFile(void)
{
	// tell thread to die
	TRACE("MAIN: Telling worker thread to die\n");
	EnterCriticalSection(&csLock);
	KillThread = 1;
	LeaveCriticalSection(&csLock);

	SetEvent(hEvent);
	
	// wait for thread to die
	TRACE("MAIN: Waiting for worker thread to die\n");
	EnterCriticalSection(&csLock);
	while (KillThread != 2)
	{
		LeaveCriticalSection(&csLock);
		Sleep(0);
		EnterCriticalSection(&csLock);
	}
	LeaveCriticalSection(&csLock);

	TRACE("MAIN: Closing down\n");
	CloseHandle(hThread);
	CloseHandle(hEvent);
}

// ThreadRun
//
// the main thread

DWORD WINAPI ThreadRun(LPVOID arg)
{
	for (;;)
	{
		// wait to be signalled
		WaitForSingleObject(hEvent, INFINITE);

		TRACE("Worker: Signalled\n");

		// get command
		EnterCriticalSection(&csLock);

		if (KillThread == 1)
		{
			TRACE("Worker: Received kill instruction\n");
			KillThread = 2;
			LeaveCriticalSection(&csLock);
			return 0;
		}

		if (CancelKey)
		{
			TRACE("Worker: Received cancel instruction\n");
			AsyncFile*	file = ActiveList.next;
			while (file != &ActiveList)
			{
				AsyncFile*	next = file->next;
				if (file->hKey == CancelKey)
				{
					Unlink(file);
					CompleteLink(file);
				}
				file = next;
			}
			CancelKey = NULL;
		}

		AsyncFile*	file = NULL;

		if (ActiveList.next != &ActiveList)
		{
			TRACE("Worker: Received file load instruction\n");
			file = ActiveList.next;
			Unlink(file);
		}

		LeaveCriticalSection(&csLock);

		if (file)
		{
			DWORD	amount;

			// read file
			TRACE("Worker: Commence reading file\n");

			while (file->blen > BytesPerMillisecond)
			{
				TRACE("Worker: Reading block\n");
				ReadFile(file->hFile, file->buffer, BytesPerMillisecond, &amount, NULL);
				file->buffer += BytesPerMillisecond;
				file->blen -= BytesPerMillisecond;
				Sleep(0);
			}

			if (file->blen)
			{
				TRACE("Worker: Reading final block\n");
				ReadFile(file->hFile, file->buffer, file->blen, &amount, NULL);
			}
			TRACE("Worker: Complete\n");

			EnterCriticalSection(&csLock);
			CompleteLink(file);
			LeaveCriticalSection(&csLock);
		}
	}

	return 0;
}

// LoadAsyncFile
//
// load an async file
// returns false on error

bool LoadAsyncFile(char* filename, void* buffer, DWORD blen, void* key)
{
	// get free AsyncFile
	if (FreeList.next == &FreeList)	return false;

	AsyncFile*	file = FreeList.next;

	// open the file
	file->hFile = CreateFile (filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (!file->hFile)	return false;

	// set the key
	file->hKey = key;

	// set up the read
	file->buffer = (UBYTE*)buffer;
	file->blen = blen;

	// success
	Unlink(file);

	// send to worker thread & signal it
	TRACE("MAIN: Adding block to active list\n");
	EnterCriticalSection(&csLock);
	ActiveLink(file);
	LeaveCriticalSection(&csLock);

	TRACE("MAIN: Signalling worker thread\n");
	SetEvent(hEvent);

	return true;
}

// GetNextCompletedAsyncFile
//
// return key for completed file

void* GetNextCompletedAsyncFile(void)
{
	EnterCriticalSection(&csLock);
	for (AsyncFile* file = CompleteList.next; file != &CompleteList; file = file->next)
	{
		Unlink(file);
		LeaveCriticalSection(&csLock);

		CloseHandle(file->hFile);
		FreeLink(file);

		return file->hKey;
	}
	LeaveCriticalSection(&csLock);

	return NULL;
}

// CancelAsyncFile
//
// cancel loading an async file

void CancelAsyncFile(void* key)
{
	if (!key)	return;

	// signal worker thread to cancel the file
	TRACE("MAIN: Instructing worker thread to cancel\n");
	EnterCriticalSection(&csLock);
	CancelKey = key;
	LeaveCriticalSection(&csLock);

	SetEvent(hEvent);

	// wait for thread
	EnterCriticalSection(&csLock);
	while (CancelKey)
	{
		LeaveCriticalSection(&csLock);
		Sleep(0);
		EnterCriticalSection(&csLock);
	}
	LeaveCriticalSection(&csLock);

	TRACE("MAIN: Continuing after cancel\n");
}

// List utilities

static void Unlink(AsyncFile* file)
{
	file->next->prev = file->prev;
	file->prev->next = file->next;
}

static void FreeLink(AsyncFile* file)
{
	file->prev = &FreeList;
	file->next = FreeList.next;

	file->next->prev = file;
	file->prev->next = file;
}

static void ActiveLink(AsyncFile* file)
{
	file->prev = &ActiveList;
	file->next = ActiveList.next;

	file->next->prev = file;
	file->prev->next = file;
}

static void CompleteLink(AsyncFile* file)
{
	file->prev = &CompleteList;
	file->next = CompleteList.next;

	file->next->prev = file;
	file->prev->next = file;
}
