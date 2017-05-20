// polypage.cpp
//
// PolyPage class - main low-level rendering

#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "poly.h"
#include "vertexbuffer.h"
#include "polypoint.h"
#include "renderstate.h"
#include "game.h"
#include "matrix.h"

#include "polypage.h"

#define	VERIFY_SORT		0	// debug - check mergesort gives correct result
#define WIREFRAME		0	// enable wireframe

#if WIREFRAME && !USE_D3D_VBUF
#error Can't define WIREFRAME and not USE_D3D_VBUF
#endif

extern int AENG_total_polys_drawn;




//-----------------------------
// PolyPage
//-----------------------------

// static members

#ifndef TARGET_DC
bool		PolyPage::s_AlphaSort = true;
#endif
ULONG		PolyPage::s_ColourMask = 0xFFFFFFFF;
float		PolyPage::s_XScale = 1.0;
float		PolyPage::s_YScale = 1.0;

float not_private_smiley_xscale;
float not_private_smiley_yscale;


// constructor & destructor

PolyPage::PolyPage(ULONG logsize)
{
	m_VertexBuffer = NULL;
	m_VertexPtr = NULL;
	m_VBLogSize = logsize;
	m_VBUsed = 0;

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	m_PolyBuffer = NULL;
	m_PolyBufSize = 1 << logsize;
	m_PolyBufUsed = 0;

	m_PolySortBuffer = NULL;
	m_PolySortBufSize = 0;
#else
	m_pwIndexBuffer = NULL;
	m_iNumIndicesAlloc = 0;
	m_iNumIndicesUsed = 0;
#endif

#ifdef TEX_EMBED
	m_UScale = 1;
	m_UOffset = 0;
	m_VScale = 1;
	m_VOffset = 0;
	pTheRealPolyPage = this;
#endif

	ASSERT(sizeof(PolyPoint2D) == sizeof(D3DTLVERTEX));
}

PolyPage::~PolyPage()
{
#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	delete[] m_PolyBuffer;
	delete[] m_PolySortBuffer;
#else
	if ( m_pwIndexBuffer != NULL )
	{
		MemFree ( m_pwIndexBuffer );
	}
#endif

	if ( m_VertexBuffer != NULL )
	{
		if ( TheVPool != NULL )
		{
			TheVPool->ReleaseBuffer(m_VertexBuffer);
		}
	}
}

// SetTexEmbed
//
// set texture embedding

#ifdef TEX_EMBED

#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
void PolyPage::SetTexOffset ( D3DTexture *src )
{
	src->GetTexOffsetAndScale ( &m_UScale, &m_UOffset, &m_VScale, &m_VOffset );
}

#else //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

void PolyPage::SetTexEmbed(float u_scale, float u_offset, float v_scale, float v_offset)
{
	m_UScale = u_scale;
	m_UOffset = u_offset;
	m_VScale = v_scale;
	m_VOffset = v_offset;
}

void PolyPage::SetTexOffset(UBYTE offset)
{
	// Kludgy test.
	//offset = 1;

	if (offset)
	{
		int	x = offset & 3;
		int y = (offset >> 2) & 3;

		m_UScale = 0.25f;
		m_VScale = 0.25f;
		m_UOffset = x * 0.25f;
		m_VOffset = y * 0.25f;
	}
	else
	{
		m_UScale = 1.0f;
		m_VScale = 1.0f;
		m_UOffset = 0.0f;
		m_VOffset = 0.0f;
	}
}
#endif //#else //#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB

#endif

// SetGreenScreen
//
// set green screen (or not)

void PolyPage::SetGreenScreen(bool enabled)
{
	s_ColourMask = enabled ? 0xFF00FF00 : 0xFFFFFFFF;
}

// PolyPage::SetScaling
//
// set screen scaling factors

void PolyPage::SetScaling(float xmul, float ymul)
{
	not_private_smiley_xscale = s_XScale = xmul;
	not_private_smiley_yscale = s_YScale = ymul;
}


// PointAlloc
//
// allocate some points

PolyPoint2D* PolyPage::PointAlloc(ULONG num_points)
{

#if defined(DEBUG) && defined(TEX_EMBED)
	// Make sure this is a "real" page.
	ASSERT ( this == pTheRealPolyPage );
#endif

	if (!m_VertexBuffer)
	{
		ASSERT(!m_VBUsed);

		m_VertexBuffer = TheVPool->GetBuffer(m_VBLogSize);

		if (!m_VertexBuffer)	return NULL;

		m_VertexPtr = (PolyPoint2D*)m_VertexBuffer->GetPtr();
		m_VBLogSize = m_VertexBuffer->GetLogSize();
	}

	if (m_VBUsed + num_points > GetVBSize())
	{
		m_VertexBuffer = TheVPool->ExpandBuffer(m_VertexBuffer);
		m_VertexPtr = (PolyPoint2D*)m_VertexBuffer->GetPtr();
		m_VBLogSize = m_VertexBuffer->GetLogSize();

		if (m_VBUsed + num_points > GetVBSize())
		{
			ASSERT ( FALSE );
			return NULL;
		}
	}

	PolyPoint2D*	ptr = m_VertexPtr + m_VBUsed;
	m_VBUsed += num_points;

	return ptr;
}



