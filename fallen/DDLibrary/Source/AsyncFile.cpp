// AsyncFile
//
// asynchronous file loading
//
// ha ha the funny thing about this code is that it will only work under Windows NT ... oops!

#include <windows.h>
#include "asyncfile.h"

static AsyncFile	File[MAX_ASYNC_FILES];

static AsyncFile	FreeList;
static AsyncFile	ActiveList;

static void Unlink(AsyncFile* file);
static void FreeLink(AsyncFile* file);
static void ActiveLink(AsyncFile* file);

// InitAsyncFile
//
// initialize

void InitAsyncFile(void)
{
	memset(File, 0, sizeof(File));
	memset(&FreeList, 0, sizeof(FreeList));
	memset(&ActiveList, 0, sizeof(ActiveList));

	FreeList.next = FreeList.prev = &FreeList;
	ActiveList.next = ActiveList.prev = &ActiveList;

	for (int ii = 0; ii < MAX_ASYNC_FILES; ii++)
	{
		FreeLink(&File[ii]);
	}
}

// TermAsyncFile
//
// terminate

void TermAsyncFile(void)
{
	CancelAsyncFile(NULL);
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
	file->hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (!file->hFile)	return false;

	// set the key
	file->hKey = key;

	// begin the read operation
	file->Control.Offset = 0;
	file->Control.OffsetHigh = 0;
	file->Control.hEvent = 0;

	if (!ReadFileEx(file->hFile, buffer, blen, &file->Control, NULL))
	{
		// failure
		CloseHandle(file->hFile);
		return false;
	}

	// success
	Unlink(file);
	ActiveLink(file);
	return true;
}

// GetNextCompletedAsyncFile
//
// return key for completed file

void* GetNextCompletedAsyncFile(void)
{
	for (AsyncFile* file = ActiveList.next; file != &ActiveList; file = file->next)
	{
		if (HasOverlappedIoCompleted(&file->Control))
		{
			// this has finished
			Unlink(file);
			FreeLink(file);

			CloseHandle(file->hFile);

			return file->hKey;
		}
	}

	return NULL;
}

// CancelAsyncFile
//
// cancel loading an async file

void CancelAsyncFile(void* key)
{
	AsyncFile*	file = ActiveList.next;

	while (file != &ActiveList)
	{
		AsyncFile*	next = file->next;

		if (!key || (file->hKey == key))
		{
			CancelIo(file->hFile);
			CloseHandle(file->hFile);
			Unlink(file);
			FreeLink(file);
		}

		file = next;
	}
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
