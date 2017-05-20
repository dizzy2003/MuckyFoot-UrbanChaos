// vertexbuffer.cpp
//
// Vertex buffer stuff (DX6)

#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "console.h"
#include "poly.h"
#ifdef	TARGET_DC
#include "target.h"
#endif

#include "vertexbuffer.h"

VertexBufferPool*	TheVPool = NULL;

void VB_Init()
{
	if (!TheVPool)		TheVPool = MFnew<VertexBufferPool>();
}

void VB_Term()
{
	MFdelete(TheVPool);
	TheVPool = NULL;
}

//-----------------------------
// VertexBuffer
//-----------------------------

// constructor / destructor

VertexBuffer::VertexBuffer()
{
	m_LogSize = 0;
	m_LockedPtr = NULL;
	m_Prev = NULL;
	m_Next = NULL;
#if USE_D3D_VBUF
	m_TheBuffer = NULL;
#endif
}

VertexBuffer::~VertexBuffer()
{
	ASSERT(!m_Next);
	ASSERT(!m_Prev);

	if (m_LockedPtr)
	{
#if !USE_D3D_VBUF
		//delete[] m_LockedPtr;
		MemFree ( m_LockedPtr );
 #endif
		Unlock();
	}

#if USE_D3D_VBUF
	if (m_TheBuffer)
	{
		m_TheBuffer->Release();
		m_TheBuffer = NULL;
	}
#endif
}

// Create
//
// create the vertex buffer

HRESULT VertexBuffer::Create(IDirect3D3* d3d, bool force_system, ULONG logsize)
{
	ASSERT(!m_LockedPtr);
	ASSERT(d3d);

#if USE_D3D_VBUF

	ASSERT(!m_TheBuffer);

	TRACE("Create vertex buffer\n");

	D3DVERTEXBUFFERDESC		desc;

	InitStruct(desc);

	desc.dwCaps = force_system ? D3DVBCAPS_SYSTEMMEMORY : 0;
//	desc.dwCaps |= D3DVBCAPS_WRITEONLY;	// oops ... can't expand with this flag set ;-)
	desc.dwFVF	= D3DFVF_TLVERTEX;
	desc.dwNumVertices = 1 << logsize;

	HRESULT	res = d3d->CreateVertexBuffer(&desc, &m_TheBuffer, 0, NULL);

	if (FAILED(res))
	{
		m_TheBuffer = NULL;
		return res;
	}

#else

	//m_LockedPtr = new D3DTLVERTEX[1 << logsize];
	m_LockedPtr = (D3DTLVERTEX*)MemAlloc ( (1 << logsize) * sizeof (D3DTLVERTEX ) );
	if (!m_LockedPtr)	return DDERR_OUTOFMEMORY;

#endif

	m_LogSize = logsize;

	return DD_OK;
}

// Lock
//
// lock the vertex buffer

D3DTLVERTEX* VertexBuffer::Lock()
{
#if USE_D3D_VBUF

	ASSERT(m_TheBuffer);
	ASSERT(!m_LockedPtr);

	HRESULT	res = m_TheBuffer->Lock(DDLOCK_SURFACEMEMORYPTR | DDLOCK_NOSYSLOCK, (LPVOID*)&m_LockedPtr, NULL);

	if (FAILED(res))
	{
		ASSERT(res != DDERR_SURFACELOST);
		m_LockedPtr = NULL;
		return NULL;
	}

#endif

	return m_LockedPtr;
}

// Unlock
//
// unlock the vertex buffer

void VertexBuffer::Unlock()
{
#if USE_D3D_VBUF

	ASSERT(m_TheBuffer);
	ASSERT(m_LockedPtr);

	HRESULT res = m_TheBuffer->Unlock();

	if (FAILED(res))
	{
		ASSERT(res != DDERR_SURFACELOST);
		ASSERT(0);
	}

	m_LockedPtr = NULL;

#endif
}

//-----------------------------
// VertexBufferPool
//-----------------------------

// constructor & destructor

VertexBufferPool::VertexBufferPool()
{
	m_D3D = NULL;
	m_SysMem = false;

	for (int ii = 0; ii < 16; ii++)
	{
		m_FreeList[ii] = NULL;
		m_BusyListLRU[ii] = NULL;
		m_BusyListMRU[ii] = NULL;
		m_Count[ii] = 0;
	}
}

VertexBufferPool::~VertexBufferPool()
{
	VertexBuffer*	p;

	for (int ii = 0; ii < 16; ii++)
	{
		while (p = m_FreeList[ii])
		{
			m_FreeList[ii] = p->m_Next;
			p->m_Next = NULL;
			p->m_Prev = NULL;
			delete p;
		}
		while (p = m_BusyListLRU[ii])
		{
			m_BusyListLRU[ii] = p->m_Next;
			p->m_Next = NULL;
			p->m_Prev = NULL;
			delete p;
		}
	}
}

// Create
//
// create an initial pool of buffers

void VertexBufferPool::Create(IDirect3D3* d3d, bool force_system)
{
#ifdef TARGET_DC
	// A quarter of the size.
	static int	Allocations[16] = {0,0,0,0, 0,0,128,64,32,16, 8,4, 0,0,0,0};
#else
	static int	Allocations[16] = {0,0,0,0, 0,0,128,64, 32,16,8,4, 0,0,0,0};	// total 48,000 vertices
#endif

	ASSERT(!m_D3D);
	ASSERT(d3d);

	m_D3D = d3d;
	m_SysMem = force_system;

	// create a variety of buffers
	for (int ii = 0; ii < 16; ii++)
	{
		for (int jj = 0; jj < Allocations[ii]; jj++)
		{
			CreateBuffer(ii);
		}
	}
}