#if !WE_NEED_POLYBUFFERS_PLEASE_BOB
// Allocates points for a fan, and adds the correct indices to the list to make a fan.
PolyPoint2D* PolyPage::FanAlloc(ULONG num_points)
{

#if defined(DEBUG) && defined(TEX_EMBED)
	// Make sure this is a "real" page.
	ASSERT ( this == pTheRealPolyPage );
#endif

	// Allocate the indices.
	int iNumNewTris = num_points - 2;
	int iNumNewIndices = iNumNewTris * 3;
	if ( iNumNewIndices + m_iNumIndicesUsed > m_iNumIndicesAlloc )
	{
		// Grow it.
		ULONG ulNewSize = iNumNewIndices + m_iNumIndicesUsed;
		// Plus a quarter to reduce thrashing.
		ulNewSize += ulNewSize >> 2;
		// And round up to the nearest 4k chunk.
		ulNewSize = ( ulNewSize + 4095 ) & ~4095;
		WORD *pwNewBuffer = (WORD *)MemAlloc ( sizeof ( WORD ) * ulNewSize );
		ASSERT ( pwNewBuffer != NULL );
		if ( m_pwIndexBuffer != NULL )
		{
			memcpy ( pwNewBuffer, m_pwIndexBuffer, sizeof ( WORD ) * m_iNumIndicesUsed );
			MemFree ( (void *)m_pwIndexBuffer );
		}
		m_pwIndexBuffer = pwNewBuffer;
		m_iNumIndicesAlloc = ulNewSize;
	}
	// Now add the indices.
	WORD iIndex0 = m_VBUsed + 0;
	WORD iIndex1 = m_VBUsed + 1;
	WORD iIndex2 = m_VBUsed + 2;
	for ( ULONG i = 2; i < num_points; i++ )
	{
		m_pwIndexBuffer[m_iNumIndicesUsed+0] = iIndex0;
		m_pwIndexBuffer[m_iNumIndicesUsed+1] = iIndex1;
		m_pwIndexBuffer[m_iNumIndicesUsed+2] = iIndex2;
		m_iNumIndicesUsed += 3;
		iIndex1++;
		iIndex2++;
	}

	// And actually allocate the vertices.
	return PointAlloc ( num_points );
}
#endif


#if WE_NEED_POLYBUFFERS_PLEASE_BOB

// PolyBufAlloc
//
// allocate a polygon in the buffer

PolyPoly* PolyPage::PolyBufAlloc()
{

#if defined(DEBUG) && defined(TEX_EMBED)
	// Make sure this is a "real" page.
	ASSERT ( this == pTheRealPolyPage );
#endif

	if (!m_PolyBuffer)
	{
		ASSERT(!m_PolyBufUsed);

		m_PolyBuffer = new PolyPoly[m_PolyBufSize];
		if (!m_PolyBuffer)	return NULL;
	}

	if (m_PolyBufUsed == m_PolyBufSize)
	{
		PolyPoly*	newbuf = new PolyPoly[m_PolyBufSize * 2];
		if (!newbuf)	return NULL;

		memcpy(newbuf, m_PolyBuffer, m_PolyBufSize * sizeof(PolyPoly));

		delete[] m_PolyBuffer;
		m_PolyBuffer = newbuf;
		m_PolyBufSize *= 2;
	}

	return m_PolyBuffer + (m_PolyBufUsed++);
}

#endif //#if WE_NEED_POLYBUFFERS_PLEASE_BOB


static inline void AlphaPremult(UBYTE* color)
{
	color[0] = UBYTE((ULONG(color[0]) * ULONG(color[3])) >> 8);
	color[1] = UBYTE((ULONG(color[1]) * ULONG(color[3])) >> 8);
	color[2] = UBYTE((ULONG(color[2]) * ULONG(color[3])) >> 8);
}

static inline void InvAlphaPremult(UBYTE* color)
{
	color[0] = UBYTE((ULONG(color[0]) * ULONG(255 - color[3])) >> 8);
	color[1] = UBYTE((ULONG(color[1]) * ULONG(255 - color[3])) >> 8);
	color[2] = UBYTE((ULONG(color[2]) * ULONG(255 - color[3])) >> 8);
}

// AddFan
//
// submit a fan

static PolyPage* ppLastPolyPageSetup = NULL;


#ifdef TEX_EMBED
#define DRAWN_PP ppDrawn
#else
#define DRAWN_PP this
#endif


#if WE_NEED_POLYBUFFERS_PLEASE_BOB

