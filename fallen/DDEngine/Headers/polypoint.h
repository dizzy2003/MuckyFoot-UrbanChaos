// polypoint.h
//
// PolyPoint class, encapsulating a D3DTLVERTEX

// PolyPoint2D
//
// really a D3DTLVERTEX with helper functions
//
// the coordinates are stored as (x/z,y/z,1-1/z,1/z)

#ifndef _POLYPOINT_
#define _POLYPOINT_

// we copy U,V using int copies; this is because u,v are uninitialized in
// a lot of cases.  we can only hope that the driver doesn't read U,V into
// the FPU because it's not practical to check every single point where a
// POLY_Point is initialized ...
#ifdef TARGET_DC
#define	INT_COPY_FLOAT(DST,SRC)		((DST)=(SRC))
#else
#define	INT_COPY_FLOAT(DST,SRC)		*((int*)&(DST)) = *((int*)&(SRC))
#endif

class PolyPoint2D : private D3DTLVERTEX	// don't even *think* about making this a public base-class
{
public:
	// Helper functions for setup:

	void	SetSC(float sx, float sy, float sz = 0);				// Set Screen coordinates

	void	SetUV(float& u, float& v);								// Set u,v
	void	SetUV2(float u, float v);								// Set u,v

	static ULONG	MakeD3DColour(UBYTE r, UBYTE g, UBYTE b, UBYTE a);		// make D3D colour value
	static ULONG	ModulateD3DColours(ULONG c1, ULONG c2);					// modulate 2 D3D colour values

	void	SetColour(ULONG d3d_col);								// Set colour
	void	SetColour(UBYTE r, UBYTE g, UBYTE b, UBYTE a = 0xFF);

	void	SetSpecular(ULONG d3d_col);								// Set specular colour
	void	SetSpecular(UBYTE r, UBYTE g, UBYTE b, UBYTE a = 0xFF);

	/*
	D3DTLVERTEX	*GetTLVert ( void )
	{
		return ( (D3DTLVERTEX *)this );
	}
	*/
};

inline void PolyPoint2D::SetSC(float _sx, float _sy, float _sz)
{
	sx  = _sx;
	sy  = _sy;
	sz  = _sz;
	rhw = 1.0F - _sz;
}

inline void PolyPoint2D::SetUV(float& u, float& v)
{
	INT_COPY_FLOAT(tu, u);
	INT_COPY_FLOAT(tv, v);
}

inline void PolyPoint2D::SetUV2(float u, float v)
{
	INT_COPY_FLOAT(tu, u);
	INT_COPY_FLOAT(tv, v);
}

inline ULONG PolyPoint2D::MakeD3DColour(UBYTE r, UBYTE g, UBYTE b, UBYTE a)
{
	return (ULONG(a) << 24) | (ULONG(r) << 16) | (ULONG(g) << 8) | ULONG(b);
}

inline void PolyPoint2D::SetColour(ULONG d3d_col)
{
	color = d3d_col;
}

inline void PolyPoint2D::SetColour(UBYTE r, UBYTE g, UBYTE b, UBYTE a)
{
	SetColour(MakeD3DColour(r,g,b,a));
}

inline void PolyPoint2D::SetSpecular(ULONG d3d_col)
{
	specular = d3d_col;
}

inline void PolyPoint2D::SetSpecular(UBYTE r, UBYTE g, UBYTE b, UBYTE a)
{
	SetSpecular(MakeD3DColour(r,g,b,a));
}

// NOTE: doesn't round properly, doesn't modulate alpha

inline ULONG PolyPoint2D::ModulateD3DColours(ULONG c1, ULONG c2)
{
	ULONG	res;

	res = ((c1 & 0xFF) * (c2 & 0xFF)) >> 8;
	c1 >>= 8;	c2 >>= 8;
	res |= ((c1 & 0xFF) * (c2 & 0xFF)) & 0xFF00;
	c1 >>= 8;	c2 >>= 8;
	res |= (((c1 & 0xFF) * (c2 & 0xFF)) & 0xFF00) << 8;
	res |= (c1 & 0xFF00) << 16;

	return res;
}

#endif