// CreateBuffer
//
// try to create a new VertexBuffer and put it on the free list

void VertexBufferPool::CreateBuffer(ULONG logsize)
{
	ASSERT(logsize < 16);

	VertexBuffer* p = new VertexBuffer;
	if (!p)	return;

	HRESULT	res = p->Create(m_D3D, m_SysMem, logsize);

	if (FAILED(res))
	{
		delete p;
		return;
	}

	if (p->Lock())
	{
		p->m_Next = m_FreeList[logsize];
		m_FreeList[logsize] = p;

		m_Count[logsize]++;
	}
	else
	{
		ASSERT(0);
	}
}

// CheckBuffers
//
// try to lock the buffers in the busy list

void VertexBufferPool::CheckBuffers(ULONG logsize, bool time_critical)
{
	VertexBuffer* list = m_BusyListLRU[logsize];

	while (list)
	{
		VertexBuffer*	next = list->m_Next;

		if (list->Lock())
		{
			// remove from busy list
			if (list->m_Prev == NULL)	m_BusyListLRU[logsize]	= list->m_Next;
			else						list->m_Prev->m_Next	= list->m_Next;
			if (list->m_Next == NULL)	m_BusyListMRU[logsize]	= list->m_Prev;
			else						list->m_Next->m_Prev	= list->m_Prev;
			// add to free list
			list->m_Prev = NULL;
			list->m_Next = m_FreeList[logsize];
			m_FreeList[logsize] = list;

			if (time_critical)	return;	// don't check any more
		}

		list = next;
	}
}

// GetBuffer
//
// get a buffer with the given size

VertexBuffer* VertexBufferPool::GetBuffer(ULONG logsize)
{
	ASSERT(logsize < 16);

	if (!m_FreeList[logsize])
	{
		// try and lock a busy buffer
		CheckBuffers(logsize, true);

		if (!m_FreeList[logsize])
		{
			// try and create a new buffer
			CreateBuffer(logsize);

			if (!m_FreeList[logsize])
			{
				if (m_BusyListLRU[logsize])
				{
					// wait for a busy buffer
					ASSERT(0);
					while (!m_FreeList[logsize])	CheckBuffers(logsize, true);
				}
				else
				{
					return NULL;
				}
			}
		}
	}

	VertexBuffer* p = m_FreeList[logsize];
	m_FreeList[logsize] = p->m_Next;
	p->m_Next = NULL;

	ASSERT(p->m_LockedPtr);

	return p;
}

// ReleaseBuffer
//
// release a buffer back to the free list

void VertexBufferPool::ReleaseBuffer(VertexBuffer* buffer)
{
	ASSERT(buffer);
	ASSERT(buffer->m_LockedPtr);
	ASSERT(!buffer->m_Next);

	buffer->m_Next = m_FreeList[buffer->m_LogSize];
	m_FreeList[buffer->m_LogSize] = buffer;
}

// ExpandBuffer
//
// expand a buffer by swapping with a bigger one

VertexBuffer* VertexBufferPool::ExpandBuffer(VertexBuffer* buffer)
{
	ASSERT(buffer);
	ASSERT(buffer->m_LockedPtr);
	ASSERT(!buffer->m_Next);
	ASSERT(buffer->m_LogSize < 16);

	VertexBuffer* p = GetBuffer(buffer->m_LogSize + 1);

	if (p)
	{
		memcpy(p->GetPtr(), buffer->GetPtr(), buffer->GetSize() * sizeof(D3DTLVERTEX));
		ReleaseBuffer(buffer);
	}
	else
	{
		p = buffer;
	}

	return p;
}

// PrepareBuffer
//
// prepare a buffer for rendering

#if USE_D3D_VBUF

IDirect3DVertexBuffer* VertexBufferPool::PrepareBuffer(VertexBuffer* buffer)
{
	ASSERT(buffer);
	ASSERT(buffer->m_LockedPtr);
	ASSERT(!buffer->m_Next);

	buffer->Unlock();

	// add to end (MRU) of busy list
	buffer->m_Next = NULL;
	buffer->m_Prev = m_BusyListMRU[buffer->m_LogSize];
	if (buffer->m_Prev)
	{
		buffer->m_Prev->m_Next = buffer;
	}
	else
	{
		m_BusyListLRU[buffer->m_LogSize] = buffer;
	}
	m_BusyListMRU[buffer->m_LogSize] = buffer;

	return buffer->m_TheBuffer;
}

#endif

// DumpInfo
//
// dump info to a file

#ifndef TARGET_DC
void VertexBufferPool::DumpInfo(FILE* fd)
{
	ReclaimBuffers();

	for (int ii = 0; ii < 16; ii++)
	{
		if (m_Count[ii])
		{
			fprintf(fd, "Buffer size %d vertices (1 << %d)\n\n", 1 << ii, ii);
			fprintf(fd, "Allocated ever = %d\n", m_Count[ii]);

			VertexBuffer*	p;
			int	busy = 0;
			int	free = 0;

			p = m_FreeList[ii];
			while (p)	{ free++; p = p->m_Next; }

			p = m_BusyListLRU[ii];
			while (p)	{ busy++; p = p->m_Next; }

			fprintf(fd, "Free now = %d\n", free);
			fprintf(fd, "Used now = %d\n", m_Count[ii] - free - busy);
			fprintf(fd, "Busy now = %d\n", busy);
		}
		else
		{
			fprintf(fd, "Buffer size %d vertices - never allocated\n", 1 << ii);
		}

		fprintf(fd, "\n");
	}
}
#endif

// ReclaimBuffers
//
// run CheckBuffer on each pool

void VertexBufferPool::ReclaimBuffers()
{
	for (int ii = 0; ii < 16; ii++)
	{
		CheckBuffers(ii, false);
	}
}