void PolyPage::AddFan(POLY_Point** pts, ULONG num_vertices)
{
	ULONG	ii;

#ifdef TEX_EMBED
	PolyPage *ppDrawn = pTheRealPolyPage;
#endif

	PolyPoly*		pp = DRAWN_PP->PolyBufAlloc();
	ASSERT ( pp != NULL );

	PolyPoint2D*	pv = DRAWN_PP->PointAlloc(num_vertices);
	ASSERT ( pv != NULL );


	pp->first_vertex = pv - DRAWN_PP->m_VertexPtr;
	pp->num_vertices = num_vertices;

	float	zmax = pts[0]->Z;

	// apply Z bias (doesn't seem to work in D3D)
	if (RS.ZLift())
	{
		float	zbias = float(RS.ZLift()) / 65536.0F;
		for (ii = 0; ii < num_vertices; ii++)
		{
			pv[ii].SetSC(pts[ii]->X * s_XScale, pts[ii]->Y * s_YScale, 1.0F - pts[ii]->Z - zbias);
#ifdef TEX_EMBED
			pv[ii].SetUV2(pts[ii]->u * m_UScale + m_UOffset,
						 pts[ii]->v * m_VScale + m_VOffset);
#else
			pv[ii].SetUV(pts[ii]->u, pts[ii]->v);
#endif
			pv[ii].SetColour(pts[ii]->colour & s_ColourMask);
			pv[ii].SetSpecular(pts[ii]->specular);

			if (pts[ii]->Z > zmax)	zmax = pts[ii]->Z;
		}

		pp->sort_z = zmax + zbias;
	}
	else
	{
		float	zmax = pts[0]->Z;

		for (ii = 0; ii < num_vertices; ii++)
		{
			pv[ii].SetSC(pts[ii]->X * s_XScale, pts[ii]->Y * s_YScale, 1.0F - pts[ii]->Z);
#ifdef TEX_EMBED
			pv[ii].SetUV2(pts[ii]->u * m_UScale + m_UOffset,
						 pts[ii]->v * m_VScale + m_VOffset);
#else
			pv[ii].SetUV(pts[ii]->u, pts[ii]->v);
#endif
			pv[ii].SetColour(pts[ii]->colour & s_ColourMask);
			pv[ii].SetSpecular(pts[ii]->specular);

			if (pts[ii]->Z > zmax)	zmax = pts[ii]->Z;
		}

		pp->sort_z = zmax;
	}

#if WIREFRAME
	AddWirePoly(pts, num_vertices);
#endif

}


// AddWirePoly
//
// submit a wireframe polygon

void PolyPage::AddWirePoly(POLY_Point** pts, ULONG num_vertices)
{
#if WIREFRAME
	ULONG	ii;

#ifdef TEX_EMBED
	PolyPage *ppDrawn = pTheRealPolyPage;
#endif


	PolyPoly*		pp = DRAWN_PP->PolyBufAlloc();
	if (!pp)	return;

	PolyPoint2D*	pv = DRAWN_PP->PointAlloc(num_vertices);
	if (!pv)	return;

	pp->first_vertex = pv - DRAWN_PP->m_VertexPtr;
	pp->num_vertices = num_vertices | 0x8000;

	// apply Z bias (doesn't seem to work in D3D)
	float	zbias = float(RS.ZLift() + 8) / 65536.0F;

	for (ii = 0; ii < num_vertices; ii++)
	{
		pv[ii].SetSC(pts[ii]->X * s_XScale, pts[ii]->Y * s_YScale, 1.0F - pts[ii]->Z - zbias);
		pv[ii].SetUV(pts[ii]->u, pts[ii]->v);
		pv[ii].SetColour(0x00808080);
		pv[ii].SetSpecular(0xFF000000);
	}

	pp->sort_z = pts[0]->Z + zbias;
#endif
}

#endif //#if WE_NEED_POLYBUFFERS_PLEASE_BOB



// MassageVertices
//
// change vertices to match RS.GetEffect()

void PolyPage::MassageVertices()
{
#ifdef TEX_EMBED
	if ( pTheRealPolyPage != this )
	{
		// Don't do this to non-drawn pages.
		return;
	}
#endif

	if (RS.GetEffect())
	{
		ULONG			ii;
		D3DTLVERTEX*	vptr = m_VertexBuffer->GetPtr();

		switch (RS.GetEffect())
		{
		case RS_AlphaPremult:
			for (ii = 0; ii < m_VBUsed; ii++)
			{
				AlphaPremult((UBYTE*)&vptr[ii].color);
			}
			break;

		case RS_BlackWithAlpha:
			for (ii = 0; ii < m_VBUsed; ii++)
			{
				vptr[ii].color &= 0xFF000000;
			}
			break;

		case RS_InvAlphaPremult:
			for (ii = 0; ii < m_VBUsed; ii++)
			{
				InvAlphaPremult((UBYTE*)&vptr[ii].color);
			}
			break;

		case RS_DecalMode:
			for (ii = 0; ii < m_VBUsed; ii++)
			{
				vptr[ii].color = 0xFFFFFFFF;
			}
			break;
		}
	}
}

// Render
//
// render to card (render state should already be set up)

static UWORD	IxBuffer[65536];

