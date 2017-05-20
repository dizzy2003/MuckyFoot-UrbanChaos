// vertexbuffer.h
//
// Vertex buffer stuff (DX6)

#ifndef _VERTEXBUFFER_
#define _VERTEXBUFFER_

#ifndef TARGET_DC
// tried and it is about 33% faster than not using D3D vertex buffers
#define	USE_D3D_VBUF	0		// set to 0 to revert to malloc'd vertex buffers

#else
// DC doesn't have (or need) them
#define	USE_D3D_VBUF	0
#endif

extern void VB_Init();
extern void VB_Term();

// VertexBuffer
//
// a D3D vertex buffer

class VertexBuffer
{
public:
	~VertexBuffer();

	D3DTLVERTEX*	GetPtr()		{ ASSERT(m_LockedPtr); return m_LockedPtr; }
	ULONG			GetSize()		{ return 1 << m_LogSize; }
	ULONG			GetLogSize()	{ return m_LogSize; }

private:
	friend class VertexBufferPool;

	VertexBuffer();

	// create the buffer
	HRESULT			Create(IDirect3D3* d3d, bool force_system, ULONG logsize = 6);

	D3DTLVERTEX*	Lock();			// lock the buffer
	void			Unlock();		// unlock the buffer

	ULONG			m_LogSize;		// log2 of size
	D3DTLVERTEX*	m_LockedPtr;	// ptr to memory iff locked
	VertexBuffer*	m_Prev;			// prev in chain
	VertexBuffer*	m_Next;			// next in chain

#if USE_D3D_VBUF
	IDirect3DVertexBuffer*	m_TheBuffer;
#endif
};

// VertexBufferPool
//
// vertex buffer pool

class VertexBufferPool
{
public:
	VertexBufferPool();
	~VertexBufferPool();

	void					Create(IDirect3D3* d3d, bool force_system);	// create pool and preallocate some buffers

	VertexBuffer*			GetBuffer(ULONG logsize = 6);				// get a new buffer
	void					ReleaseBuffer(VertexBuffer* buffer);		// release a buffer

	VertexBuffer*			ExpandBuffer(VertexBuffer* buffer);			// expand a buffer
#if USE_D3D_VBUF
	IDirect3DVertexBuffer*	PrepareBuffer(VertexBuffer* buffer);		// prepare a buffer for rendering
#endif

#ifndef TARGET_DC
	void					DumpInfo(FILE* fd);							// dump info out
#endif
	
	void					ReclaimBuffers();							// try to reclaim any free buffers

private:
	IDirect3D3*		m_D3D;					// D3D object
	bool			m_SysMem;				// use system memory?
	VertexBuffer*	m_FreeList[16];			// free vertex buffers (locked)
	VertexBuffer*	m_BusyListLRU[16];		// busy vertex buffers, least recent end = head
	VertexBuffer*	m_BusyListMRU[16];		// busy vertex buffers, most recent end = tail
	ULONG			m_Count[16];			// total number of buffers

	void	CreateBuffer(ULONG logsize);						// create a buffer of the given size
	void	CheckBuffers(ULONG logsize, bool time_critical);	// check busy buffers
};

extern VertexBufferPool*	TheVPool;

#endif
