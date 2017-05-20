// polypage.h
//
// PolyPage class - main low-level rendering

#ifndef TARGET_DC

// PC

// this makes absolutely fuck-all difference to speed (tested)
//#define TEX_EMBED		// must be set the same in D3DTexture.h
// Do need to sort, and so need polybuffers
#define WE_NEED_POLYBUFFERS_PLEASE_BOB 1

#else

// DREAMCAST

// But it makes the VQ much more efficient, so do it!
#define TEX_EMBED		// must be set the same in D3DTexture.h
// Don't need to sort, and so don't need polybuffers
#define WE_NEED_POLYBUFFERS_PLEASE_BOB 0

#endif

#ifndef _POLYPAGE_
#define _POLYPAGE_

#include "renderstate.h"
#include "vertexbuffer.h"
#include "polypoint.h"

class PolyPage;

// PolyPoly
//
// a polygon in a PolyPage

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
struct PolyPoly
{
	float		sort_z;			// z-value to sort on
	UWORD		first_vertex;	// first vertex #
	UWORD		num_vertices;	// number of vertices; if top bit set, draw as wireframe
	PolyPage*	page;			// page for polygon (only when in a bucket)
	PolyPoly*	next;			// next polygon (only when in a bucket)
};

inline bool operator<(const PolyPoly& arg1, const PolyPoly& arg2)	{ return arg1.sort_z < arg2.sort_z; }
inline bool operator<=(const PolyPoly& arg1, const PolyPoly& arg2)	{ return arg1.sort_z <= arg2.sort_z; }
inline bool operator>(const PolyPoly& arg1, const PolyPoly& arg2)	{ return !(arg1 <= arg2); }
inline bool operator>=(const PolyPoly& arg1, const PolyPoly& arg2)	{ return !(arg1 < arg2); }

#endif //#if WE_NEED_POLYBUFFERS_PLEASE_BOB

// PolyPage
//
// a polygon page

#pragma pack( push, 4 )
class PolyPage
{
public:
	PolyPage(ULONG logsize = 6);
	~PolyPage();

#ifdef TEX_EMBED
	// texture embedding

	PolyPage			*pTheRealPolyPage;			// The poly page you actually need to add tris to.
													// This is never NULL, but may point back to this one.

#if USE_FANCY_TEXTURE_PAGES_PLEASE_BOB
	void				SetTexOffset ( D3DTexture *src );
#else
	void				SetTexEmbed(float u_scale, float u_offset, float v_scale, float v_offset);
#endif
	void				SetTexOffset(UBYTE offset);	// 0 for (0,0)-(1,1) else 128 + (0-15) for the subtexture
#endif

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	// fan submission
	void				AddFan(POLY_Point** pts, ULONG num_vertices);
	void				AddWirePoly(POLY_Point** pts, ULONG num_vertices);
#endif

	// set greenscreen
	static void			SetGreenScreen(bool enabled = true);

	// set scaling for different screen sizes
	static void			SetScaling(float xmul, float ymul);

#ifdef TARGET_DC
	// DC does all our sorting for us.
	static void			EnableAlphaSort()				{}
	static void			DisableAlphaSort()				{}
	static bool			AlphaSortEnabled()				{ return FALSE; }
#else

	// sort polygons in approx. Z order
	void				SortBackFirst();

	// sort enabling
	static void			EnableAlphaSort()				{ s_AlphaSort = true; }
	static void			DisableAlphaSort()				{ s_AlphaSort = false; }
	static bool			AlphaSortEnabled()				{ return s_AlphaSort; }
#endif

	// render polygons to card
#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	bool				NeedsRendering()				{ return m_PolyBufUsed > 0; }
#else
	bool				NeedsRendering()				{ return m_VBUsed > 0; }
#endif
	void				Render(IDirect3DDevice3* dev);

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	// render polygons using bucket sort
	void				AddToBuckets(PolyPoly* buckets[], int count);
	void				DrawSinglePoly(PolyPoly* poly, IDirect3DDevice3* dev);
#endif

	// clear without rendering
	void				Clear();

	// render state for the page
	RenderState			RS;


	// static members
#ifndef TARGET_DC
	static bool			s_AlphaSort;		// alpha sort enabled flag
#endif
	static ULONG		s_ColourMask;		// colour mask for green-screen monitor FX
	static float		s_XScale;			// X scale for screen vertices
	static float		s_YScale;			// Y scale for screen vertices


	PolyPoint2D*		PointAlloc(ULONG num_points);	// allocate some points - DONT USE.
	PolyPoint2D*		FanAlloc(ULONG num_points);		// Allocate a fan polygon.
														// You just fill in the data, the indices are handled magically.


#ifdef TEX_EMBED
	float				m_UScale;
	float				m_UOffset;
	float				m_VScale;
	float				m_VOffset;
#endif



//private:

	// member variables
	VertexBuffer*		m_VertexBuffer;		// vertex buffer
	PolyPoint2D*		m_VertexPtr;		// pointer to vertices in buffer
	ULONG				m_VBLogSize;		// log2 of buffer size
	ULONG				m_VBUsed;			// number of vertices used
	ULONG				GetVBSize()			{ return 1 << m_VBLogSize; }

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	PolyPoly*			m_PolyBuffer;		// polygon buffer
	ULONG				m_PolyBufSize;		// size of polygon buffer
	ULONG				m_PolyBufUsed;		// number of polygons used