void PolyPage::Render(IDirect3DDevice3* dev)
{
	ULONG	ii;

	if (!m_VertexBuffer/* || !m_PolyBufUsed*/)	return;

#if defined(DEBUG) && defined(TEX_EMBED)
	// Should only be rendering stuff from polypages that are real ones,
	// i.e. they are not just living on another page.
	// So this should just map back to itself.
	ASSERT ( this == pTheRealPolyPage );
#endif

	// apply vertex FX
	MassageVertices();

#if USE_D3D_VBUF
	IDirect3DVertexBuffer*	vb = TheVPool->PrepareBuffer(m_VertexBuffer);
	m_VertexBuffer = NULL;
	m_VertexPtr = NULL;

	PolyPoly*	src = m_PolyBuffer;
	UWORD*		dst = IxBuffer;

	for (ii = 0; ii < m_PolyBufUsed; ii++)
	{
		UWORD	v1 = src->first_vertex;

		ASSERT(dst - IxBuffer + 3 < 65536);

	#if WIREFRAME
		if (!(src->num_vertices & 0x8000))
	#endif
		{
			for (ULONG jj = 2; jj < src->num_vertices; jj++)
			{
				*dst++ = v1;
				*dst++ = v1 + jj - 1;
				*dst++ = v1 + jj;
			}
		}
		src++;
	}

	dev->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, vb, IxBuffer, dst - IxBuffer, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT);

#if WIREFRAME
	src = m_PolyBuffer;
	dst = IxBuffer;

	for (ii = 0; ii < m_PolyBufUsed; ii++)
	{
		UWORD v1 = src->first_vertex;

		if (src->num_vertices & 0x8000)
		{
			for (ULONG jj = 1; jj < (src->num_vertices & 0x7FFF); jj++)
			{
				*dst++ = v1 + jj - 1;
				*dst++ = v1 + jj;
			}
			*dst++ = v1 + jj - 1;
			*dst++ = v1;
		}
		src++;
	}

	dev->SetTexture(0, NULL);
	dev->DrawIndexedPrimitiveVB(D3DPT_LINELIST, vb, IxBuffer, dst - IxBuffer, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT);
#endif

#else	// !USE_D3D_VBUF


	#ifndef TARGET_DC
	if (!Keys[KB_F8])
	{
		PolyPoly*	src = m_PolyBuffer;
		UWORD*		dst = IxBuffer;

		for (ii = 0; ii < m_PolyBufUsed; ii++)
		{
			UWORD	v1 = src->first_vertex;

			for (ULONG jj = 2; jj < src->num_vertices; jj++)
			{
				*dst++ = v1;
				*dst++ = v1 + jj - 1;
				*dst++ = v1 + jj;
			}
			src++;
		}


		HRESULT hres;

		hres = dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, m_VertexPtr, m_VBUsed, IxBuffer, dst - IxBuffer, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);

		// ASSERT(hres == D3D_OK);
	}
	else
	#endif
	{
		HRESULT hres;

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
		hres = dev->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, m_VertexPtr, m_VBUsed, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
#else
		hres = dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, m_VertexPtr, m_VBUsed, m_pwIndexBuffer, m_iNumIndicesUsed, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT);
#endif

		ASSERT(hres == D3D_OK);
	}

#endif

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	AENG_total_polys_drawn += m_PolyBufUsed;
	m_PolyBufUsed = 0;
#else
	//AENG_total_polys_drawn += m_VBUsed / 3;
	AENG_total_polys_drawn += m_iNumIndicesUsed / 3;
	m_iNumIndicesUsed = 0;
#endif

	m_VBUsed = 0;
}



#if WE_NEED_POLYBUFFERS_PLEASE_BOB

// DrawSinglePoly
//
// draw a single polygon

void PolyPage::DrawSinglePoly(PolyPoly* poly, IDirect3DDevice3* dev)
{
	UWORD*	dst = IxBuffer;

	UWORD	v1 = poly->first_vertex;

	for (ULONG jj = 2; jj < poly->num_vertices; jj++)
	{
		*dst++ = v1;
		*dst++ = v1 + jj - 1;
		*dst++ = v1 + jj;
	}

#if USE_D3D_VBUF
//	dev->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, m_VB, IxBuffer, dst - IxBuffer, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP | D3DDP_DONOTLIGHT);
#endif
}

// AddToBuckets
//
// add polygons to buckets

void PolyPage::AddToBuckets(PolyPoly* buckets[], int count)
{
	if (!m_VertexBuffer || !m_PolyBufUsed)	return;

	// apply vertex effects
	MassageVertices();

	// obtain vertex buffer pointer
#if USE_D3D_VBUF
	m_VB = TheVPool->PrepareBuffer(m_VertexBuffer);
#endif

	// add to buckets
	for (DWORD ii = 0; ii < m_PolyBufUsed; ii++)
	{
		PolyPoly*	poly = &m_PolyBuffer[ii];

		int	bucket = int(poly->sort_z * count);
		if (bucket < 0)			bucket = 0;
		if (bucket >= count)	bucket = count - 1;

		poly->page = this;
		poly->next = buckets[bucket];
		buckets[bucket] = poly;
	}

	// clear everything else
#if USE_D3D_VBUF
	m_VertexBuffer = NULL;
	m_VertexPtr = NULL;
#endif
	m_VBUsed = 0;
	m_PolyBufUsed = 0;
}

#endif //#if WE_NEED_POLYBUFFERS_PLEASE_BOB


// Clear
//
// clear buffer

void PolyPage::Clear()
{
	if (!m_VertexBuffer)	return;

//#if USE_D3D_VBUF
	TheVPool->ReleaseBuffer(m_VertexBuffer);
	m_VertexBuffer = NULL;
	m_VertexPtr = NULL;
//#endif

	m_VBUsed = 0;
#if WE_NEED_POLYBUFFERS_PLEASE_BOB
#ifdef TARGET_DC
#error Don't use this on a DC you fool!
#endif
	m_PolyBufUsed = 0;
#else
	// Free up the index buffer
	if ( m_pwIndexBuffer != NULL )
	{
		MemFree ( m_pwIndexBuffer );
		ASSERT ( m_iNumIndicesAlloc != 0 );
	}
	else
	{
		ASSERT ( m_iNumIndicesAlloc == 0 );
	}
	m_pwIndexBuffer = NULL;
	m_iNumIndicesAlloc = 0;
	m_iNumIndicesUsed = 0;
#endif //#if WE_NEED_POLYBUFFERS_PLEASE_BOB
}

// SortBackFirst
//
// sort polygons (approx) by Z

#if WE_NEED_POLYBUFFERS_PLEASE_BOB

void PolyPage::SortBackFirst()
{
	if (!m_PolyBufUsed || !PolyPage::AlphaSortEnabled())	return;

	// delete old sort array if buffer has been resized
	if (m_PolySortBuffer && (m_PolySortBufSize != m_PolyBufSize))
	{
		delete[] m_PolySortBuffer;
		m_PolySortBuffer = NULL;
	}

	// allocate a sort array if necessary
	// (lazy allocation so unsorted pages don't waste RAM)
	if (!m_PolySortBuffer)
	{
		m_PolySortBufSize = m_PolyBufSize;
		m_PolySortBuffer = new PolyPoly[m_PolySortBufSize];

		if (!m_PolySortBuffer)	return;	// erk
	}

	// run the merge sort (non-recursive for speed)
	ULONG	sort_len = 1;	// sort pairs first

	while (sort_len < m_PolyBufUsed)
	{
		// sort each <n*2> items by merging 2 sets of <n> items
		MergeSortIteration(sort_len);
		
		// switch arrays
		PolyPoly* tmp = m_PolyBuffer;
		m_PolyBuffer = m_PolySortBuffer;
		m_PolySortBuffer = tmp;

		// now merge pairs of these sets
		sort_len *= 2;
	}

#if VERIFY_SORT
	for (ULONG jj = 0; jj < m_PolyBufUsed - 1; jj++)
	{
		ASSERT(m_PolyBuffer[jj] <= m_PolyBuffer[jj+1]);
	}
#endif
}

// DoMerge
//
// merge a single set (class T must have a < & <= operator, hopefully inline)

template <class T>
inline static void DoMerge(const T* src, T* dst, ULONG len1, ULONG len2)
{
	ULONG	pos1 = 0;
	ULONG	pos2 = len1;
	ULONG	wpos = 0;
	ULONG	end = len1 + len2;

	for (;;)
	{
		if (src[pos2] < src[pos1])
		{
			// write from 2nd array
			dst[wpos++] = src[pos2++];
			if (pos2 == end)	break;	// end of 2nd input array
		}
		else
		{
			// write from 1st array
			dst[wpos++] = src[pos1++];
			if (pos1 == len1)	break;	// end of 1st input array
		}
	}

	// append the rest of the other array
	if (pos1 == len1)
	{
		while (pos2 < end)
		{
			dst[wpos++] = src[pos2++];
		}
	}
	else
	{
		while (pos1 < len1)
		{
			dst[wpos++] = src[pos1++];
		}
	}

#if VERIFY_SORT
	ASSERT(wpos == end);
	ASSERT(pos1 == len1);
	ASSERT(pos2 == end);

	for (ULONG jj = 0; jj < end - 1; jj++)
	{
		ASSERT(dst[jj] <= dst[jj+1]);
	}
#endif
}

// MergeSortIteration
//
// merge pairs of size sort_len

void PolyPage::MergeSortIteration(ULONG sort_len)
{
	ULONG		ii;
	ULONG		set_len = sort_len * 2;
	ULONG		limit = m_PolyBufUsed - set_len;	// inclusive
	PolyPoly*	src = m_PolyBuffer;
	PolyPoly*	dst = m_PolySortBuffer;

	if (sort_len == 1)
	{
		// zip through pair-sorts
		for (ii = 0; ii <= limit; ii += set_len)
		{
			if (src[0].sort_z > src[1].sort_z)
			{
				// wrong order
				dst[0] = src[1];
				dst[1] = src[0];
			}
			else
			{
				// right order
				dst[0] = src[0];
				dst[1] = src[1];
			}
			src += set_len;
			dst += set_len;
		}
		// last bit
		if (ii != m_PolyBufUsed)
		{
			dst[0] = src[0];
		}
	}
	else
	{
		// merge each pair of sets
		for (ii = 0; ii <= limit; ii += set_len)
		{
			DoMerge<PolyPoly>(src, dst, sort_len, sort_len);
			src += set_len;
			dst += set_len;
		}
		// last bit
		if (ii != m_PolyBufUsed)
		{
			if (ii + sort_len >= m_PolyBufUsed)
			{
				// only 1 part - just copy
				while (ii < m_PolyBufUsed)
				{
					*dst++ = *src++;
					ii++;
				}
			}
			else
			{
				// 2 parts - merge
				DoMerge<PolyPoly>(src, dst, sort_len, m_PolyBufUsed - (ii + sort_len));
			}
		}
	}
}