	PolyPoly*			m_PolySortBuffer;	// polygon sort buffer
	ULONG				m_PolySortBufSize;	// size of polygon sort buffer
#else
	// Index buffer.
	WORD				*m_pwIndexBuffer;	// The list of indices.
	ULONG				m_iNumIndicesAlloc;	// How many indices are allocated.
	ULONG				m_iNumIndicesUsed;	// How many indices are used.
#endif


#if USE_D3D_VBUF
	IDirect3DVertexBuffer*	m_VB;			// vertex buffer pointer, only used in bucket sort
#endif

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	// SortBackFirst iteration
	void				MergeSortIteration(ULONG sort_len);
#endif

	// submission helpers
//	PolyPoint2D*		PointAlloc(ULONG num_points);	// allocate some points

#if WE_NEED_POLYBUFFERS_PLEASE_BOB
	PolyPoly*			PolyBufAlloc();					// allocate a polygon
#endif


	// massage vertices according to RS.GetEffect()
	void				MassageVertices();
};
#pragma pack( pop )

extern PolyPage	POLY_Page[POLY_NUM_PAGES];

#endif



// A routine to emulate the DC's DrawPrimtiveMM call on the PC, so
// that people can use it when developing on the PC.


// An actual function.



// Notes:
//
// No front-plane clipping is performed - do it yourself, or use the old AddTri routs.
// Side-band clipping is done by the hardware.
// Only D3D_VERTEX and D3D_LVERTEX types supported.
// No alpha-blended/alpha-tested polys allowed - solid only.
// Lighting is complex - yell at TomF if you want to do lighting. I'm not going to address it in this header file.

// The 12th byte of the vertex data holds the index
// of the matrix it uses for transformation. This byte is the least-significant
// part of the mantissa for N.X - it makes no difference to the lighting at all.
// You can set it easily using this macro:
//#define SET_MM_INDEX(v,i) (((unsigned char*) &v)[12] = (unsigned char)i)
// Remember to do this AFTER copying in all the standard data :-)

// The indices are not actually in list order - they are in strip order, but
// an index of -1 (0xFFFF) starts a new strip. So the order
// The number of indices supplied MUST include the final FFFF. Also, when
// allocating the indices, make sure you allocate one more. Doesn't matter what's in it,
// but it must be valid memory. A small bug in the MS driver means this word is read,
// then discarded, and it doesn't cause a problem except when the index falls off the
// end of an allocated page and causes a page fault. Took me ages to find that bug.
// I will be checking that it is 0x1234 for debugging purposes, unless you give
// me a good reason not to (e.g. you're storing index lists right next to each other).
//
// On this strips-with-termination thing, the index list 0,1,2,3,4,-1,5,6,7,-1,2,4,8,-1 gives you:
//
// 1---3      5---6
// |\  |\     |  /
// | \ | \    | /
// |  \|  \   |/
// 0---2---4  7
//     |  /
//     | /
//     |/
//     8
//
// If you want to generate optimal strips from random indexed list data, I have
// some utility routs that will do it for you. Yell - TomF.

// The matrices are generated in a slightly odd way. The easiest way to do this
// is to call this function to generate them:
// If mWorldMatrix == NULL, then the rout will get it from the standard camera setup.
extern void GenerateMMMatrixFromStandardD3DOnes (	D3DMATRIX *mOutput,
											const D3DMATRIX *mProjectionMatrix,
											const D3DMATRIX *mWorldMatrix,
											const D3DVIEWPORT2 *d3dvpt );

// You can usually get the standard data from these globals - I keep them 
// all current, and update g_matWorld when you call POLY_set_local_rotation
// and similar calls. You can use a different matrix of course and not call
// POLY_set_local_rotation, which is probably slightly faster.
extern D3DMATRIX g_matProjection;
// Actually, don't use g_matWorld, just pass in NULL.
extern D3DMATRIX g_matWorld;
extern D3DVIEWPORT2 g_viewData;



#ifdef TARGET_DC

// Just a straight alias.
#define DrawIndPrimMM(dev,type,d3dmm,numvert,pwind,numind) dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,type,(void*)d3dmm,numvert,pwind,numind,D3DDP_MULTIMATRIX)


#else


// This is already defined by DX in a DC build.
struct D3DMULTIMATRIX
{
	LPVOID			lpvVertices;			// Pointer to the vertex data. MUST be 32-byte aligned.
	LPD3DMATRIX		lpd3dMatrices;			// Pointer to the array of matrices. MUST be 32-byte aligned.
	LPVOID			lpvLightDirs;			// Pointer to the array of light vectors (NULL if not lighting). MUST be 8-byte aligned.
	LPD3DCOLOR		lpLightTable;			// Pointer to the fade table (NULL if not lighting). MUST be 4-byte aligned.
};

// Also in the DC/DX headers,
#define SET_MM_INDEX(v,i) (((unsigned char*) &v)[12] = (unsigned char)i)


// dwFVFType must be D3DFVF_VERTEX or D3DFVF_LVERTEX.
// d3dmm is the multimatrix info block:
extern HRESULT DrawIndPrimMM ( LPDIRECT3DDEVICE3 lpDevice,
						DWORD dwFVFType,
						D3DMULTIMATRIX *d3dmm,
						WORD wNumVertices,
						WORD *pwIndices,
						DWORD dwNumIndices );

#endif


// Useful.
#define GET_MM_INDEX(v) (((unsigned char*)&v)[12])