#endif //#if WE_NEED_POLYBUFFERS_PLEASE_BOB




// A routine to emulate the DC's DrawPrimtiveMM call on the PC, so
// that people can use it when developing on the PC.

// See polypage.h for more details.


void GenerateMMMatrixFromStandardD3DOnes (	D3DMATRIX *pmOutput,
											const D3DMATRIX *mProjectionMatrix,
											const D3DMATRIX *mWorldMatrix,
											const D3DVIEWPORT2 *d3dvpt )
{
	// Matrices must be 32-byte aligned.
	ASSERT ( ( (DWORD)(pmOutput) & 31 ) == 0 );

	D3DMATRIX mMyPrivateWorld;
	if ( mWorldMatrix == NULL )
	{
		// Use the standard camera matrix, rather than anything custom.
		// This is faster than calling POLY_set_local_rotation_none first.
		float POLY_cam_off_x = -POLY_cam_x;
		float POLY_cam_off_y = -POLY_cam_y;
		float POLY_cam_off_z = -POLY_cam_z;

		MATRIX_MUL(
			POLY_cam_matrix,
			POLY_cam_off_x,
			POLY_cam_off_y,
			POLY_cam_off_z);

		mMyPrivateWorld._11 = POLY_cam_matrix[0];
		mMyPrivateWorld._21 = POLY_cam_matrix[1];
		mMyPrivateWorld._31 = POLY_cam_matrix[2];
		mMyPrivateWorld._41 = POLY_cam_off_x;
		mMyPrivateWorld._12 = POLY_cam_matrix[3];
		mMyPrivateWorld._22 = POLY_cam_matrix[4];
		mMyPrivateWorld._32 = POLY_cam_matrix[5];
		mMyPrivateWorld._42 = POLY_cam_off_y;
		mMyPrivateWorld._13 = POLY_cam_matrix[6];
		mMyPrivateWorld._23 = POLY_cam_matrix[7];
		mMyPrivateWorld._33 = POLY_cam_matrix[8];
		mMyPrivateWorld._43 = POLY_cam_off_z;
		mMyPrivateWorld._14 = 0.0f;
		mMyPrivateWorld._24 = 0.0f;
		mMyPrivateWorld._34 = 0.0f;
		mMyPrivateWorld._44 = 1.0f;
		mWorldMatrix = &mMyPrivateWorld;
	}

	D3DMATRIX matTemp;
	{
		//_Multiply4dM((float *)pResultMatrix, (float *)g_matWorld, (float *)g_matProjection);

		matTemp._11 = mWorldMatrix->_11*mProjectionMatrix->_11 + mWorldMatrix->_12*mProjectionMatrix->_21 + mWorldMatrix->_13*mProjectionMatrix->_31 + mWorldMatrix->_14*mProjectionMatrix->_41;
		matTemp._12 = mWorldMatrix->_11*mProjectionMatrix->_12 + mWorldMatrix->_12*mProjectionMatrix->_22 + mWorldMatrix->_13*mProjectionMatrix->_32 + mWorldMatrix->_14*mProjectionMatrix->_42;
		matTemp._13 = mWorldMatrix->_11*mProjectionMatrix->_13 + mWorldMatrix->_12*mProjectionMatrix->_23 + mWorldMatrix->_13*mProjectionMatrix->_33 + mWorldMatrix->_14*mProjectionMatrix->_43;
		matTemp._14 = mWorldMatrix->_11*mProjectionMatrix->_14 + mWorldMatrix->_12*mProjectionMatrix->_24 + mWorldMatrix->_13*mProjectionMatrix->_34 + mWorldMatrix->_14*mProjectionMatrix->_44;

		matTemp._21 = mWorldMatrix->_21*mProjectionMatrix->_11 + mWorldMatrix->_22*mProjectionMatrix->_21 + mWorldMatrix->_23*mProjectionMatrix->_31 + mWorldMatrix->_24*mProjectionMatrix->_41;
		matTemp._22 = mWorldMatrix->_21*mProjectionMatrix->_12 + mWorldMatrix->_22*mProjectionMatrix->_22 + mWorldMatrix->_23*mProjectionMatrix->_32 + mWorldMatrix->_24*mProjectionMatrix->_42;
		matTemp._23 = mWorldMatrix->_21*mProjectionMatrix->_13 + mWorldMatrix->_22*mProjectionMatrix->_23 + mWorldMatrix->_23*mProjectionMatrix->_33 + mWorldMatrix->_24*mProjectionMatrix->_43;
		matTemp._24 = mWorldMatrix->_21*mProjectionMatrix->_14 + mWorldMatrix->_22*mProjectionMatrix->_24 + mWorldMatrix->_23*mProjectionMatrix->_34 + mWorldMatrix->_24*mProjectionMatrix->_44;

		matTemp._31 = mWorldMatrix->_31*mProjectionMatrix->_11 + mWorldMatrix->_32*mProjectionMatrix->_21 + mWorldMatrix->_33*mProjectionMatrix->_31 + mWorldMatrix->_34*mProjectionMatrix->_41;
		matTemp._32 = mWorldMatrix->_31*mProjectionMatrix->_12 + mWorldMatrix->_32*mProjectionMatrix->_22 + mWorldMatrix->_33*mProjectionMatrix->_32 + mWorldMatrix->_34*mProjectionMatrix->_42;
		matTemp._33 = mWorldMatrix->_31*mProjectionMatrix->_13 + mWorldMatrix->_32*mProjectionMatrix->_23 + mWorldMatrix->_33*mProjectionMatrix->_33 + mWorldMatrix->_34*mProjectionMatrix->_43;
		matTemp._34 = mWorldMatrix->_31*mProjectionMatrix->_14 + mWorldMatrix->_32*mProjectionMatrix->_24 + mWorldMatrix->_33*mProjectionMatrix->_34 + mWorldMatrix->_34*mProjectionMatrix->_44;

		matTemp._41 = mWorldMatrix->_41*mProjectionMatrix->_11 + mWorldMatrix->_42*mProjectionMatrix->_21 + mWorldMatrix->_43*mProjectionMatrix->_31 + mWorldMatrix->_44*mProjectionMatrix->_41;
		matTemp._42 = mWorldMatrix->_41*mProjectionMatrix->_12 + mWorldMatrix->_42*mProjectionMatrix->_22 + mWorldMatrix->_43*mProjectionMatrix->_32 + mWorldMatrix->_44*mProjectionMatrix->_42;
		matTemp._43 = mWorldMatrix->_41*mProjectionMatrix->_13 + mWorldMatrix->_42*mProjectionMatrix->_23 + mWorldMatrix->_43*mProjectionMatrix->_33 + mWorldMatrix->_44*mProjectionMatrix->_43;
		matTemp._44 = mWorldMatrix->_41*mProjectionMatrix->_14 + mWorldMatrix->_42*mProjectionMatrix->_24 + mWorldMatrix->_43*mProjectionMatrix->_34 + mWorldMatrix->_44*mProjectionMatrix->_44;
	}

#if 0
	// Officially correct version.
	DWORD dwWidth = d3dvpt.dwWidth >> 1;
	DWORD dwHeight = d3dvpt.dwHeight >> 1;
	DWORD dwX = d3dvpt.dwX;
	DWORD dwY = d3dvpt.dwY;
#else
	// Version that knows about the letterbox mode hack.
extern DWORD g_dw3DStuffHeight;
extern DWORD g_dw3DStuffY;
	DWORD dwWidth = d3dvpt->dwWidth >> 1;
	DWORD dwHeight = g_dw3DStuffHeight >> 1;
	DWORD dwX = d3dvpt->dwX;
	DWORD dwY = g_dw3DStuffY;
#endif
	pmOutput->_11 = 0.0f;
	pmOutput->_12 = matTemp._11 *   (float)dwWidth  + matTemp._14 * (float)( dwX + dwWidth  );
	pmOutput->_13 = matTemp._12 * - (float)dwHeight + matTemp._14 * (float)( dwY + dwHeight );
	pmOutput->_14 = matTemp._14;
	pmOutput->_21 = 0.0f;
	pmOutput->_22 = matTemp._21 *   (float)dwWidth  + matTemp._24 * (float)( dwX + dwWidth  );
	pmOutput->_23 = matTemp._22 * - (float)dwHeight + matTemp._24 * (float)( dwY + dwHeight );
	pmOutput->_24 = matTemp._24;
	pmOutput->_31 = 0.0f;
	pmOutput->_32 = matTemp._31 *   (float)dwWidth  + matTemp._34 * (float)( dwX + dwWidth  );
	pmOutput->_33 = matTemp._32 * - (float)dwHeight + matTemp._34 * (float)( dwY + dwHeight );
	pmOutput->_34 = matTemp._34;
	// Validation magic number.
	unsigned long EVal = 0xe0001000;
	pmOutput->_41 = *(float *)&EVal;
	pmOutput->_42 = matTemp._41 *   (float)dwWidth  + matTemp._44 * (float)( dwX + dwWidth  );
	pmOutput->_43 = matTemp._42 * - (float)dwHeight + matTemp._44 * (float)( dwY + dwHeight );
	pmOutput->_44 = matTemp._44;
}









// You can usually get the standard data from these globals - I keep them 
// all current, and update g_matWorld when you call POLY_set_local_rotation
// and similar calls. You can use a different matrix of course and not call
// POLY_set_local_rotation, which is probably slightly faster.
extern D3DMATRIX g_matProjection;
extern D3DMATRIX g_matWorld;
extern D3DVIEWPORT2 g_viewData;


#ifndef TARGET_DC

// The DC version is just #defined in the header.

// dwFVFType must be D3DFVF_VERTEX or D3DFVF_LVERTEX.
// d3dmm is the multimatrix info block:
HRESULT DrawIndPrimMM ( LPDIRECT3DDEVICE3 lpDevice,
						DWORD dwFVFType,
						D3DMULTIMATRIX *d3dmm,
						WORD wNumVertices,
						WORD *pwIndices,
						DWORD dwNumIndices )
{
	// Check alignments.
	ASSERT ( ( (DWORD)(d3dmm->lpd3dMatrices) & 31 ) == 0 );
	ASSERT ( ( (DWORD)(d3dmm->lpvVertices) & 31 ) == 0 );
	ASSERT ( ( (DWORD)(d3dmm->lpLightTable) & 3 ) == 0 );
	ASSERT ( ( (DWORD)(d3dmm->lpvLightDirs) & 7 ) == 0 );

	ASSERT ( ( dwFVFType == D3DFVF_LVERTEX ) || ( dwFVFType == D3DFVF_VERTEX ) );

	D3DTLVERTEX pTLVert[3];
	D3DLVERTEX *pLVert = (D3DLVERTEX *)d3dmm->lpvVertices;

	WORD *pwCurIndex = pwIndices;
	while ( TRUE )
	{
		WORD wIndex[3];
		// Start a strip.
		wIndex[1] = *pwCurIndex++;
		wIndex[2] = *pwCurIndex++;
		ASSERT ( dwNumIndices > 1 );
		dwNumIndices -= 2;
		bool bEven = TRUE;
		while ( TRUE )
		{
			bEven = !bEven;
			wIndex[0] = wIndex[1];
			wIndex[1] = wIndex[2];
			wIndex[2] = *pwCurIndex++;
			ASSERT ( dwNumIndices > 0 );
			dwNumIndices--;

			if ( wIndex[2] == 0xffff )
			{
				// End of list.
				break;
			}

			
			// Load & transform the verts.
			for ( int i = 0; i < 3; i++ )
			{
				WORD wVertIndex = wIndex[i];
				ASSERT ( wVertIndex < wNumVertices );
				D3DLVERTEX *pLVertCur = pLVert + wVertIndex;

				BYTE bMatIndex = ((unsigned char*)(pLVertCur))[12];

				D3DMATRIX *pmCur = &(d3dmm->lpd3dMatrices[bMatIndex]);
				ASSERT ( *((DWORD*)(&(pmCur->_41))) == 0xe0001000 );

				pTLVert[i].dvSX = pLVertCur->dvX * pmCur->_12 + pLVertCur->dvY * pmCur->_22 + pLVertCur->dvZ * pmCur->_32 + pmCur->_42;
				pTLVert[i].dvSY = pLVertCur->dvX * pmCur->_13 + pLVertCur->dvY * pmCur->_23 + pLVertCur->dvZ * pmCur->_33 + pmCur->_43;
				pTLVert[i].dvSZ = pLVertCur->dvX * pmCur->_14 + pLVertCur->dvY * pmCur->_24 + pLVertCur->dvZ * pmCur->_34 + pmCur->_44;
				pTLVert[i].dvRHW = 1.0f / pTLVert[i].dvSZ;
				pTLVert[i].dvSX *= pTLVert[i].dvRHW;
				pTLVert[i].dvSY *= pTLVert[i].dvRHW;
				pTLVert[i].dvSZ *= ( POLY_ZCLIP_PLANE );


				pTLVert[i].dvTU			= pLVert[wIndex[i]].dvTU;
				pTLVert[i].dvTV			= pLVert[wIndex[i]].dvTV;


				if ( dwFVFType == D3DFVF_VERTEX )
				{
					// I don't actually do lighting yet - just make sure it's visible.
					pTLVert[i].dcColor		= 0xffffffff;
					pTLVert[i].dcSpecular	= 0xffffffff;
				}
				else
				{
					pTLVert[i].dcColor		= pLVert[wIndex[i]].dcColor;
					pTLVert[i].dcSpecular	= pLVert[wIndex[i]].dcSpecular;
				}
			}

			// And draw the thing.
			WORD wMyIndices[3];
			if ( bEven )
			{
				wMyIndices[0] = 0;
				wMyIndices[1] = 2;
				wMyIndices[2] = 1;
			}
			else
			{
				wMyIndices[0] = 0;
				wMyIndices[1] = 1;
				wMyIndices[2] = 2;
			}
			//HRESULT hres = lpDevice->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, pTLVert, 3, wMyIndices, 3, 0 );
			HRESULT hres = lpDevice->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, pTLVert, 3, wMyIndices, 3, 0 );
			if ( FAILED ( hres ) )
			{
				return ( hres );
			}
		}
		if ( dwNumIndices == 0 )
		{
			// No more indices.
			break;
		}
	}

	// Check that the next index exists to work around MS driver bug.
	//ASSERT ( *pwCurIndex == 0x1234 );

	return ( DD_OK );
}


#endif
