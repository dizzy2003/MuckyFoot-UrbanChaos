//
// Draw a person.
// 

#include "game.h"
#include "aeng.h"
#include "poly.h"
#include "figure.h"
#include "sprite.h"
#include "fmatrix.h"
#include "mav.h"
#include "interact.h"
#include "night.h"
#include "shadow.h"
#include "matrix.h"
#include "animate.h"
#include "mesh.h"
#include "dirt.h"
#include "texture.h"

#include "math.h"
#include "interfac.h"



#include "Hierarchy.h"  // JCL is gay // MA wears girly shorts // JCL
#include "Quaternion.h"
#include "memory.h"

#include "person.h"
#include "pcom.h"
#include "eway.h"
#include "dirt.h"
#include	"ddlib.h"
#include	"panel.h"

#include "renderstate.h"
#include "poly.h"
#include "polypage.h"

#include "psystem.h"



#ifndef TARGET_DC

#define LOG_ENTER(x) {}
#define LOG_EXIT(x)  {}
#define LOG_EVENT(x) {}

#endif


#ifdef TARGET_DC
// intrinsic maths
#include <shsgintr.h>
#endif


extern BOOL allow_debug_keys;

SLONG FIGURE_alpha = 255;

SLONG	steam_seed;

extern	UWORD	alt_texture[];

UBYTE	body_part_upper[]=
{
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0

};




void DeadAndBuried ( DWORD dwColour )
{
	// draw a colour on the screen to try to diagnose things if they do crash.

	DDSURFACEDESC2	mine;
	HRESULT			res;

	InitStruct(mine);
	mine.dwFlags = DDSD_PITCH;
	res = the_display.lp_DD_FrontSurface->Lock ( NULL, &mine, DDLOCK_WAIT, NULL );
	if (FAILED(res))	return;

	char *pdwDest = (char *)mine.lpSurface;
	for ( int i = 0; i < 50; i++ )
	{
		DWORD *pdwDest1 = (DWORD*)pdwDest;
		pdwDest += mine.lPitch;
		for ( int j = 0; j < 50; j++ )
		{
			*pdwDest1++ = dwColour;
		}
	}

	res = the_display.lp_DD_FrontSurface->Unlock ( NULL );

}





// Returns TRUE if this chunk is not near-Z clipped, FALSE if it is.
bool FIGURE_draw_prim_tween_person_only_just_set_matrix (
		int iMatrixNum,
		SLONG prim,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		SLONG recurse_level,
		Thing    *p_thing
		);

void FIGURE_draw_prim_tween_person_only(
		SLONG prim,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		SLONG recurse_level,
		Thing    *p_thing
		);



// Now enabled only on a cheat!
//#define HIGH_REZ_PEOPLE_PLEASE_BOB


#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB
bool m_bPleaseInflatePeople = FALSE;
#endif



#if USE_TOMS_ENGINE_PLEASE_BOB



#if USE_TOMS_ENGINE_PLEASE_BOB


// Useful.
#define ALIGNED_STATIC_ARRAY(def,name,number,mytype,align)														\
	static char c##name##mytype##align##StaticArray [ align + number * sizeof ( mytype ) ];						\
	def##name = (mytype *)( ( (DWORD)c##name##mytype##align##StaticArray + (align-1) ) & ~(align-1) )



ALIGNED_STATIC_ARRAY ( static D3DCOLOR *, MM_pcFadeTable, 128, D3DCOLOR, 4 );
ALIGNED_STATIC_ARRAY ( static D3DCOLOR *, MM_pcFadeTableTint, 128, D3DCOLOR, 4 );
ALIGNED_STATIC_ARRAY ( static D3DMATRIX *, MM_pMatrix, 1, D3DMATRIX, 32 );
ALIGNED_STATIC_ARRAY ( static D3DVERTEX *, MM_Vertex, 4, D3DVERTEX, 32 );
ALIGNED_STATIC_ARRAY ( static float *, MM_pNormal, 4, float, 8 );

#if 0
// The MM lighting table.
D3DCOLOR *MM_pcFadeTable = NULL;
D3DCOLOR *MM_pcFadeTableTint = NULL;
D3DMATRIX *MM_pMatrix = NULL;
D3DVERTEX *MM_Vertex = NULL;
float *MM_pNormal = NULL;
#endif

D3DVECTOR MM_vLightDir;
bool MM_bLightTableAlreadySetUp = FALSE;



// Something to build lighting tables.
void BuildMMLightingTable ( Pyro *p, DWORD colour_and=0xffffffff )
{
	LOG_ENTER ( Figure_Build_Table )


	// REMEMBER THAT ALL THE LIGHT DIRECTIONS ARE NEGATIVE,
	// EXCEPT FOR THE AMBIENT ONE, WHICH IS CORRECT.
	// Daft bloody system.

	ASSERT ( MM_pcFadeTable != NULL );
	ASSERT ( MM_pcFadeTableTint != NULL );
	ASSERT ( MM_pMatrix != NULL );
	ASSERT ( MM_Vertex != NULL );
	ASSERT ( MM_pNormal != NULL );


	// Find the brightest-lit vector.
	float fBright = NIGHT_amb_red * 0.35f + NIGHT_amb_green * 0.45f + NIGHT_amb_blue * 0.2f;
	D3DVECTOR vTotal;
	vTotal.x = fBright * NIGHT_amb_norm_x;
	vTotal.y = fBright * NIGHT_amb_norm_y;
	vTotal.z = fBright * NIGHT_amb_norm_z;
	NIGHT_Found *nf;
	int j;
	for (j = 0; j < NIGHT_found_upto; j++)
	{
		nf = &NIGHT_found[j];

		// Find a rough approximation to brightness.
		float fBright = nf->r * 0.35f + nf->g * 0.45f + nf->b * 0.2f;

		// If it is a negative light - make it "illuminate" from the opposite direction.
		// This solves the problem of a red light above, an anti-red below, and a soft green to the right.
		// If the reds both pulled in opposite direction, the green would actually be the main one,
		// which is silly as the object is obviously mainly red and black.
		//if ( fBright > 0.0f )	
		{
			// These are negative!
			vTotal.x -= fBright * nf->dx;
			vTotal.y -= fBright * nf->dy;
			vTotal.z -= fBright * nf->dz;
		}
#if 0
		else
		{
			// Negative light - make it "illuminate" from the correct direction.
			// Directions are also negative!
			vTotal.x += fBright * nf->dx;
			vTotal.y += fBright * nf->dy;
			vTotal.z += fBright * nf->dz;
		}
#endif
	}

	// Unitise the vector.
#ifdef TARGET_DC
	float fLength = _InvSqrtA ( vTotal.x * vTotal.x + vTotal.y * vTotal.y + vTotal.z * vTotal.z );
#else
	float fLength = 1.0f / sqrtf ( vTotal.x * vTotal.x + vTotal.y * vTotal.y + vTotal.z * vTotal.z );
#endif
	vTotal.x *= fLength;
	vTotal.y *= fLength;
	vTotal.z *= fLength;

	// Now light this vector and its negative, just as the software engine does.
	D3DCOLORVALUE cvAmb, cvLight;
//#define AMBIENT_FACTOR (1.0f * 256.0f)
#define AMBIENT_FACTOR (0.5f * 256.0f)
	// Real ambient factor.
	cvAmb.r = NIGHT_amb_red * AMBIENT_FACTOR;
	cvAmb.g = NIGHT_amb_green * AMBIENT_FACTOR;
	cvAmb.b = NIGHT_amb_blue * AMBIENT_FACTOR;

	// Terrible fudge - on nighttime missions, the ambient is too dark, or on daytime ones, it's too bright.
	// So if the given ambient (the sun) is below a certain value, ramp up the ambient to compensate.
	if ( ( NIGHT_amb_red + NIGHT_amb_green + NIGHT_amb_blue ) < 100 )
	{
		// FUDGE IT!
		cvAmb.r *= 2.0f;
		cvAmb.g *= 2.0f;
		cvAmb.b *= 2.0f;
	}

	cvLight = cvAmb;

	float fDotProd = vTotal.x * NIGHT_amb_norm_x + vTotal.y * NIGHT_amb_norm_y + vTotal.z * NIGHT_amb_norm_z;
	if ( fDotProd < 0.0f )
	{
		cvAmb.r -= fDotProd * NIGHT_amb_red;
		cvAmb.g -= fDotProd * NIGHT_amb_green;
		cvAmb.b -= fDotProd * NIGHT_amb_blue;
	}
	else
	{
		cvLight.r += fDotProd * NIGHT_amb_red;
		cvLight.g += fDotProd * NIGHT_amb_green;
		cvLight.b += fDotProd * NIGHT_amb_blue;
	}
	for (j = 0; j < NIGHT_found_upto; j++)
	{
		nf = &NIGHT_found[j];
		// These are negative!
		fDotProd = - vTotal.x * nf->dx - vTotal.y * nf->dy - vTotal.z * nf->dz;
		if ( fDotProd < 0.0f )
		{
			cvAmb.r -= fDotProd * nf->r;
			cvAmb.g -= fDotProd * nf->g;
			cvAmb.b -= fDotProd * nf->b;
		}
		else
		{
			cvLight.r += fDotProd * nf->r;
			cvLight.g += fDotProd * nf->g;
			cvLight.b += fDotProd * nf->b;
		}
	}

	MM_vLightDir = vTotal;

	// Is this thing on fire?
	if ( p != NULL )
	{
		// On fire. Blacken with soot or something.
		cvLight.r = (cvLight.r > p->counter) ? (cvLight.r - p->counter) : 10;
		cvLight.g = (cvLight.g > p->counter) ? (cvLight.g - p->counter) :  4;
		cvLight.b = (cvLight.b > p->counter) ? (cvLight.b - p->counter) :  3;
		cvAmb.r = (cvAmb.r > p->counter) ? (cvAmb.r - p->counter) : 10;
		cvAmb.g = (cvAmb.g > p->counter) ? (cvAmb.g - p->counter) :  4;
		cvAmb.b = (cvAmb.b > p->counter) ? (cvAmb.b - p->counter) :  3;
	}

#if 1
	// On the PC, people are lit x2, so we need to double these.
	cvLight.r *= 2.0f;
	cvLight.g *= 2.0f;
	cvLight.b *= 2.0f;
	cvAmb.r *= 2.0f;
	cvAmb.g *= 2.0f;
	cvAmb.b *= 2.0f;
#endif

	// OK, now make up the fade table for this light.
	const float fColourScale = ( 1.0f / ( 256.0f * 128.0f ) ) * 255.0f * NIGHT_LIGHT_MULTIPLIER;
	D3DCOLORVALUE cvCur = cvAmb;
	cvLight.r -= cvAmb.r;
	cvLight.g -= cvAmb.g;
	cvLight.b -= cvAmb.b;
	cvCur.r *= fColourScale;
	cvCur.g *= fColourScale;
	cvCur.b *= fColourScale;
	cvLight.r *= ( fColourScale / 64.0f );
	cvLight.g *= ( fColourScale / 64.0f );
	cvLight.b *= ( fColourScale / 64.0f );

	// Ambient colour.
	unsigned char cR, cG, cB;
	if ( cvCur.r > 255.0f )
	{
		cR = 255;
	}
	else if ( cvCur.r < 0.0f )
	{
		cR = 0;
	}
	else
	{
		cR = (unsigned char)( cvCur.r );
	}
	if ( cvCur.g > 255.0f )
	{
		cG = 255;
	}
	else if ( cvCur.g < 0.0f )
	{
		cG = 0;
	}
	else
	{
		cG = (unsigned char)( cvCur.g );
	}
	if ( cvCur.b > 255.0f )
	{
		cB = 255;
	}
	else if ( cvCur.b < 0.0f )
	{
		cB = 0;
	}
	else
	{
		cB = (unsigned char)( cvCur.b );
	}

	DWORD dwColour = ( (DWORD)cR << 16 ) | ( (DWORD)cG << 8 ) | ( (DWORD)cB );
	int i;
	for ( i = 64; i < 128; i++ )
	{
		MM_pcFadeTable[i] = dwColour;
		MM_pcFadeTableTint[i] = dwColour & colour_and;
	}

	// Ramp up to full brightness.
	for ( i = 0; i < 64; i++ )
	{
		if ( cvCur.r > 255.0f )
		{
			cR = 255;
		}
		else if ( cvCur.r < 0.0f )
		{
			cR = 0;
		}
		else
		{
			cR = (unsigned char)( cvCur.r );
		}
		if ( cvCur.g > 255.0f )
		{
			cG = 255;
		}
		else if ( cvCur.g < 0.0f )
		{
			cG = 0;
		}
		else
		{
			cG = (unsigned char)( cvCur.g );
		}
		if ( cvCur.b > 255.0f )
		{
			cB = 255;
		}
		else if ( cvCur.b < 0.0f )
		{
			cB = 0;
		}
		else
		{
			cB = (unsigned char)( cvCur.b );
		}
		dwColour = ( (DWORD)cR << 16 ) | ( (DWORD)cG << 8 ) | ( (DWORD)cB );
		MM_pcFadeTable[i] = dwColour;
		MM_pcFadeTableTint[i] = dwColour & colour_and;
		cvCur.r += cvLight.r;
		cvCur.g += cvLight.g;
		cvCur.b += cvLight.b;
	}

	LOG_EXIT ( Figure_Build_Table )
}


#endif





struct EdgeList
{
	WORD wPt1, wPt2;
	WORD wMidPt;
};



// The MS list optimiser. They gave it to me, so I'll use it! Might as well.

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    Copyright (c) 1998-1999 Microsoft Corporation

    Module Name: optlist.cpp

    Abstract: Source code for Index List Optimizer
-------------------------------------------------------------------*/ 

typedef struct tagIndexTri
{
    int v1,v2,v3;
    int done;
} INDEXTRISTRUCT;

typedef struct tagIndexVert
{
    WORD wIndex;           // Index of this vertex
    int nShareCount;       // Number of triangles sharing this vertex
    int rgSharedTris[30];  // Array of triangles that share this vertex
} INDEXVERTSTRUCT;

class MSMesh
{
private:
    INDEXTRISTRUCT *m_aTri;
    int        m_nNumTri;
    int        m_nMaxTri;
    INDEXVERTSTRUCT   *m_aVert;
    int        m_nNumVert;
    int        m_nMaxVert;

    int     NextTri( int v1, int v2, int *v3 );
    void    ClearTempDone( int nValue );
    int     StripLen( int startTri, int dir, int setit, WORD *pwOut, int nMaxLen );
    
public:
            MSMesh();
            ~MSMesh();
    void    Clear() { m_nNumTri = 0; m_nNumVert = 0; }
    int     AddTri( WORD wV1, WORD wV2, WORD wV3 );
    int     GetStrip( WORD *pwV, int maxlen );
    int     NumVertices() { return m_nNumVert; }
    int     FindOrAddVertex( WORD wVert );
    int     SetSize( int nTriangles );
};

MSMesh::MSMesh()
{
    memset( this, 0, sizeof(*this) );
}

int MSMesh::SetSize ( int nTriangles )
{
    if ( nTriangles <= 0 )
    {
    //  printf("SetSize: freeing allocations\n");
        if ( m_aTri )
        {
            MemFree( m_aTri );
            m_aTri = (INDEXTRISTRUCT *)NULL;
        }
        if ( m_aVert )
        {
            MemFree( m_aVert );
            m_aVert = (INDEXVERTSTRUCT *)NULL;
        }
        m_nMaxTri = 0;
        m_nMaxVert = 0;
    }
    else
    {
        if( nTriangles > m_nMaxTri )
        {
            // MemFree any previous allocation
            SetSize(0);
            m_nMaxTri = nTriangles + 10;    // allow room for growth
            m_nMaxVert = m_nMaxTri * 3;
            if( m_nMaxVert > 65536 )
                m_nMaxVert = 65536;
            // Allocate new space
            // printf("SetSize allocating space for %d indices and %d vertices\n",
            //      m_nMaxTri, m_nMaxVert );
            m_aTri = (INDEXTRISTRUCT *)MemAlloc( m_nMaxTri * sizeof(INDEXTRISTRUCT) );
			if ( m_aTri == NULL ) { DeadAndBuried ( 0x07e007e0 ); }
            m_aVert = (INDEXVERTSTRUCT *)MemAlloc( m_nMaxVert * sizeof(INDEXVERTSTRUCT) );
			if ( m_aVert == NULL ) { DeadAndBuried ( 0x07e007e0 ); }
            if( !m_aTri || !m_aVert )
                SetSize(0);    // MemFree half-allocations
        }
    }
    return m_nMaxTri;
}

MSMesh::~MSMesh()
{
    SetSize(0);
}

int MSMesh::FindOrAddVertex( WORD wVert )
{
    int vertno;
    for( vertno =0; vertno < m_nNumVert; vertno++ )
        if( m_aVert[vertno].wIndex == wVert )
            return vertno;
    vertno = m_nNumVert++;
    m_aVert[vertno].wIndex = wVert;
    m_aVert[vertno].nShareCount = 0;
    return vertno;
}

int MSMesh::AddTri( WORD wV1, WORD wV2, WORD wV3 )
{
    int trino = m_nNumTri++;
    int v1 = FindOrAddVertex( wV1 );
    int v2 = FindOrAddVertex( wV2 );
    int v3 = FindOrAddVertex( wV3 );

    m_aVert[v1].rgSharedTris[m_aVert[v1].nShareCount++] = trino;
    m_aVert[v2].rgSharedTris[m_aVert[v2].nShareCount++] = trino;
    m_aVert[v3].rgSharedTris[m_aVert[v3].nShareCount++] = trino;
    
    m_aTri[trino].v1 = v1;
    m_aTri[trino].v2 = v2;
    m_aTri[trino].v3 = v3;
/*
    printf(" tri %d(%d,%d,%d)\n", trino,
        m_aTri[trino].v1,
        m_aTri[trino].v2,
        m_aTri[trino].v3 );
*/
    m_aTri[trino].done = 0;

    return trino;
}

void MSMesh::ClearTempDone( int nValue )
{
    int trino;
    for( trino = 0; trino < m_nNumTri; trino++ )
        if( m_aTri[trino].done == 2 )
        {
            m_aTri[trino].done = nValue;
        }
}

int MSMesh::StripLen( int startTri, int dir, int setit, WORD *pwOut, int nMaxLen )
{
    int v1, v2, v3;
    int trino = startTri;
    int len = 0;

    if( (dir>>1)  == 0 )
    {
        v1 = m_aTri[startTri].v1;
        v2 = m_aTri[startTri].v2;
        v3 = m_aTri[startTri].v3;
    }
    else if( (dir>>1) == 1 )
    {
        v1 = m_aTri[startTri].v2;
        v2 = m_aTri[startTri].v3;
        v3 = m_aTri[startTri].v1;
    }
    else
    {
        v1 = m_aTri[startTri].v3;
        v2 = m_aTri[startTri].v1;
        v3 = m_aTri[startTri].v2;
    }

    while( trino != -1 )
    {
        if( pwOut )
        {
            *pwOut++ = m_aVert[v1].wIndex;
            *pwOut++ = m_aVert[v2].wIndex;
            *pwOut++ = m_aVert[v3].wIndex;
        //  printf("len = %d, adding triangle %d:  %d,%d,%d\n", len, trino, v1, v2, v3 );
        }
        len++;
        m_aTri[trino].done = 2;    // mark as temporarily done
        if( len >= nMaxLen )
            break;

        // Sequence:
        //
        //  0 1 2
        //  1 3 2
        //  2 3 4
        //  3 5 4

        if( len & 1 )    // generate 1 3 2 from 0 1 2
        {
            v1 = v2;
            trino = NextTri( v3, v1, &v2 );
        }
        else            // generate 2 3 4 from 1 3 2
        {
            v1 = v3;
            trino = NextTri( v1, v2, &v3 );
        }

    }

    ClearTempDone(setit);

    // printf( "\nStrip starting at triangle %d, dir %d len=%d\n", startTri, dir, len);

    return len;
}

int MSMesh::GetStrip ( WORD *pwVT, int maxLen )
{
    int trino;
    int len;
    int dir;

    int bestStart;
    int bestDir;
    int bestLen=0;

    // printf("MSMesh:  %d triangles,  %d vertices\n", m_nNumTri, m_nNumVert );

    for( trino = 0; trino < m_nNumTri; trino++ )
    {
        if( m_aTri[trino].done )
            continue;
        for( dir = 0; dir<3; dir++ )
        {
            if( ( len = StripLen( trino, dir, 0, NULL, maxLen ) ) > bestLen )
            {                                                   
                bestLen = len;                                   
                bestStart = trino;                                   
                bestDir = dir;
                if( bestLen >= maxLen )
                {
                    // force it to exit
                    trino = m_nNumTri;
                    break;
                }
            }

        }
    }
    if( bestLen > 0 )
        StripLen( bestStart, bestDir, 1, pwVT, maxLen );

    return bestLen;
}

int MSMesh::NextTri( int v1, int v2, int *v3 )
{
    int subtri;
    int trino;

    for( subtri = 0; subtri < m_aVert[v1].nShareCount; subtri++ )
    {
        trino = m_aVert[v1].rgSharedTris[subtri];
        if( m_aTri[trino].done )
            continue;
        if( m_aTri[trino].v1 == v1 )
        {
            if( m_aTri[trino].v2 == v2 )
            {
                *v3 = m_aTri[trino].v3;
        //  printf("found tri %d:  %d,%d,%d\n", trino, v1, v2, *v3 );
                return trino;
            }
        }
        else if( m_aTri[trino].v2 == v1 )
        {
            if( m_aTri[trino].v3 == v2 )
            {
                *v3 = m_aTri[trino].v1;
        //  printf("found tri %d:  %d,%d,%d\n", trino, v1, v2, *v3 );
                return trino;
            }
        }
        else if( m_aTri[trino].v3 == v1 )
        {
            if( m_aTri[trino].v1 == v2 )
            {
                *v3 = m_aTri[trino].v2;
       //  printf("found tri %d:  %d,%d,%d\n", trino, v1, v2, *v3 );
                return trino;
            }
        }
    }
    return -1; // no more in this direction
}

static MSMesh mesh;

BOOL MSOptimizeIndexedList( WORD *pwIndices, int nTriangles )
{

   if( !mesh.SetSize( nTriangles ) )
      return FALSE;

   mesh.Clear();

   WORD *pwInd = pwIndices;
   
   for ( int i = 0; i < nTriangles; i++ )
   {
      mesh.AddTri( pwInd[0], pwInd[1], pwInd[2] );
      pwInd += 3;
   }

   if( mesh.NumVertices() == 0 )
     return FALSE;

   int nLength = nTriangles;
   int nTotal = 0;

   while ( nTotal < nTriangles )
   {
      nLength = mesh.GetStrip( pwIndices, nLength );
      pwIndices += ( nLength * 3 );
      nTotal += nLength;
   }

   return TRUE;
}


#endif //#if USE_TOMS_ENGINE_PLEASE_BOB



/*


// D3D lighting fucntions.
#define MAX_NUM_OF_LIGHTS 10
LPDIRECT3DLIGHT m_pLight[MAX_NUM_OF_LIGHTS];
bool m_bLightInScene[MAX_NUM_OF_LIGHTS];
int m_iNumLights = 0;

int FindMeAFreeLightAndPutItInTheScene ( void )
{
	int i = 0;
	while ( TRUE )
	{
		ASSERT ( i < MAX_NUM_OF_LIGHTS );
		if ( !m_bLightInScene[i] )
		{
			// Found one.
			break;
		}
		i++;
	}

	if ( m_pLight[i] == NULL )
	{
		// Haven't created this one yet.
		HRESULT hres = (the_display.lp_D3D)->CreateLight ( &(m_pLight[i]), NULL );
		ASSERT ( SUCCEEDED ( hres ) );
	}
	// Add the light in.
	HRESULT hres = (the_display.lp_D3D_Viewport)->AddLight ( m_pLight[i] );
	ASSERT ( SUCCEEDED ( hres ) );
	m_bLightInScene[i] = TRUE;
	return ( i );
}

void MakeThisLightLikeThis ( int iLightNum, D3DLIGHT *pd3dLight )
{
	ASSERT ( m_bLightInScene[iLightNum] );
	ASSERT ( m_pLight[iLightNum] != NULL );
	HRESULT hres = (m_pLight[iLightNum])->SetLight ( (D3DLIGHT *)pd3dLight );
	ASSERT ( SUCCEEDED ( hres ) );
}

void MakeThisLightLikeThis ( int iLightNum, D3DLIGHT2 *pd3dLight )
{
	MakeThisLightLikeThis ( iLightNum, (D3DLIGHT *)pd3dLight );
}

void RemoveThisLightFromScene ( int iLightNum )
{
	ASSERT ( m_bLightInScene[iLightNum] );
	ASSERT ( m_pLight[iLightNum] != NULL );
	HRESULT hres = (the_display.lp_D3D_Viewport)->DeleteLight ( m_pLight[iLightNum] );
	ASSERT ( SUCCEEDED ( hres ) );
	m_bLightInScene[iLightNum] = FALSE;
}

#if 0
int AddThisSortOfLightToTheScene ( D3DLIGHT *pd3dLight )
{
	int iLightNum = FindMeAFreeLightAndPutItInTheScene();
	MakeThisLightLikeThis ( iLightNum, pd3dLight );
	return ( iLightNum );
}

int AddThisSortOfLightToTheScene ( D3DLIGHT2 *pd3dLight )
{
	return ( AddThisSortOfLightToTheScene ( (D3DLIGHT *)pd3dLight ) );
}
void RemoveAllLightsFromScene ( void )
{
	for ( int i = 0; i < MAX_NUM_OF_LIGHTS; i++ )
	{
		if ( m_bLightInScene[i] )
		{
			RemoveThisLightFromScene ( i );
		}
	}
}

#else
// HACKED!
int AddThisSortOfLightToTheScene ( D3DLIGHT2 *pd3dLight )
{
	if ( m_pLight[0] == NULL )
	{
		FindMeAFreeLightAndPutItInTheScene();
		MakeThisLightLikeThis ( 0, pd3dLight );
	}
	return ( 0 );
}
void RemoveAllLightsFromScene ( void )
{
}
#endif


*/



SLONG	get_steam_rand(void)
{
	steam_seed*=31415965;
	steam_seed+=123456789;
	return(steam_seed>>8);
}

extern	int AENG_detail_crinkles;

#define	MAX_STEAM	100
void	draw_steam(SLONG x,SLONG y,SLONG z,SLONG lod)
{
	SLONG	c0;
	float	u,v;
	SLONG	trans;
	//
	// waft gently up from x,y,z
	//

	// Internal gubbins.
	POLY_set_local_rotation_none();

	steam_seed=54321678;
	if(lod>40)
		lod=40;

	for(c0=0;c0<lod;c0++)
	{
		SLONG	dx,dy,dz;

		if(c0&1)
			u=0.0;
		else
			u=0.5;
		if(c0&2)
			v=0.0;
		else
			v=0.5;
		
		dy=get_steam_rand()&0x1ff;
		dy+=(GAME_TURN*((c0&3)+2));
		dy%=500;
		dx=(((get_steam_rand()&0xff)-128)*((dy>>2)+80))>>9;
		dz=(((get_steam_rand()&0xff)-128)*((dy>>2)+80))>>9;


		if(dy>16)
		{
//			dx+=COS(GAME_TURN*10)%(dy>>4);
//			dz+=SIN(GAME_TURN*10)%(dy>>4);
		}

		if(c0&4)
		{
			dx+=COS(dy*4);
			dz+=SIN(dy*4);

		}

/*		trans=(c0*128)/MAX_STEAM;
		if(dy>300)
        	trans=(trans*5)/(dy-295);*/
/*
        if (dy<40)
			trans=dy*3;
		else
//		    trans=(256-dy)*0.5;
		    trans=(256-dy) / 2; // JCL - changed to fix an annoying warning (don't think it worked anyway...)
*/
/*
		trans=128-(dy>>2);
		if(trans<1)
			trans=1;
*/

		if(dy>500-(13*16))
		{
			trans=(500-dy)>>4;
		}
		if(dy<13*4)
		{
			trans=dy>>2;
		}
		else
		{
			trans=13;
		}


		dx+=x;
		dy+=y;
		dz+=z;


		if(trans>=1)
		{
//			trans*=0x010101;
			trans |= trans << 8;
			trans |= trans << 8;

			SPRITE_draw_tex(
				float(dx),
				float(dy),
				float(dz),
				float(((dy-y)>>2)+150),
				trans,
				0,
				POLY_PAGE_STEAM,
				u,v,0.5,0.5,
				SPRITE_SORT_NORMAL);
		}
	}
}


UBYTE fire_pal[768];

void    init_flames()
{
	MFFileHandle	handle	=	FILE_OPEN_ERROR;

	handle=FileOpen("data\\flames1.pal");

	if(handle!=FILE_OPEN_ERROR)
	{
		FileRead(handle,(UBYTE*)&fire_pal,768);

		FileClose(handle);
	} else TRACE("Missing flame palette file.\n");
}

extern	int AENG_detail_skyline;

void	draw_flames(SLONG x,SLONG y,SLONG z,SLONG lod,SLONG offset)
{
	SLONG	c0;
	SLONG	trans;
	SLONG   page;
	float   scale;
	float   u,v,w,h;
	UBYTE*  palptr;
	SLONG   palndx;
	float   wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4;
	UBYTE   type;
	//
	// waft gently up from x,y,z
	//

	steam_seed=54321678+offset;

#if 0
	// Now moved to the actual callers instead.
	SLONG lod_mul;

#ifndef TARGET_DC
	extern UBYTE sw_hack;

	if (sw_hack || !AENG_detail_skyline)//||ShiftFlag)
#else
	if (!AENG_detail_skyline)//||ShiftFlag)
#endif
	{
		lod_mul = 2;
	}
	else
	{
		lod_mul = 5;
	}
#endif

	//for(c0=0;c0<lod*lod_mul;c0++)
	for(c0=0;c0<lod;c0++)
	{
		SLONG	dx,dy,dz;

		w=h=1.0;
		u=v=0.0;

		type=c0&3;

/*		if (!type) {
			page=POLY_PAGE_SMOKECLOUD2;
		} else */
		{
//			page=POLY_PAGE_FLAMES2;
			if ((type==1)||(type==2))
				page=POLY_PAGE_FLAMES2;
			else
				page=POLY_PAGE_PCFLAMER;
			dy=(GAME_TURN+c0)/2;
			u=0.25f*(dy&3);
			v=0.25f*((dy&!3)/3);
			v+=0.002f;
			w=h=0.25f;
			dy=0;
		}

		dy=get_steam_rand()&0x1ff;
		dy+=(GAME_TURN*5);
		dy%=500;
		dx=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;
		dz=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;
		if (type==2) {
			dx>>=2; dz>>=2;
		}
		if (type==1) {
			dx>>=1; dz>>=1;
		}

		wy1=wy2=wy3=wy4=0;
		wx1=(float)COS(((dy+c0)*11)&1023)*0.0001f;
		wx2=(float)COS(((dy+c0)*10)&1023)*0.0001f;
		wx3=(float)COS(((dy+c0)*8)&1023)*0.0001f;
		wx4=(float)COS(((dy+c0)*9)&1023)*0.0001f;
		

		trans=255-dy;

		dx+=x;
		dy+=y;
		dz+=z;


		if(trans>=1)
		{
			switch (type) {
/*			  case 0: // smoke
				trans+=(trans<<8)|(trans<<16)|(trans<<24);
				scale=float(((dy-y)>>1)+80);
				dy+=50;
			    palptr=(trans*3)+fire_pal;
				palndx=(256-trans)*3;
				trans<<=24;
				trans+=(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];*/
//				break;
			  case 1: // outer
				dy>>=1;
			  case 2: // innter
			    palptr=(trans*3)+fire_pal;
				palndx=(256-trans)*3;
//				trans=0xff;
				trans<<=24;
//				trans+=0xffffff;
//				trans+=((*palptr++)<<8)|((*palptr++)<<16)|((*palptr++)<<24);
				trans+=(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];
				//scale=(dy>>2)+50;
				scale=50;
				break;
			  case 0:
			  case 3: // flame
				trans=abs(dy-y)*20;
				trans=SIN(trans);
				trans|=0x00FFFFFF;
				scale=100;
				dy=y;
				break;
			}

//			SPRITE_draw_tex(
#ifdef TARGET_DC
			SPRITE_draw_tex_distorted_params Params;
#define SET_IN_PARAMS(argname) Params.argname = argname
			SET_IN_PARAMS(u);
			SET_IN_PARAMS(v);
			SET_IN_PARAMS(w);
			SET_IN_PARAMS(h);
			SET_IN_PARAMS(wx1);
			SET_IN_PARAMS(wy1);
			SET_IN_PARAMS(wx2);
			SET_IN_PARAMS(wy2);
			SET_IN_PARAMS(wx3);
			SET_IN_PARAMS(wy3);
			SET_IN_PARAMS(wx4);
			SET_IN_PARAMS(wy4);
#undef SET_IN_PARAMS

			SPRITE_draw_tex_distorted(
				float(dx),
				float(dy),
				float(dz),
				float(scale),
				trans,
				0xff000000,
				page,
				SPRITE_SORT_NORMAL,
				&Params);

#else //#ifdef TARGET_DC
			SPRITE_draw_tex_distorted(
				float(dx),
				float(dy),
				float(dz),
				float(scale),
				trans,
				0xff000000,
				page,
				u, v, w, h,
				wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4,
				SPRITE_SORT_NORMAL);
#endif //#else //#ifdef TARGET_DC

		}
	}
}

#ifndef	PSX
void	draw_flame_element(SLONG x,SLONG y,SLONG z,SLONG c0,UBYTE base,UBYTE rand)
{
//	SLONG	c0;
	SLONG	trans;
	SLONG   page;
	float   scale;
	float   u,v,w,h;
	UBYTE*  palptr;
	SLONG   palndx;
	float   wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4;
	SLONG	dx,dy,dz;
	//
	// waft gently up from x,y,z
	//

	if (rand)
		steam_seed=54321678+(c0*get_steam_rand());
	else
		steam_seed=54321678+c0;


		w=h=1.0;
		u=v=0.0;

		if(!(c0&3))
			page=POLY_PAGE_FLAMES;
		else
		  if (c0&4)
			page=POLY_PAGE_SMOKE;
		  else {
			  if (!base) {
				page=POLY_PAGE_FLAMES;
			  } else {
				page=POLY_PAGE_FLAMES2;
				dy=(GAME_TURN+c0)/2;
				u=0.25f*(dy&3);
//				v=0.25f*((dy&~3)/4);
				v=0.25f*((dy>>2)&3);
				w=h=0.25f;
			  }
		  }

		dy=get_steam_rand()&0x1ff;
		dy+=(GAME_TURN*5);
		dy%=500;
		dx=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;
		dz=(((get_steam_rand()&0xff)-128)*((dy>>2)+150))>>9;
		if (page==POLY_PAGE_FLAMES) {
			dx>>=2; dz>>=2;
		}

		wx1=wx2=wx3=wx4=0;
		wy1=wy2=wy3=wy4=0;
/*		wx1=COS(((dy+c0)*11)%1024)*0.0001;
		wx2=COS(((dy+c0)*10)%1024)*0.0001;
		wx3=COS(((dy+c0)*8)%1024)*0.0001;
		wx4=COS(((dy+c0)*9)%1024)*0.0001;
*/
		wx1=(float)COS(((dy+c0)*11)&1023)*0.0001f;
		wx2=(float)COS(((dy+c0)*10)&1023)*0.0001f;
		wx3=(float)COS(((dy+c0)*8)&1023)*0.0001f;
		wx4=(float)COS(((dy+c0)*9)&1023)*0.0001f;

		trans=255-dy;

		dx+=x;
		dy+=y;
		dz+=z;


		if(trans>=1)
		{
			switch (page) {
			  case POLY_PAGE_FLAMES:
			    palptr=(trans*3)+fire_pal;
				palndx=(256-trans)*3;
				trans<<=24;
				trans+=(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];
				scale=50;
				break;
			  case POLY_PAGE_FLAMES2:
//				trans=0xFFFFFFFF;
			    palptr=(trans*3)+fire_pal;
				palndx=(256-trans)*3;
				trans<<=24;
				trans+=(fire_pal[palndx]<<16)+(fire_pal[palndx+1]<<8)+fire_pal[palndx+2];
				scale=100;
				dy=50;
				break;
			  case POLY_PAGE_SMOKE:
				trans+=(trans<<8)|(trans<<16)|(trans<<24);
				scale=float(((dy-y)>>1)+50);
				dy+=50;
				break;
			}

#ifdef TARGET_DC
			SPRITE_draw_tex_distorted_params Params;
#define SET_IN_PARAMS(argname) Params.argname = argname
			SET_IN_PARAMS(u);
			SET_IN_PARAMS(v);
			SET_IN_PARAMS(w);
			SET_IN_PARAMS(h);
			SET_IN_PARAMS(wx1);
			SET_IN_PARAMS(wy1);
			SET_IN_PARAMS(wx2);
			SET_IN_PARAMS(wy2);
			SET_IN_PARAMS(wx3);
			SET_IN_PARAMS(wy3);
			SET_IN_PARAMS(wx4);
			SET_IN_PARAMS(wy4);
#undef SET_IN_PARAMS

			SPRITE_draw_tex_distorted(
				float(dx),
				float(dy),
				float(dz),
				float(scale),
				trans,
				0xff000000,
				page,
				SPRITE_SORT_NORMAL,
				&Params);

#else //#ifdef TARGET_DC

			SPRITE_draw_tex_distorted(
				float(dx),
				float(dy),
				float(dz),
				float(scale),
				trans,
				0xff000000,
				page,
				u, v, w, h,
				wx1,wy1,wx2,wy2,wx3,wy3,wx4,wy4,
				SPRITE_SORT_NORMAL);
#endif //#else //#ifdef TARGET_DC
		}
	
}
#endif



//void FIGURE_rotate_obj2(SLONG matrix[9], SLONG yaw, SLONG pitch, SLONG roll)
void FIGURE_rotate_obj2(SLONG pitch,SLONG yaw,SLONG roll, Matrix33 *r3) 
{
	SLONG cy, cp, cr;
	SLONG sy, sp, sr;

	cy = COS(yaw   & 2047);
	cp = COS(pitch & 2047);
	cr = COS(roll  & 2047);
	
	sy = SIN(yaw   & 2047);
	sp = SIN(pitch & 2047);
	sr = SIN(roll  & 2047);

	//
	// Jan I trust you... but only becuase I've already seen it working!
	//

	r3->M[0][0]= (MUL64( cy, cr) + MUL64(MUL64(sy, sp), sr))>>1;
	r3->M[0][1]= (MUL64( cy, sr) - MUL64(MUL64(sy, sp), cr))>>1;
	r3->M[0][2]= (MUL64( sy, cp))>>1;
	r3->M[1][0]= (MUL64(-cp, sr))>>1;
	r3->M[1][1]= (MUL64( cp, cr))>>1;
	r3->M[1][2]= (sp)>>1;
	r3->M[2][0]= (MUL64(-sy, cr) + MUL64(MUL64(cy, sp), sr))>>1;
	r3->M[2][1]= (MUL64(-sy, sr) - MUL64(MUL64(cy, sp), cr))>>1;
	r3->M[2][2]= (MUL64( cy, cp))>>1;
}

void FIGURE_rotate_obj(SLONG xangle,SLONG yangle,SLONG zangle, Matrix33 *r3) 
{
	SLONG	sinx, cosx, siny, cosy, sinz, cosz;
 	SLONG	cxcz,sysz,sxsycz,sxsysz,sysx,cxczsy,sxsz,cxsysz,czsx,cxsy,sycz,cxsz;

	FIGURE_rotate_obj2(xangle,yangle,zangle,r3);
	return;

	sinx = SIN(xangle & (2048-1)) >>1;  	// 15's
	cosx = COS(xangle & (2048-1)) >>1;
	siny = SIN(yangle & (2048-1)) >>1;
	cosy = COS(yangle & (2048-1)) >>1;
	sinz = SIN(zangle & (2048-1)) >>1;
	cosz = COS(zangle & (2048-1)) >>1;

	cxsy    = (cosx*cosy);				  		// 30's
	sycz    = (cosy*cosz);
	cxcz	= (cosx*cosz);
	cxsz	= (cosx*sinz);
	czsx	= (cosz*sinx);
	sysx    = (cosy*sinx);
	sysz	= (cosy*sinz);
	sxsz 	= (sinx*sinz);
	sxsysz  = (sxsz>>15)*siny;
	cxczsy	= (cxcz>>15)*siny;
	cxsysz  = ((cosx*siny)>>15)*sinz;
	sxsycz  = (czsx>>15)*siny;

	// Define rotation matrix r3.

	r3->M[0][0] = (sycz)>>15;						// 14's
	r3->M[0][1] = (-sysz)>>15;
	r3->M[0][2] = siny;
	r3->M[1][0] = (sxsycz+cxsz)>>15;
	r3->M[1][1] = (cxcz-sxsysz)>>15;
	r3->M[1][2] = (-sysx)>>15;
	r3->M[2][0] = (sxsz-cxczsy)>>15;
	r3->M[2][1] = (cxsysz+czsx)>>15;
	r3->M[2][2] = (cxsy)>>15;
}


//UBYTE	store_dprod[500];
extern	UBYTE TEXTURE_dontexist[];

// 0->3
// 4->7
// 8->11
// 12->15

UWORD	jacket_lookup[4][8]=
{
	{64+21,10*64+2,10*64+2,10*64+32},
	{64+22,10*64+3,10*64+3,10*64+33},
	{64+24,10*64+4,10*64+4,10*64+36},
	{64+25,10*64+5,10*64+5,10*64+37}
	
};



#if USE_TOMS_ENGINE_PLEASE_BOB


// A huge number!!!!
#define MAX_NUMBER_D3D_PRIMS MAX_PRIM_OBJECTS
TomsPrimObject D3DObj[MAX_NUMBER_D3D_PRIMS];
float m_fObjectBoundingSphereRadius[MAX_NUMBER_D3D_PRIMS];


// 16 people types, 32 types of civ, and 6 different Roper ones (five weapons + no weapon).
// Oh - number eight is his magic hip-flask!
#define NUM_ROPERS_THINGIES 8
#define MAX_NUMBER_D3D_PEOPLE (32 + 16 + NUM_ROPERS_THINGIES)
TomsPrimObject D3DPeopleObj[MAX_NUMBER_D3D_PEOPLE];


// iFaceNum: the face number
// bTri: TRUE if it's a tri, FALSE if it's a quad.
UWORD FIGURE_find_face_D3D_texture_page ( int iFaceNum, bool bTri )
{
	// Find the texture page/shading info.
	UWORD wTexturePage;
	DWORD dwDrawFlags, dwFaceFlags;
	if ( bTri )
	{
		PrimFace3 *p_f3 = &prim_faces3[iFaceNum];
		dwDrawFlags = p_f3->DrawFlags;
		dwFaceFlags = p_f3->FaceFlags;

		wTexturePage   = p_f3->UV[0][0] & 0xc0;
		wTexturePage <<= 2;
		wTexturePage  |= p_f3->TexturePage;
	}
	else
	{
		PrimFace4 *p_f4 = &prim_faces4[iFaceNum];
		dwDrawFlags = p_f4->DrawFlags;
		dwFaceFlags = p_f4->FaceFlags;

		wTexturePage   = p_f4->UV[0][0] & 0xc0;
		wTexturePage <<= 2;
		wTexturePage  |= p_f4->TexturePage;
	}

	if ( ( dwDrawFlags & POLY_FLAG_TEXTURED ) == 0 )
	{
		// Untextured.
		wTexturePage = TEXTURE_PAGE_FLAG_NOT_TEXTURED | POLY_PAGE_COLOUR;
	}
	else
	{
		// Textured.
		if(dwFaceFlags&FACE_FLAG_THUG_JACKET)
		{
			switch(wTexturePage)
			{
				case	64+21:
				case	10*64+2:
				case	10*64+32:
					wTexturePage = 0 | TEXTURE_PAGE_FLAG_JACKET;
					//page=jacket_lookup[0][GET_SKILL(p_thing)>>2];
					break;
				case	64+22:
				case	10*64+3:
				case	10*64+33:
					wTexturePage = 1 | TEXTURE_PAGE_FLAG_JACKET;
					//page=jacket_lookup[1][GET_SKILL(p_thing)>>2];
					break;
				case	64+24:
				case	10*64+4:
				case	10*64+36:
					wTexturePage = 2 | TEXTURE_PAGE_FLAG_JACKET;
					//page=jacket_lookup[2][GET_SKILL(p_thing)>>2];
					break;
				case	64+25:
				case	10*64+5:
				case	10*64+37:
					wTexturePage = 3 | TEXTURE_PAGE_FLAG_JACKET;
					//page=jacket_lookup[3][GET_SKILL(p_thing)>>2];
					break;
				default:
					ASSERT(FALSE);
					break;
			}
			//page+=FACE_PAGE_OFFSET;
		}
		else if ( ( wTexturePage > 10*64 ) && ( alt_texture[wTexturePage-10*64] != 0 ) )
		{
			// An "offset" texture. This will be offset by a certain amount to
			// allow each prim to have different coloured clothes on.
			// Store the original value in the page number, because if tex_offset is zero
			// at runtime, we just add on FACE_PAGE_OFFSET.
			ASSERT ( ( wTexturePage & ~TEXTURE_PAGE_MASK ) == 0 );

			wTexturePage |= TEXTURE_PAGE_FLAG_OFFSET;
		}
		else
		{
			wTexturePage += FACE_PAGE_OFFSET;
			// Make sure it actually fits in the available bit space.
			ASSERT ( ( wTexturePage & ~TEXTURE_PAGE_MASK ) == 0 );
		}
	}

	if (dwFaceFlags & FACE_FLAG_TINT)
	{
		wTexturePage |= TEXTURE_PAGE_FLAG_TINT;
	}

	return ( wTexturePage );

}




// The LRU queue for the prims.
// Psycho Park is the most hungry.
// Actually, Day of Reckoning is pretty bad too.

// Max length of actual queue in TomsPrimObjects
// MUST BE LESS THAN 256!
#define PRIM_LRU_QUEUE_LENGTH 250
// Max size in number of vertices in queue.
#define PRIM_LRU_QUEUE_SIZE 6000

int m_iLRUQueueSize = 0;
TomsPrimObject *ptpoLRUQueue[PRIM_LRU_QUEUE_LENGTH];
DWORD dwGameTurnLastUsed[PRIM_LRU_QUEUE_LENGTH];

// Size of the queue in vertices.
DWORD m_dwSizeOfQueue = 0;

// "Cleans" a slot.
void FIGURE_clean_LRU_slot ( int iSlot )
{
	TomsPrimObject *ptpo = ptpoLRUQueue[iSlot];

	ASSERT ( ptpo != NULL );
	if ( ptpo == NULL )
	{
		// Toast aversion. This will cause it to leak memory,
		// but at least it won't die immediately.
		return;
	}

	m_dwSizeOfQueue -= (DWORD)ptpo->wTotalSizeOfObj;
	// Make sure it doesn't go negative!
	ASSERT ( ( m_dwSizeOfQueue & 0x80000000 ) == 0 );

	ASSERT ( ptpo != NULL );
	ASSERT ( ptpo->bLRUQueueNumber == iSlot );
	ASSERT ( ptpo->wNumMaterials > 0 );
	ASSERT ( ptpo->pMaterials != NULL );
	ASSERT ( ptpo->pwListIndices != NULL );
	MemFree ( ptpo->pMaterials );
	// This block include pwStripIndices and pD3DVertices.
	MemFree ( ptpo->pwListIndices );

	ptpo->pMaterials = NULL;
	ptpo->pwListIndices = NULL;
	ptpo->pD3DVertices = NULL;
	ptpo->pwStripIndices = NULL;
	ptpo->bLRUQueueNumber = 0xff;
	ptpo->wNumMaterials = 0;
	ptpo->wTotalSizeOfObj = 0;

	ptpoLRUQueue[iSlot] = NULL;
}


// This is suitable for calling at end of level.
void FIGURE_clean_all_LRU_slots ( void )
{
	for ( int i = 0; i < m_iLRUQueueSize; i++ )
	{
		if ( ptpoLRUQueue[i] != NULL )
		{
			FIGURE_clean_LRU_slot ( i );
		}
	}
	// And zero-length the queue.
	m_iLRUQueueSize = 0;

	// Make sure the queue is actually zero length,
	// or some accounting has screwed up somewhere.
	ASSERT ( m_dwSizeOfQueue == 0 );
}


#ifdef DEBUG
int g_iCacheReplacements = 0;
bool g_bCacheReplacementThrash = FALSE;
#endif

// Adds the primobj to the LRU queue somewhere, evicting something else if need be.
void FIGURE_find_and_clean_prim_queue_item ( TomsPrimObject *pPrimObj, int iThrashIndex = 0 )
{
	int iQueuePos;

	// Make sure nothing even gets close to filling all the space - that would be a disaster.
	// If this happens, the queue is far too small.
	// Roper is the biggest so far, at 529 vertices.
	ASSERT ( (DWORD)pPrimObj->wTotalSizeOfObj * 5< PRIM_LRU_QUEUE_SIZE );

	// Silly man!
	ASSERT ( (DWORD)pPrimObj->wTotalSizeOfObj > 0 );


	if ( ( m_iLRUQueueSize < PRIM_LRU_QUEUE_LENGTH ) &&
		 ( ( (DWORD)pPrimObj->wTotalSizeOfObj + m_dwSizeOfQueue ) < PRIM_LRU_QUEUE_SIZE ) )
	{
		// That's easy then.
		iQueuePos = m_iLRUQueueSize;
		m_iLRUQueueSize++;
	}
	else
	{
		// No more free slots, or not enough free "memory" (vertices). Find the oldest one then.

		// First find enough memory.
		while ( ( (DWORD)pPrimObj->wTotalSizeOfObj + m_dwSizeOfQueue ) >= PRIM_LRU_QUEUE_SIZE )
		{
	#ifdef DEBUG
			g_iCacheReplacements++;
	#endif

			DWORD dwMostTurns = 0;
			int iOldestSlot = -1;
			for ( int i = 0; i < m_iLRUQueueSize; i++ )
			{
				if ( ptpoLRUQueue[i] != NULL )
				{
					// Careful of wrap-arounds!
					DWORD dwTurnsAgo = ( GAME_TURN - dwGameTurnLastUsed[i] );
					if ( dwMostTurns <= dwTurnsAgo )
					{
						// This is older.
						dwMostTurns = dwTurnsAgo;
						iOldestSlot = i;
					}
				}
			}
			ASSERT ( ( iOldestSlot > -1 ) && ( iOldestSlot < m_iLRUQueueSize ) );
			if ( dwMostTurns < 3 )
			{
				// Shit - this is probably about to thrash.
				// There is nothing we can do to prevent it in this case really.
				// Could try binning the _newest_ instead, but that's a real pain.
				// Just try not to.
				// iOldestSlot = iThrashIndex;

	#ifdef DEBUG
				// And warn us.
				g_bCacheReplacementThrash = TRUE;
	#endif
			}

			// Clean out this prim & slot.
			FIGURE_clean_LRU_slot ( iOldestSlot );

			iQueuePos = iOldestSlot;
		}


		// Now check there is a queue space free.
		if ( m_iLRUQueueSize >= PRIM_LRU_QUEUE_LENGTH )
		{
			// Nope - bin oldest.
	#ifdef DEBUG
			g_iCacheReplacements++;
	#endif

			DWORD dwMostTurns = 0;
			int iOldestSlot = -1;
			for ( int i = 0; i < m_iLRUQueueSize; i++ )
			{
				if ( ptpoLRUQueue[i] != NULL )
				{
					// Careful of wrap-arounds!
					DWORD dwTurnsAgo = ( GAME_TURN - dwGameTurnLastUsed[i] );
					if ( dwMostTurns <= dwTurnsAgo )
					{
						// This is older.
						dwMostTurns = dwTurnsAgo;
						iOldestSlot = i;
					}
				}
			}
			ASSERT ( ( iOldestSlot > -1 ) && ( iOldestSlot < m_iLRUQueueSize ) );
			if ( dwMostTurns < 3 )
			{
				// Shit - this is probably about to thrash. Prevent too much thrashing by always using the same slot.
				// FIRST CHECK THAT THIS SLOW IS ACTUALLY USED.
				if ( ptpoLRUQueue[iThrashIndex] == NULL )
				{
					// No, it isn't actually used.
					// Allow the thrash then.
				}
				else
				{
					// Yes, it's used - bin it.
					iOldestSlot = iThrashIndex;
				}

	#ifdef DEBUG
				// And warn us.
				g_bCacheReplacementThrash = TRUE;
	#endif
			}


			// In some cases, we may have decided to clean a cache position that
			// isn't ocupied, which will die nicely.
			// In this case, just find the first that is occupied and clean it instead.
			if ( ptpoLRUQueue[iOldestSlot] == NULL )
			{
				// Warn me.
				ASSERT ( FALSE );

				// And recover gracefully...


				// Nads. Find the first used one.
				iOldestSlot = -1;
				for ( int i = 0; i < m_iLRUQueueSize; i++ )
				{
					if ( ptpoLRUQueue[i] != NULL )
					{
						// Found one.
						iOldestSlot = i;
						break;
					}
				}
			}

			// Clean out this prim & slot.
			FIGURE_clean_LRU_slot ( iOldestSlot );

			iQueuePos = iOldestSlot;
		}
	}


	ptpoLRUQueue[iQueuePos] = pPrimObj;
	pPrimObj->bLRUQueueNumber = iQueuePos;
	dwGameTurnLastUsed[iQueuePos] = GAME_TURN;
	m_dwSizeOfQueue += (DWORD)pPrimObj->wTotalSizeOfObj;
	ASSERT ( m_dwSizeOfQueue < PRIM_LRU_QUEUE_SIZE );
}

void FIGURE_touch_LRU_of_object ( TomsPrimObject *pPrimObj )
{
	ASSERT ( ( pPrimObj->bLRUQueueNumber >= 0 ) && ( pPrimObj->bLRUQueueNumber < m_iLRUQueueSize ) );
	ASSERT ( ptpoLRUQueue[pPrimObj->bLRUQueueNumber] == pPrimObj );
	dwGameTurnLastUsed[pPrimObj->bLRUQueueNumber] = GAME_TURN;
}



#ifdef DEBUG
int m_iMaxNumVertsUsed = 0;
int m_iMaxNumIndicesUsed = 0;
#endif




// Used by the FIGURE_TPO lot.
static D3DVERTEX *TPO_pVert = NULL;
static UWORD *TPO_pStripIndices = NULL;
static UWORD *TPO_pListIndices = NULL;
static int *TPO_piVertexRemap = NULL;
static int *TPO_piVertexLinks = NULL;
static D3DVERTEX *TPO_pCurVertex = NULL;
static UWORD *TPO_pCurStripIndex = NULL;
static UWORD *TPO_pCurListIndex = NULL;
static TomsPrimObject *TPO_pPrimObj = NULL;
static int TPO_iNumListIndices = 0;
static int TPO_iNumStripIndices = 0;
static int TPO_iNumVertices = 0;

#define TPO_MAX_NUMBER_PRIMS 16
static int TPO_iNumPrims = 0;
//static PrimObject *TPO_PrimObjects[TPO_MAX_NUMBER_PRIMS];
static SLONG TPO_PrimObjects[TPO_MAX_NUMBER_PRIMS];
static UBYTE TPO_ubPrimObjMMIndex[TPO_MAX_NUMBER_PRIMS];
static int TPO_iPrimObjIndexOffset[TPO_MAX_NUMBER_PRIMS+1];


// Starts off a 3D object.
// iThrashIndex - can be ignored normally.
void FIGURE_TPO_init_3d_object ( TomsPrimObject *pPrimObj /*, int iThrashIndex = 0 */ )
{
	//PrimObject  *p_obj = &prim_objects[prim];
	//ASSERT ( prim < MAX_NUMBER_D3D_PRIMS );


	TRACE("Started conversion 0x%x...", pPrimObj);


	// Make sure we're not in the middle of compiling and object already.
	ASSERT ( TPO_pVert == NULL );
	ASSERT ( TPO_pStripIndices == NULL );
	ASSERT ( TPO_pListIndices == NULL );
	ASSERT ( TPO_piVertexRemap == NULL );
	ASSERT ( TPO_piVertexLinks == NULL );
	ASSERT ( TPO_pCurVertex == NULL );
	ASSERT ( TPO_pCurStripIndex == NULL );
	ASSERT ( TPO_pCurListIndex == NULL );
	ASSERT ( TPO_pPrimObj == NULL );
	ASSERT ( TPO_iNumListIndices == 0 );
	ASSERT ( TPO_iNumStripIndices == 0 );
	ASSERT ( TPO_iNumVertices == 0 );
	ASSERT ( TPO_iNumPrims == 0 );


	// Temporary space that gets freed at the end.
#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB

#define MAX_VERTS 4096
#define MAX_INDICES (MAX_VERTS * 4)

#else

#define MAX_VERTS 1024
#define MAX_INDICES (MAX_VERTS * 4)

#endif

	TPO_pVert = (D3DVERTEX *)MemAlloc ( MAX_VERTS * sizeof ( D3DVERTEX ) );
	ASSERT ( TPO_pVert != NULL );
	if ( TPO_pVert == NULL ) { DeadAndBuried ( 0xf800f800 ); }
	TPO_pStripIndices = (UWORD *)MemAlloc ( MAX_INDICES * sizeof ( UWORD ) );
	ASSERT ( TPO_pStripIndices != NULL );
	if ( TPO_pStripIndices == NULL ) { DeadAndBuried ( 0xf800f800 ); }
	TPO_pListIndices = (UWORD *)MemAlloc ( MAX_INDICES * sizeof ( UWORD ) );
	ASSERT ( TPO_pListIndices != NULL );
	if ( TPO_pListIndices == NULL ) { DeadAndBuried ( 0xf800f800 ); }

	TPO_piVertexRemap = (int *)MemAlloc ( MAX_VERTS * sizeof ( int ) );
	ASSERT ( TPO_piVertexRemap != NULL );
	if ( TPO_piVertexRemap == NULL ) { DeadAndBuried ( 0xf800f800 ); }
	TPO_piVertexLinks = (int *)MemAlloc ( MAX_VERTS * sizeof ( int ) );
	ASSERT ( TPO_piVertexLinks != NULL );
	if ( TPO_piVertexLinks == NULL ) { DeadAndBuried ( 0xf800f800 ); }

	TPO_pCurVertex = TPO_pVert;
	TPO_pCurStripIndex = TPO_pStripIndices;
	TPO_pCurListIndex = TPO_pListIndices;


	//TomsPrimObject *pPrimObj = &(D3DObj[prim]);
	// Make sure it's not been initialised.
	ASSERT ( pPrimObj->pD3DVertices == NULL );
	ASSERT ( pPrimObj->pMaterials == NULL );
	ASSERT ( pPrimObj->pwListIndices == NULL );
	ASSERT ( pPrimObj->pwStripIndices == NULL );
	ASSERT ( pPrimObj->wNumMaterials == 0 );

	TPO_pPrimObj = pPrimObj;

#if 0
// Do this after the object has been compiled, so we know how big it is.
	// Add this to a good place in the LRU queue.
	FIGURE_find_and_clean_prim_queue_item ( pPrimObj, iThrashIndex );
#endif

	// No index offset for the first object
	TPO_iPrimObjIndexOffset[0] = 0;


	pPrimObj->wFlags = 0;
	pPrimObj->fBoundingSphereRadius = 0.0f;

	TPO_iNumListIndices = 0;
	TPO_iNumStripIndices = 0;
	TPO_iNumVertices = 0;
	TPO_iNumPrims = 0;

	// Ready for action.

}


// Add a prim to this 3D object.
// prim = the prim number to add.
// iSubObjectNumber = sub-object number, used for multimatrix stuff.
//		Start at 0 and go up by one for each object.
//void FIGURE_TPO_add_prim_to_current_object ( PrimObject  *p_obj, UBYTE ubSubObjectNumber )
void FIGURE_TPO_add_prim_to_current_object ( SLONG prim, UBYTE ubSubObjectNumber )
{
	// Make sure it's been properly prepped.
	ASSERT ( TPO_pVert != NULL );
	ASSERT ( TPO_pStripIndices != NULL );
	ASSERT ( TPO_pListIndices != NULL );
	ASSERT ( TPO_piVertexRemap != NULL );
	ASSERT ( TPO_piVertexLinks != NULL );
	ASSERT ( TPO_pCurVertex != NULL );
	ASSERT ( TPO_pCurStripIndex != NULL );
	ASSERT ( TPO_pCurListIndex != NULL );
	ASSERT ( TPO_pPrimObj != NULL );

	ASSERT ( prim < MAX_NUMBER_D3D_PRIMS );
	PrimObject  *p_obj = &prim_objects[prim];
	ASSERT ( p_obj != NULL );
	TRACE("added prim 0x%x...", p_obj);

	TPO_PrimObjects[TPO_iNumPrims] = prim;
	TPO_ubPrimObjMMIndex[TPO_iNumPrims] = ubSubObjectNumber;


	// Work out what index offset to add to the _next_ object.
	// It's this object's offset plus the number of points
	// in this object.
	TPO_iPrimObjIndexOffset[TPO_iNumPrims+1] = TPO_iPrimObjIndexOffset[TPO_iNumPrims] + ( p_obj->EndPoint - p_obj->StartPoint );

	TPO_iNumPrims++;
	ASSERT ( TPO_iNumPrims <= TPO_MAX_NUMBER_PRIMS );

}




// Compile the object - this actually does all the work.
// pPrimObj the prim obj that was passed to FIGURE_TPO_init_3d_object 
// iThrashIndex - can be ignored normally.
void FIGURE_TPO_finish_3d_object ( TomsPrimObject *pPrimObj, int iThrashIndex = 0  )
{
	// Make sure one has actually been started.
	ASSERT ( TPO_pVert != NULL );
	ASSERT ( TPO_pStripIndices != NULL );
	ASSERT ( TPO_pListIndices != NULL );
	ASSERT ( TPO_piVertexRemap != NULL );
	ASSERT ( TPO_piVertexLinks != NULL );
	ASSERT ( TPO_pCurVertex != NULL );
	ASSERT ( TPO_pCurStripIndex != NULL );
	ASSERT ( TPO_pCurListIndex != NULL );
	ASSERT ( TPO_pPrimObj != NULL );

	ASSERT ( pPrimObj == TPO_pPrimObj );


	TRACE ( "compiling %i prims...", TPO_iNumPrims );

	// OK, now scan through the faces. Check to see if the face uses a new material.
	// If so, create this new material and add in this face and all subsequent faces
	// that use it.
	// If not, then this face should already have been added, so ignore it.
	//
	// This sequence is, of course, complicated by there being both quads and tris. :-(

	for ( int iOuterPrimNumber = 0; iOuterPrimNumber < TPO_iNumPrims; iOuterPrimNumber++ )
	{
		PrimObject  *pOuterObj = &prim_objects[TPO_PrimObjects[iOuterPrimNumber]];

		ASSERT ( pOuterObj != NULL );

		bool bOuterTris = FALSE;
		do
		{
			int iOuterFaceNum;
			int iOuterFaceEnd;
			if ( bOuterTris )
			{
				iOuterFaceNum = pOuterObj->StartFace3;
				iOuterFaceEnd = pOuterObj->EndFace3;
			}
			else
			{
				iOuterFaceNum = pOuterObj->StartFace4;
				iOuterFaceEnd = pOuterObj->EndFace4;
			}

			for ( ; iOuterFaceNum < iOuterFaceEnd; iOuterFaceNum++ )
			{
				UWORD wTexturePage = FIGURE_find_face_D3D_texture_page ( iOuterFaceNum, bOuterTris );

				// Find the _rendered_ page, to allow texture paging to do its stuff.
				UWORD wRealPage = wTexturePage & TEXTURE_PAGE_MASK;

				PolyPage *pRenderedPage = NULL;

				#ifdef TEX_EMBED

				if ( ( wTexturePage & ~TEXTURE_PAGE_MASK ) != 0 )
				{
					// Something special about this page, e.g. jacket, special shading, etc.
					// Don't try to combine them.
					pRenderedPage = NULL;
				}
				else
				{
					pRenderedPage = POLY_Page[wTexturePage & TEXTURE_PAGE_MASK].pTheRealPolyPage;
				}

				#endif

				// OK, now we have a material description in wTexturePage.
				// Look for an existing material with this.
				PrimObjectMaterial *pMaterial = pPrimObj->pMaterials;
				int iMatNum;
				for ( iMatNum = pPrimObj->wNumMaterials; iMatNum > 0; iMatNum-- )
				{
	#if 1
	#ifdef TEX_EMBED
					if ( pRenderedPage != NULL )
					{
						if ( ( pMaterial->wTexturePage & ~TEXTURE_PAGE_MASK ) == 0 )
						{
							// Nothing special about this page.
							if ( pRenderedPage == ( POLY_Page[pMaterial->wTexturePage & TEXTURE_PAGE_MASK].pTheRealPolyPage ) )
							{
								// The textures match!
								break;
							}
						}
					}
	#endif
	#endif

					if ( pMaterial->wTexturePage == wTexturePage )
					{
						// Exactly the same page.
						break;
					}
					pMaterial++;
				}
				if ( iMatNum != 0 )
				{
					// Found one. So this face should be in it somewhere already. Ignore.
				}
				else
				{
					// Didn't find this material, so add it.

					pPrimObj->wNumMaterials++;
					// Yes, this is a horrible way to do things - memory fragmented all over the place. Tough. Deal with it.
					// And yes, I could use realloc(), but it doesn't seem to be very happy on the DC, so I won't.
					void *pOldMats = (void *)pPrimObj->pMaterials;
					pPrimObj->pMaterials = (PrimObjectMaterial *)MemAlloc ( pPrimObj->wNumMaterials * sizeof ( *pMaterial ) );
					ASSERT ( pPrimObj->pMaterials != NULL );
					if ( pPrimObj->pMaterials == NULL ) { DeadAndBuried ( 0x001f001f ); }

					if ( pOldMats != NULL )
					{
						memcpy ( pPrimObj->pMaterials, pOldMats, ( pPrimObj->wNumMaterials - 1 ) * sizeof ( *pMaterial ) );
						MemFree ( pOldMats );
					}

					pMaterial = pPrimObj->pMaterials + pPrimObj->wNumMaterials - 1;
					pMaterial->wNumListIndices = 0;
					pMaterial->wNumStripIndices = 0;
					pMaterial->wNumVertices = 0;
					pMaterial->wTexturePage = wTexturePage;

					D3DVERTEX *pFirstVertex = TPO_pCurVertex;

					WORD *pFirstListIndex = TPO_pCurListIndex;
					WORD *pFirstStripIndex = TPO_pCurStripIndex;

					// Renit the vertex remapping.
					for ( int i = 0; i < MAX_VERTS; i++ )
					{
						// The first D3Dvertex that has the right posnorm.
						TPO_piVertexRemap[i] = -1;
						// A possible next vertex that uses this posnorm.
						TPO_piVertexLinks[i] = -1;
					}


					// Now scan all the rest of the faces and add any that have this page.
					// Just scan from this face onwards.

					for ( int iInnerPrimNumber = iOuterPrimNumber; iInnerPrimNumber < TPO_iNumPrims; iInnerPrimNumber++ )
					{
						PrimObject  *pInnerObj = &prim_objects[TPO_PrimObjects[iInnerPrimNumber]];

						float *pfBoundingSphereRadius = &(m_fObjectBoundingSphereRadius[TPO_PrimObjects[iInnerPrimNumber]]);
						*pfBoundingSphereRadius = 0.0f;

						ASSERT ( pInnerObj != NULL );

						bool bInnerTris;
						if ( iInnerPrimNumber == iOuterPrimNumber )
						{
							// Just scan this object from where the outer one is.
							bInnerTris = bOuterTris;
						}
						else
						{
							bInnerTris = FALSE;
						}

						do
						{
							int iInnerFaceEnd;
							int iInnerFaceNum;
							if ( iInnerPrimNumber == iOuterPrimNumber )
							{
								// Just scan from where we are onwards
								if ( bInnerTris )
								{
									if ( bOuterTris )
									{
										iInnerFaceNum = iOuterFaceNum;
									}
									else
									{
										iInnerFaceNum = pInnerObj->StartFace3;
									}
									iInnerFaceEnd = pInnerObj->EndFace3;
								}
								else
								{
									if ( bOuterTris )
									{
										// If the outer loop was scanning tris,
										// it did so after scanning all quads,
										// so why are we scanning quads in the inner loop?
										ASSERT ( FALSE );
										iInnerFaceNum = pInnerObj->StartFace4;
									}
									else
									{
										iInnerFaceNum = iOuterFaceNum;
									}
									iInnerFaceEnd = pInnerObj->EndFace4;
								}
							}
							else
							{
								if ( bInnerTris )
								{
									iInnerFaceNum = pInnerObj->StartFace3;
									iInnerFaceEnd = pInnerObj->EndFace3;
								}
								else
								{
									iInnerFaceNum = pInnerObj->StartFace4;
									iInnerFaceEnd = pInnerObj->EndFace4;
								}
							}

							ASSERT ( iInnerFaceNum <= iInnerFaceEnd );
							for ( ; iInnerFaceNum < iInnerFaceEnd; iInnerFaceNum++ )
							{
								UWORD wTexturePage = FIGURE_find_face_D3D_texture_page ( iInnerFaceNum, bInnerTris );

								bool bSamePage = FALSE;

								if ( pMaterial->wTexturePage == wTexturePage )
								{
									// Exactly the same page.
									bSamePage = TRUE;
								}
		#if 1
		#ifdef TEX_EMBED
								else if ( pRenderedPage != NULL )
								{
									//if ( ( ( pMaterial->wTexturePage & ~TEXTURE_PAGE_MASK ) == 0 ) &&
									//	 ( ( wTexturePage & ~TEXTURE_PAGE_MASK ) == 0 )
									if ( ( ( pMaterial->wTexturePage | wTexturePage ) & ~TEXTURE_PAGE_MASK ) == 0 )
									{
										// Nothing special about either page.
										if ( pRenderedPage == ( POLY_Page[wTexturePage & TEXTURE_PAGE_MASK].pTheRealPolyPage ) )
										{
											// The textures match!
											bSamePage = TRUE;
										}
									}
								}
		#endif
		#endif

								if ( bSamePage )
								{
									// This uses this material, so add it.


									// Find the actual polypage, because that will give us UV offsets.
									// NOTE! This means that all thug jackets must have the same offsets * scales.
									// Shouldn't be too hard to arrange.
									// Ditto for offset tetxures.
									UWORD wRealPage = wTexturePage & TEXTURE_PAGE_MASK;
									if ( wTexturePage & TEXTURE_PAGE_FLAG_JACKET )
									{
										// Find the real jacket page.
										// Assume skill of 0
										wRealPage = jacket_lookup [ wRealPage ][0];
										wRealPage += FACE_PAGE_OFFSET;
									}
									else if ( wTexturePage & TEXTURE_PAGE_FLAG_OFFSET )
									{
										// An "offset" texture. This will be offset by a certain amount to
										// allow each prim to have different coloured clothes on.
										// For now, assume no offset.
										wRealPage += FACE_PAGE_OFFSET;
									}

							#ifdef DEBUG
									if ( wTexturePage & TEXTURE_PAGE_FLAG_NOT_TEXTURED )
									{
										ASSERT ( wRealPage == POLY_PAGE_COLOUR );
									}
							#endif

									PolyPage *pa = &(POLY_Page[wRealPage]);



									// Add the vertices.
									int iIndices[4];
									PrimFace3 *p_f3;
									PrimFace4 *p_f4;
									int iVerts;
									if ( bInnerTris )
									{
										p_f3 = &prim_faces3[iInnerFaceNum];
										iVerts = 3;
									}
									else
									{
										p_f4 = &prim_faces4[iInnerFaceNum];
										iVerts = 4;
									}
									for ( int i = 0; i < iVerts; i++ )
									{
										const float fNormScale = 1.0f / 256.0f;

										D3DVERTEX d3dvert;

										int pt;
										if ( bInnerTris )
										{
											pt = p_f3->Points[i];
											d3dvert.dvTU = float(p_f3->UV[i][0] & 0x3f) * (1.0F / 32.0F);
											d3dvert.dvTV = float(p_f3->UV[i][1]       ) * (1.0F / 32.0F);
										}
										else
										{
											pt = p_f4->Points[i];
											d3dvert.dvTU = float(p_f4->UV[i][0] & 0x3f) * (1.0F / 32.0F);
											d3dvert.dvTV = float(p_f4->UV[i][1]       ) * (1.0F / 32.0F);
										}
		#ifdef TEX_EMBED
										// Clamp if they go out of range.
										// This can produce a little bit of distortion, but it's better than nothing.
										if ( d3dvert.dvTU < 0.0f )
										{
											TRACE("Clamped a U coord to 0");
											d3dvert.dvTU = 0.0f;
										}
										else if ( d3dvert.dvTU > 1.0f )
										{
											TRACE("Clamped a U coord to 1");
											d3dvert.dvTU = 1.0f;
										}
										if ( d3dvert.dvTV < 0.0f )
										{
											TRACE("Clamped a V coord to 0");
											d3dvert.dvTV = 0.0f;
										}
										else if ( d3dvert.dvTV > 1.0f )
										{
											TRACE("Clamped a V coord to 1");
											d3dvert.dvTV = 1.0f;
										}

										d3dvert.dvTU = d3dvert.dvTU * pa->m_UScale + pa->m_UOffset;
										d3dvert.dvTV = d3dvert.dvTV * pa->m_VScale + pa->m_VOffset;

										// Now scale by the page offset and scale
		#endif
										d3dvert.dvX = AENG_dx_prim_points[pt].X;
										d3dvert.dvY = AENG_dx_prim_points[pt].Y;
										d3dvert.dvZ = AENG_dx_prim_points[pt].Z;
										d3dvert.dvNX = prim_normal[pt].X * fNormScale;
										d3dvert.dvNY = prim_normal[pt].Y * fNormScale;
										d3dvert.dvNZ = prim_normal[pt].Z * fNormScale;
										// This must be done after setting the rest up, since it blats the 12th byte of the vertex.
										SET_MM_INDEX ( d3dvert, TPO_ubPrimObjMMIndex[iInnerPrimNumber] );

										// Look for this vertex.
										ASSERT ( pt >= pInnerObj->StartPoint );
										ASSERT ( pt < pInnerObj->EndPoint );
										ASSERT ( ( pt - pInnerObj->StartPoint ) < MAX_VERTS );

										if ( ( pt - pInnerObj->StartPoint ) >= MAX_VERTS ) { DeadAndBuried ( 0xffffffff ); }


										int iPtIndex = TPO_iPrimObjIndexOffset[iInnerPrimNumber] + ( pt - pInnerObj->StartPoint );
										int iVertIndex = TPO_piVertexRemap[iPtIndex];
										if ( iVertIndex == -1 )
										{
											// Never-referenced vertex. Add it.
											TPO_piVertexRemap[iPtIndex] = pMaterial->wNumVertices;
											TPO_piVertexLinks[pMaterial->wNumVertices] = -1;

											iIndices[i] = pMaterial->wNumVertices;

											*TPO_pCurVertex = d3dvert;
											TPO_pCurVertex++;
											pMaterial->wNumVertices++;
											TPO_iNumVertices++;
											ASSERT ( TPO_iNumVertices < MAX_VERTS );

											if ( TPO_iNumVertices >= MAX_VERTS ) { DeadAndBuried ( 0xffffffff ); }


		#ifdef DEBUG
											if ( m_iMaxNumVertsUsed < TPO_iNumVertices )
											{
												//ASSERT ( TPO_iNumVertices < 256 );
												m_iMaxNumVertsUsed = TPO_iNumVertices;
											}
		#endif

											// Grow the bounding sphere if need be.

#if 0
											// THIS IS NOT BEING DONE RIGHT!
											float fDistSqu = ( d3dvert.dvX * d3dvert.dvX ) + ( d3dvert.dvY * d3dvert.dvY ) + ( d3dvert.dvZ * d3dvert.dvZ );
											if ( pPrimObj->fBoundingSphereRadius < fDistSqu )
											{
												pPrimObj->fBoundingSphereRadius = fDistSqu;
											}
#else
											float fDistSqu = ( d3dvert.dvX * d3dvert.dvX ) + ( d3dvert.dvY * d3dvert.dvY ) + ( d3dvert.dvZ * d3dvert.dvZ );
											if ( ( *pfBoundingSphereRadius * *pfBoundingSphereRadius ) < fDistSqu )
											{
												*pfBoundingSphereRadius = sqrtf(fDistSqu);
											}
#endif
										}
										else
										{
											// OK, try the links.
											int iLastIndex = iVertIndex;
											while ( iVertIndex != -1 )
											{
												ASSERT ( pFirstVertex[iVertIndex].dvX == d3dvert.dvX );
												ASSERT ( pFirstVertex[iVertIndex].dvY == d3dvert.dvY );
												ASSERT ( pFirstVertex[iVertIndex].dvZ == d3dvert.dvZ );
												ASSERT ( pFirstVertex[iVertIndex].dvNX == d3dvert.dvNX );
												ASSERT ( pFirstVertex[iVertIndex].dvNY == d3dvert.dvNY );
												ASSERT ( pFirstVertex[iVertIndex].dvNZ == d3dvert.dvNZ );
		#define CLOSE_ENOUGH(a,b) ( fabsf ( (a) - (b) ) < 0.00001f )
												if ( CLOSE_ENOUGH ( pFirstVertex[iVertIndex].dvTU, d3dvert.dvTU ) &&
													 CLOSE_ENOUGH ( pFirstVertex[iVertIndex].dvTV, d3dvert.dvTV ) )
												{
													// Yes, this is a match.
													iIndices[i] = iVertIndex;
													break;
												}
												else
												{
													// Nope - try the next one.
													iLastIndex = iVertIndex;
													iVertIndex = TPO_piVertexLinks[iVertIndex];
												}
											}
											if ( iVertIndex == -1 )
											{
												// Didn't find one, so add new one.
												TPO_piVertexLinks[iLastIndex] = pMaterial->wNumVertices;
												TPO_piVertexLinks[pMaterial->wNumVertices] = -1;

												iIndices[i] = pMaterial->wNumVertices;

												*TPO_pCurVertex = d3dvert;
												TPO_pCurVertex++;
												pMaterial->wNumVertices++;
												TPO_iNumVertices++;
												ASSERT ( TPO_iNumVertices < MAX_VERTS );

												if ( TPO_iNumVertices >= MAX_VERTS ) { DeadAndBuried ( 0xffffffff ); }

		#ifdef DEBUG
												if ( m_iMaxNumVertsUsed < TPO_iNumVertices )
												{
													//ASSERT ( TPO_iNumVertices < 256 );
													m_iMaxNumVertsUsed = TPO_iNumVertices;
												}
		#endif
											}
										}
									}

									// Now add the indices.
									// This is a braindead method - we will optimise it afterwards.
									if ( bInnerTris )
									{
										ASSERT ( ( iIndices[0] >= 0 ) && ( iIndices[0] < pMaterial->wNumVertices ) );
										ASSERT ( ( iIndices[1] >= 0 ) && ( iIndices[1] < pMaterial->wNumVertices ) );
										ASSERT ( ( iIndices[2] >= 0 ) && ( iIndices[2] < pMaterial->wNumVertices ) );

		#if 0
										// Strip indices.
										*TPO_pCurStripIndex++ = iIndices[0];
										*TPO_pCurStripIndex++ = iIndices[1];
										*TPO_pCurStripIndex++ = iIndices[2];
										*TPO_pCurStripIndex++ = -1;
										TPO_iNumStripIndices += 4;
										pMaterial->wNumStripIndices += 4;
										ASSERT ( TPO_iNumStripIndices < MAX_INDICES );

										if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

		#endif
										// List indices.
										*TPO_pCurListIndex++ = iIndices[0];
										*TPO_pCurListIndex++ = iIndices[1];
										*TPO_pCurListIndex++ = iIndices[2];
										TPO_iNumListIndices += 3;
										pMaterial->wNumListIndices += 3;
										ASSERT ( TPO_iNumListIndices < MAX_INDICES );

										if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }


		#ifdef DEBUG
										if ( m_iMaxNumIndicesUsed < TPO_iNumListIndices )
										{
											m_iMaxNumIndicesUsed = TPO_iNumListIndices;
										}
		#endif

									}
									else
									{
										ASSERT ( ( iIndices[0] >= 0 ) && ( iIndices[0] < pMaterial->wNumVertices ) );
										ASSERT ( ( iIndices[1] >= 0 ) && ( iIndices[1] < pMaterial->wNumVertices ) );
										ASSERT ( ( iIndices[2] >= 0 ) && ( iIndices[2] < pMaterial->wNumVertices ) );
										ASSERT ( ( iIndices[3] >= 0 ) && ( iIndices[3] < pMaterial->wNumVertices ) );

		#if 0
										// Strip indices.
										*TPO_pCurStripIndex++ = iIndices[0];
										*TPO_pCurStripIndex++ = iIndices[1];
										*TPO_pCurStripIndex++ = iIndices[2];
										*TPO_pCurStripIndex++ = iIndices[3];
										*TPO_pCurStripIndex++ = -1;
										TPO_iNumStripIndices += 5;
										pMaterial->wNumStripIndices += 5;
										ASSERT ( TPO_iNumStripIndices < MAX_INDICES );

										if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

		#endif
										// List indices.
										*TPO_pCurListIndex++ = iIndices[0];
										*TPO_pCurListIndex++ = iIndices[1];
										*TPO_pCurListIndex++ = iIndices[2];
										*TPO_pCurListIndex++ = iIndices[2];
										*TPO_pCurListIndex++ = iIndices[1];
										*TPO_pCurListIndex++ = iIndices[3];
										TPO_iNumListIndices += 6;
										pMaterial->wNumListIndices += 6;
										ASSERT ( TPO_iNumListIndices < MAX_INDICES );

										if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

		#ifdef DEBUG
										if ( m_iMaxNumIndicesUsed < TPO_iNumListIndices )
										{
											m_iMaxNumIndicesUsed = TPO_iNumListIndices;
										}
		#endif

									}
								}
							}

							bInnerTris = !bInnerTris;
						} while ( bInnerTris );
					}


					// And that's that material fully done.


	#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB

					// Do they need inflating?
					if ( m_bPleaseInflatePeople )
					{

						TRACE("Inflate");
						// Make a list of all edges.

						// At most 3 edges per tri.
						EdgeList *pEdgeList = (EdgeList *)MemAlloc ( sizeof ( EdgeList ) * pMaterial->wNumListIndices );
						ASSERT ( pEdgeList != NULL );
						if ( pEdgeList == NULL ) { DeadAndBuried ( 0x0000001f ); }

						// Add the edges.
						int iNumEdges = 0;
						WORD *pSrcIndex = pFirstListIndex;
						WORD wI1, wI2, wI3, wI4;
						for ( i = pMaterial->wNumListIndices / 3; i > 0; i-- )
						{
							wI1 = pSrcIndex[0];
							wI2 = pSrcIndex[1];
							wI3 = pSrcIndex[2];
							wI4 = pSrcIndex[0];

							for ( int j = 0; j < 3; j++ )
							{
								// Look for edge wI1, wI2.
								bool bFound = FALSE;
								for ( int k = 0; k < iNumEdges; k++ )
								{
									if ( ( ( pEdgeList[k].wPt1 == wI1 ) && ( pEdgeList[k].wPt2 == wI2 ) ) ||
										 ( ( pEdgeList[k].wPt1 == wI2 ) && ( pEdgeList[k].wPt2 == wI1 ) ) )
									{
										// Found the edge.
										bFound = TRUE;
										break;
									}
								}
								if ( !bFound )
								{
									// Make the edge.
									EdgeList *pEdgeCur = &(pEdgeList[iNumEdges]);
									pEdgeCur->wPt1 = wI1;
									pEdgeCur->wPt2 = wI2;
									iNumEdges++;
								}

								// Next edge.
								wI1 = wI2;
								wI2 = wI3;
								wI3 = wI4;
							}

							pSrcIndex += 3;
						}

						// Now scan the edges, creating the midpoints.
						EdgeList *pEdgeCur = pEdgeList;
						for ( i = 0; i < iNumEdges; i++ )
						{
							D3DVERTEX *pvertMid, *pvert1, *pvert2;

							pvert1 = &pFirstVertex[pEdgeCur->wPt1];
							pvert2 = &pFirstVertex[pEdgeCur->wPt2];
							pEdgeCur->wMidPt = pMaterial->wNumVertices;
							pvertMid = TPO_pCurVertex;

							// Make the midpoint.
							D3DVECTOR vPos1, vPos2;
							vPos1.x = pvert1->x;
							vPos1.y = pvert1->y;
							vPos1.z = pvert1->z;
							vPos2.x = pvert2->x;
							vPos2.y = pvert2->y;
							vPos2.z = pvert2->z;
							D3DVECTOR vNorm1, vNorm2;
							vNorm1.x = pvert1->nx;
							vNorm1.y = pvert1->ny;
							vNorm1.z = pvert1->nz;
							vNorm2.x = pvert2->nx;
							vNorm2.y = pvert2->ny;
							vNorm2.z = pvert2->nz;

							// The edge's vector.
							D3DVECTOR vEdge = vPos1 - vPos2;
							D3DVECTOR vAvNorm = vNorm1 + vNorm2;

							float fLenAvNorm = Magnitude ( vAvNorm );
							if ( fLenAvNorm < 0.00001f )
							{
								// Panic.
								vAvNorm = vNorm1;
							}
							else
							{
								vAvNorm *= ( 1.0f / fLenAvNorm );
							}
							
							// Find the normal to the edge.
							D3DVECTOR vNormToEdge = CrossProduct ( vEdge, vAvNorm );
							vNormToEdge = CrossProduct ( vNormToEdge, vEdge );

							float fLenNormToEdge = Magnitude ( vNormToEdge );
							if ( fLenNormToEdge < 0.00001f )
							{
								// Panic.
								vNormToEdge = vNorm1;
							}
							else
							{
								vNormToEdge *= ( 1.0f / fLenNormToEdge );
							}
							
							// Find the new midpoint.
							// Adjust lambda until it's a "nice" value.
							D3DVECTOR vPos3;
							if ( fLenAvNorm < 0.75f * 2.0f )
							{
								// No curve - just make it flat.
								vPos3 = ( vPos1 + vPos2 ) * 0.5;
							}
							else
							{
								// Curve.
								const float fLambda = 0.15f;
								vPos3 = ( vPos1 + vPos2 ) * 0.5 + ( ( fLambda * DotProduct ( ( vNorm1 - vNorm2 ), vEdge ) ) * vAvNorm );
							}

						#if 0
							// And find the new mid normal.
							// This is theoretically more "correct", but tends to give unstable behaviour.
							D3DVECTOR vNorm3 = ( 2 * vNormToEdge ) - vAvNorm;
						#elif 0
							// Much more stable, but produces nasty edges.
							D3DVECTOR vNorm3 = vAvNorm;
						#elif 0
							// A compromise between the two.
							D3DVECTOR vNorm3 = vNormToEdge;
						#else
							// A blend of above, but hopefully more stable.
							D3DVECTOR vNorm3 = ( 4 * vNormToEdge ) - vAvNorm;
						#endif
							
							float fLenNorm3 = Magnitude ( vNorm3 );
							if ( fLenNorm3 < 0.00001f )
							{
								// Panic.
								vNorm3 = vNorm1;
							}
							else
							{
								vNorm3 *= ( 1.0f / fLenNorm3 );
							}

							pvertMid->x = vPos3.x;
							pvertMid->y = vPos3.y;
							pvertMid->z = vPos3.z;
							pvertMid->nx = vNorm3.x;
							pvertMid->ny = vNorm3.y;
							pvertMid->nz = vNorm3.z;

							pvertMid->tu = ( pvert1->tu + pvert2->tu ) * 0.5f;
							pvertMid->tv = ( pvert1->tv + pvert2->tv ) * 0.5f;
							//pvertMid->dif = ( ( ( pvert1->dif & 0xfefefefe ) >> 1 ) + ( ( pvert2->dif & 0xfefefefe ) >> 1 ) );
							//pvertMid->spec = ( ( ( pvert1->spec & 0xfefefefe ) >> 1 ) + ( ( pvert2->spec & 0xfefefefe ) >> 1 ) );

							// This must be done after setting the rest up, since it blats the 12th byte of the vertex.
							// Just pick either of the vertices - in theory they should all be the same.
							SET_MM_INDEX ( (*pvertMid), GET_MM_INDEX(*pvert1) );


							// Add it officially.
							TPO_pCurVertex++;
							pMaterial->wNumVertices++;
							TPO_iNumVertices++;
							ASSERT ( TPO_iNumVertices < MAX_VERTS );

							if ( TPO_iNumVertices >= MAX_VERTS ) { DeadAndBuried ( 0xffffffff ); }

		#ifdef DEBUG
							if ( m_iMaxNumVertsUsed < TPO_iNumVertices )
							{
								//ASSERT ( TPO_iNumVertices < 256 );
								m_iMaxNumVertsUsed = TPO_iNumVertices;
							}
		#endif

							pEdgeCur++;
						}


						// Now scan all the tris, find their edges, and carve them up into 4 tris.
						// If we scan the list backwards, we can use the same list.

						// Add the new indices first, then work out what they are.
						int iNewNumListIndices = TPO_iNumListIndices + 3 * pMaterial->wNumListIndices;
						int iNewMatNumListIndices = 4 * pMaterial->wNumListIndices;
						TPO_pCurListIndex += 3 * pMaterial->wNumListIndices;
						ASSERT ( iNewNumListIndices < MAX_INDICES );

						if ( iNewNumListIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }


		#ifdef DEBUG
						if ( m_iMaxNumIndicesUsed < iNewNumListIndices )
						{
							m_iMaxNumIndicesUsed = iNewNumListIndices;
						}
		#endif


						pSrcIndex = pFirstListIndex + pMaterial->wNumListIndices;
						WORD *pDstIndex = pFirstListIndex + iNewMatNumListIndices;
						WORD wMid[3];
						for ( i = pMaterial->wNumListIndices / 3; i > 0; i-- )
						{
							pSrcIndex -= 3;
							pDstIndex -= 3 * 4;

							wI1 = pSrcIndex[0];
							wI2 = pSrcIndex[1];
							wI3 = pSrcIndex[2];
							wI4 = pSrcIndex[0];
							for ( int j = 0; j < 3; j++ )
							{
								// Look for edge wI1, wI2.
								for ( int k = 0; k < iNumEdges; k++ )
								{
									if ( ( ( pEdgeList[k].wPt1 == wI1 ) && ( pEdgeList[k].wPt2 == wI2 ) ) ||
										 ( ( pEdgeList[k].wPt1 == wI2 ) && ( pEdgeList[k].wPt2 == wI1 ) ) )
									{
										// Found the edge - store the midpoint.
										wMid[j] = pEdgeList[k].wMidPt;
										break;
									}
								}
								ASSERT ( k != iNumEdges );

								// Next edge.
								wI1 = wI2;
								wI2 = wI3;
								wI3 = wI4;
							}

							ASSERT ( wI1 < pMaterial->wNumVertices );
							ASSERT ( wI2 < pMaterial->wNumVertices );
							ASSERT ( wI3 < pMaterial->wNumVertices );
							ASSERT ( wMid[0] < pMaterial->wNumVertices );
							ASSERT ( wMid[1] < pMaterial->wNumVertices );
							ASSERT ( wMid[2] < pMaterial->wNumVertices );

							// OK, so we now create 4 tris from wI1-3 and wMid[0-2].
							// wMid[0] will be between wI1 and wI2
							// wMid[1] will be between wI2 and wI3
							// wMid[2] will be between wI3 and wI1

							// Copy these, coz the very last 4 tris overwrites them.
							WORD wSrcIndex0 = pSrcIndex[0];
							WORD wSrcIndex1 = pSrcIndex[1];
							WORD wSrcIndex2 = pSrcIndex[2];

							pDstIndex[0*3+0] = wSrcIndex0;
							pDstIndex[0*3+1] = wMid[0];
							pDstIndex[0*3+2] = wMid[2];

							pDstIndex[1*3+0] = wSrcIndex1;
							pDstIndex[1*3+1] = wMid[1];
							pDstIndex[1*3+2] = wMid[0];

							pDstIndex[2*3+0] = wSrcIndex2;
							pDstIndex[2*3+1] = wMid[2];
							pDstIndex[2*3+2] = wMid[1];

							pDstIndex[3*3+0] = wMid[0];
							pDstIndex[3*3+1] = wMid[1];
							pDstIndex[3*3+2] = wMid[2];
						}

						ASSERT ( pSrcIndex == pDstIndex );


						TPO_iNumListIndices = iNewNumListIndices;
						pMaterial->wNumListIndices = iNewMatNumListIndices;

						// Inflation done!
						MemFree ( pEdgeList );
					}

#endif //#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB

					WORD *pSrcIndex;


					TRACE("Optimise");
					// Optimise the lists using MS's optimiser. Ta, MS. Saves me the hassle.
					int iRes = MSOptimizeIndexedList ( pFirstListIndex, pMaterial->wNumListIndices / 3 );
					ASSERT ( iRes != 0 );

					TRACE("Stripify");

					// And convert back to the strip format.
					ASSERT ( TPO_pCurStripIndex == pFirstStripIndex );
					pSrcIndex = pFirstListIndex;
					// Previous edge.
					WORD wIndex0 = -1;
					WORD wIndex1 = -1;
					bool bOdd = FALSE;
					bool bFirst = TRUE;
					for ( int i = pMaterial->wNumListIndices / 3; i > 0; i-- )
					{
						// Can we continue the list?
						WORD wNextIndex = -1;
						if ( ( wIndex0 == pSrcIndex[2] ) && ( wIndex1 == pSrcIndex[0] ) )
						{
							wNextIndex = pSrcIndex[1];
						}
						else if ( ( wIndex0 == pSrcIndex[0] ) && ( wIndex1 == pSrcIndex[1] ) )
						{
							wNextIndex = pSrcIndex[2];
						}
						else if ( ( wIndex0 == pSrcIndex[1] ) && ( wIndex1 == pSrcIndex[2] ) )
						{
							wNextIndex = pSrcIndex[0];
						}
						if ( wNextIndex != (WORD)-1 )
						{
							// We can continue the strip.
							*TPO_pCurStripIndex++ = wNextIndex;
							TPO_iNumStripIndices += 1;
							pMaterial->wNumStripIndices += 1;
							ASSERT ( TPO_iNumStripIndices < MAX_INDICES );

							if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

	#ifdef DEBUG
							if ( m_iMaxNumIndicesUsed < TPO_iNumStripIndices )
							{
								m_iMaxNumIndicesUsed = TPO_iNumStripIndices;
							}
	#endif

							if ( bOdd )
							{
								wIndex0 = wNextIndex;
							}
							else
							{
								wIndex1 = wNextIndex;
							}
							bOdd = !bOdd;
						}
						else
						{
							// No. Start a new one.
							if ( !bFirst )
							{
								*TPO_pCurStripIndex++ = -1;
								TPO_iNumStripIndices += 1;
								pMaterial->wNumStripIndices += 1;
								ASSERT ( TPO_iNumStripIndices < MAX_INDICES );

								if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

	#ifdef DEBUG
								if ( m_iMaxNumIndicesUsed < TPO_iNumStripIndices )
								{
									m_iMaxNumIndicesUsed = TPO_iNumStripIndices;
								}
	#endif

							}
							else
							{
								bFirst = FALSE;
							}
							*TPO_pCurStripIndex++ = pSrcIndex[0];
							*TPO_pCurStripIndex++ = pSrcIndex[1];
							*TPO_pCurStripIndex++ = pSrcIndex[2];
							TPO_iNumStripIndices += 3;
							pMaterial->wNumStripIndices += 3;
							ASSERT ( TPO_iNumStripIndices < MAX_INDICES );

							if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

	#ifdef DEBUG
							if ( m_iMaxNumIndicesUsed < TPO_iNumStripIndices )
							{
								m_iMaxNumIndicesUsed = TPO_iNumStripIndices;
							}
	#endif
							wIndex0 = pSrcIndex[2];
							wIndex1 = pSrcIndex[1];
							bOdd = FALSE;
						}
						pSrcIndex += 3;
					}
					// Finish with a final -1
					*TPO_pCurStripIndex++ = -1;
					TPO_iNumStripIndices += 1;
					pMaterial->wNumStripIndices += 1;
					ASSERT ( TPO_iNumStripIndices < MAX_INDICES );

					if ( TPO_iNumStripIndices >= MAX_INDICES ) { DeadAndBuried ( 0x07ff07ff ); }

	#ifdef DEBUG
					if ( m_iMaxNumIndicesUsed < TPO_iNumStripIndices )
					{
						m_iMaxNumIndicesUsed = TPO_iNumStripIndices;
					}
	#endif


					ASSERT ( pMaterial->wNumStripIndices == ( TPO_pCurStripIndex - pFirstStripIndex ) );

					TRACE("Done ");


				}

			}

			bOuterTris = !bOuterTris;
		} while ( bOuterTris );
	}


	// Set up the total number of vertices used.
	pPrimObj->wTotalSizeOfObj = TPO_iNumVertices;




	// Copy the vertices and indices.
	// These vertices much be aligned properly for the MM stuff, remember!



	// Do the alloc in one block to be nice to the TLB.
	DWORD dwTotalSize = 0;
	dwTotalSize += TPO_iNumListIndices * sizeof ( UWORD );
	dwTotalSize += 32 + TPO_iNumVertices * sizeof ( D3DVERTEX );
	dwTotalSize += TPO_iNumStripIndices * sizeof ( UWORD );

	// NOTE! I allocate an extra bit of space on the end because the MM code
	// actually reads the index just after the end, so that must be valid memory,
	// or you get a page fault. Now _that_ was a tricky bug to find, and had
	// been happening on and off for ages, and I could never track it down
	// (coz it would crash in the D3D driver, and the stack gets scrogged
	// in there, so I can't back-trace to find out what happened).
	dwTotalSize += 4 * sizeof (WORD);

	// Now that it's a unified block, TPO_pListIndices is the thing that needs to
	// be freed. Obviously, don't free the others - they don't own memory.
	char *pcBlock = (char *)MemAlloc ( dwTotalSize );
	ASSERT ( pcBlock != NULL );
	if ( pcBlock == NULL ) { DeadAndBuried ( 0xffe0ffe0 ); }

	pPrimObj->pwListIndices = (UWORD *)pcBlock;
	memcpy ( pPrimObj->pwListIndices, TPO_pListIndices, TPO_iNumListIndices * sizeof ( UWORD ) );
	pcBlock += TPO_iNumListIndices * sizeof ( UWORD );

	// Now the verts, aligned to 32 bytes lines.
	pPrimObj->pD3DVertices = (void *)( ( (DWORD)pcBlock + 31 ) & ~31 );
	memcpy ( pPrimObj->pD3DVertices , TPO_pVert, TPO_iNumVertices * sizeof ( D3DVERTEX ) );
	pcBlock = (char *)pPrimObj->pD3DVertices + TPO_iNumVertices * sizeof ( D3DVERTEX );

	pPrimObj->pwStripIndices = (UWORD *)pcBlock;
	memcpy ( pPrimObj->pwStripIndices, TPO_pStripIndices, TPO_iNumStripIndices * sizeof ( UWORD ) );
	pcBlock += TPO_iNumStripIndices * sizeof ( UWORD );

	ASSERT ( (DWORD)pcBlock < (DWORD)pPrimObj->pwListIndices + dwTotalSize );


#if 0
	//void *pPermanentVerts = MemAlloc ( 32 + TPO_iNumVertices * sizeof ( D3DVERTEX ) );
	pPermanentVerts  = (void *)( ( (DWORD)pPermanentVerts + 31 ) & ~31 );
	ASSERT ( pPermanentVerts != NULL );
	memcpy ( pPermanentVerts, TPO_pVert, TPO_iNumVertices * sizeof ( D3DVERTEX ) );
	pPrimObj->pD3DVertices = pPermanentVerts;

	//UWORD *pPermanentIndices = (UWORD *)MemAlloc ( TPO_iNumListIndices * sizeof ( UWORD ) );
	ASSERT ( pPermanentIndices != NULL );
	memcpy ( pPermanentIndices, TPO_pListIndices, TPO_iNumListIndices * sizeof ( UWORD ) );
	pPrimObj->pwListIndices = pPermanentIndices;

	//pPermanentIndices = (UWORD *)MemAlloc ( TPO_iNumStripIndices * sizeof ( UWORD ) );
	ASSERT ( pPermanentIndices != NULL );
	memcpy ( pPermanentIndices, TPO_pStripIndices, TPO_iNumStripIndices * sizeof ( UWORD ) );
	pPrimObj->pwStripIndices = pPermanentIndices;
#endif

	// Free the temporary spaces.
	MemFree ( TPO_piVertexLinks );
	MemFree ( TPO_piVertexRemap );
	MemFree ( TPO_pListIndices );
	MemFree ( TPO_pStripIndices );
	MemFree ( TPO_pVert );


	// And remember to take the square-root of this to make it an actual radius.
	pPrimObj->fBoundingSphereRadius = sqrtf ( pPrimObj->fBoundingSphereRadius );


	// Add this to a good place in the LRU queue.
	FIGURE_find_and_clean_prim_queue_item ( pPrimObj, iThrashIndex );


	// Clean up.
	TPO_pVert = NULL;
	TPO_pStripIndices = NULL;
	TPO_pListIndices = NULL;
	TPO_piVertexRemap = NULL;
	TPO_piVertexLinks = NULL;
	TPO_pCurVertex = NULL;
	TPO_pCurStripIndex = NULL;
	TPO_pCurListIndex = NULL;
	TPO_pPrimObj = NULL;
	TPO_iNumListIndices = 0;
	TPO_iNumStripIndices = 0;
	TPO_iNumVertices = 0;
	TPO_iNumPrims = 0;

	TRACE("done all.\n");

#ifdef DEBUG
	TRACE ( "Max so far: verts %i, indices %i\n", m_iMaxNumVertsUsed, m_iMaxNumIndicesUsed );
#endif

}





// Takes a "prim" description and generates D3D-style data for a single prim.
void FIGURE_generate_D3D_object ( SLONG prim )
{
	PrimObject  *p_obj = &prim_objects[prim];
	TomsPrimObject *pPrimObj = &(D3DObj[prim]);

	// Set it up.
	FIGURE_TPO_init_3d_object ( pPrimObj );

	// Add this prim to it as object 0.
	//FIGURE_TPO_add_prim_to_current_object ( p_obj, 0 );
	FIGURE_TPO_add_prim_to_current_object ( prim, 0 );


	// And clean it all up.
	FIGURE_TPO_finish_3d_object ( pPrimObj );
}





#endif //#if USE_TOMS_ENGINE_PLEASE_BOB





void FIGURE_draw_prim_tween(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG tween,
		struct GameKeyFrameElement *anim_info,
		struct GameKeyFrameElement *anim_info_next,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		ULONG colour,
		ULONG specular,
		CMatrix33 *parent_base_mat,
		Matrix31 *parent_base_pos,
		Matrix33 *parent_curr_mat,
		Matrix31 *parent_curr_pos,
		Matrix33 *end_mat,
		Matrix31 *end_pos,
		Thing    *p_thing,
		SLONG     part_number = 0xffffffff,
		ULONG     colour_and  = 0xffffffff)
{

	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG dprod;
	SLONG r;
	SLONG g;
	SLONG b;

	SLONG dr;
	SLONG dg;
	SLONG db;

	SLONG face_colour;

	SLONG page;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;

	ULONG qc0;
	ULONG qc1;
	ULONG qc2;
	ULONG qc3;

	SVector temp;
	
	PrimFace4   *p_f4;
	PrimFace3   *p_f3;
	PrimObject  *p_obj;
	NIGHT_Found *nf;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];
	SLONG	tex_page_offset;


	LOG_ENTER ( Figure_Draw_Prim_Tween )

	tex_page_offset=p_thing->Genus.Person->pcom_colour&0x3;

	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	if (parent_base_mat)
	{
		// we've got hierarchy info!
		
		Matrix31	p;
		p.M[0] = anim_info->OffsetX;
		p.M[1] = anim_info->OffsetY;
		p.M[2] = anim_info->OffsetZ;

		HIERARCHY_Get_Body_Part_Offset(&offset, &p,
									   parent_base_mat, parent_base_pos,
									   parent_curr_mat, parent_curr_pos);
		
		// pass data up the hierarchy
		if (end_pos)
			*end_pos = offset;
	}
	else
	{
		// process at highter resolution
		offset.M[0] = (anim_info->OffsetX << 8) + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween);
		offset.M[1] = (anim_info->OffsetY << 8) + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween);
		offset.M[2] = (anim_info->OffsetZ << 8) + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween);

/* We don't have bikes.
		if (p_thing->Class == CLASS_BIKE && part_number == 3)
		{	
			//offset.M[0] = 0x0;
			//offset.M[1] = 0x900;
			offset.M[2] = 0x3500;
		}
*/
		if (end_pos)
		{
			*end_pos = offset;
		}
	}

//	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);
	// convert pos to floating point here to preserve accuracy and prevent overflow.
	float	off_x = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[0][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[0][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[0][2]) / 32768.f);
	float	off_y = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[1][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[1][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[1][2]) / 32768.f);
	float	off_z = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[2][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[2][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[2][2]) / 32768.f);


	SLONG	character_scale  = person_get_scale(p_thing);
	float	character_scalef = float(character_scale) / 256.f;
	
	off_x *= character_scalef;
	off_y *= character_scalef;
	off_z *= character_scalef;

	/*

	if (p_thing->Class == CLASS_BIKE)
	{
		off_x = 0;
		off_y = 0;
		off_z = 0;
	}

	*/

	off_x += float(x);
	off_y += float(y);
	off_z += float(z);

	//
	// Do everything in floats.
	//

	float fmatrix[9];
	SLONG imatrix[9];

	{
		//
		// Create a temporary "tween" matrix between current and next
		//

		CMatrix33	m1, m2;
		GetCMatrix(anim_info, &m1);
		GetCMatrix(anim_info_next, &m2);

		CQuaternion::BuildTween(&mat2, &m1, &m2, tween);

		// pass data up the hierarchy
		if (end_mat)
			*end_mat = mat2;
	
		//
		// Apply local rotation matrix to get mat_final that rotates
		// the point into world space.
		//

		matrix_mult33(&mat_final, rot_mat, &mat2);

		//!  yeehaw!
		mat_final.M[0][0] = (mat_final.M[0][0] * character_scale) / 256;
		mat_final.M[0][1] = (mat_final.M[0][1] * character_scale) / 256;
		mat_final.M[0][2] = (mat_final.M[0][2] * character_scale) / 256;
		mat_final.M[1][0] = (mat_final.M[1][0] * character_scale) / 256;
		mat_final.M[1][1] = (mat_final.M[1][1] * character_scale) / 256;
		mat_final.M[1][2] = (mat_final.M[1][2] * character_scale) / 256;
		mat_final.M[2][0] = (mat_final.M[2][0] * character_scale) / 256;
		mat_final.M[2][1] = (mat_final.M[2][1] * character_scale) / 256;
		mat_final.M[2][2] = (mat_final.M[2][2] * character_scale) / 256;

		fmatrix[0] = float(mat_final.M[0][0]) * (1.0F / 32768.0F);
		fmatrix[1] = float(mat_final.M[0][1]) * (1.0F / 32768.0F);
		fmatrix[2] = float(mat_final.M[0][2]) * (1.0F / 32768.0F);
		fmatrix[3] = float(mat_final.M[1][0]) * (1.0F / 32768.0F);
		fmatrix[4] = float(mat_final.M[1][1]) * (1.0F / 32768.0F);
		fmatrix[5] = float(mat_final.M[1][2]) * (1.0F / 32768.0F);
		fmatrix[6] = float(mat_final.M[2][0]) * (1.0F / 32768.0F);
		fmatrix[7] = float(mat_final.M[2][1]) * (1.0F / 32768.0F);
		fmatrix[8] = float(mat_final.M[2][2]) * (1.0F / 32768.0F);

		imatrix[0] = mat_final.M[0][0] * 2;
		imatrix[1] = mat_final.M[0][1] * 2;
		imatrix[2] = mat_final.M[0][2] * 2;
		imatrix[3] = mat_final.M[1][0] * 2;
		imatrix[4] = mat_final.M[1][1] * 2;
		imatrix[5] = mat_final.M[1][2] * 2;
		imatrix[6] = mat_final.M[2][0] * 2;
		imatrix[7] = mat_final.M[2][1] * 2;
		imatrix[8] = mat_final.M[2][2] * 2;
	}

	/*

	if (part_number == SUB_OBJECT_HEAD)
	{
		fmatrix[0]=	+0.131534;
		fmatrix[1]=	-0.000000;
		fmatrix[2]=	+0.991312;
		fmatrix[3]=	+0.00133604;
		fmatrix[4]=	+0.999999;
		fmatrix[5]=	-0.000177275;
		fmatrix[6]=	-0.991311;
		fmatrix[7]=	0.00134775;
		fmatrix[8]=	0.131534;;
	}

	*/

	if (prim == 267)
	{
		static SLONG count = 0;

		count += 1;

#ifndef FINAL
#ifndef TARGET_DC
		if (ControlFlag&&allow_debug_keys)
		{
			LOG_EXIT ( Figure_Draw_Prim_Tween )
			return;
		}
#endif
#endif
	}

	LOG_ENTER ( Figure_Set_Rotation )

	POLY_set_local_rotation(
		off_x,
		off_y,
		off_z,
		fmatrix);

	LOG_ENTER ( Figure_Set_Rotation )


	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;

	// Check for being a gun
#ifndef	BUILD_PSX
	if (prim==256)
	{
		i=sp;		
	}
	else
	// Or a shotgun
	  if (prim==258)
	  {
		i=sp+15;
	  }
	  // or an AK
	  else
		  if (prim==260)
		  {
			i=sp+32;
		  }
		  else goto no_muzzle_calcs; // which skips...


	//
	// this bit, which only executes if one of the above tests is true.
    //
	pp = &POLY_buffer[POLY_buffer_upto]; // no ++, so reused
	pp->x=AENG_dx_prim_points[i].X;
	pp->y=AENG_dx_prim_points[i].Y;
	pp->z=AENG_dx_prim_points[i].Z;
	MATRIX_MUL(
		fmatrix,
		pp->x,
		pp->y,
		pp->z);

	pp->x+=off_x; pp->y+=off_y; pp->z+=off_z;
	p_thing->Genus.Person->GunMuzzle.X=pp->x*256;
	p_thing->Genus.Person->GunMuzzle.Y=pp->y*256;
	p_thing->Genus.Person->GunMuzzle.Z=pp->z*256;
//	x=pp->x*256; y=pp->y*256; z=pp->z*256;


#endif
no_muzzle_calcs:





#if USE_TOMS_ENGINE_PLEASE_BOB

	if ( !MM_bLightTableAlreadySetUp )
	{

#if 0

		// Set it up then.
		ASSERT ( MM_pcFadeTable == NULL );
		ASSERT ( MM_pcFadeTableTint == NULL );
		ASSERT ( MM_pMatrix == NULL );
		ASSERT ( MM_Vertex == NULL );
		ASSERT ( MM_pNormal == NULL );
		// Set up some data for the MM rendering thing if it's not already been done.

//	#define ALIGNED_STATIC_ARRAY(name,number,mytype,align)														\
//		static char c##name##mytype##align##StaticArray [ align + number * sizeof ( mytype ) ];					\
//		name = (mytype *)( ( (DWORD)c##name##mytype##align##StaticArray + (align-1) ) & ~(align-1) )

		ALIGNED_STATIC_ARRAY ( MM_pcFadeTable, 128, D3DCOLOR, 4 );
		ALIGNED_STATIC_ARRAY ( MM_pcFadeTableTint, 128, D3DCOLOR, 4 );
		ALIGNED_STATIC_ARRAY ( MM_pMatrix, 1, D3DMATRIX, 32 );
		ALIGNED_STATIC_ARRAY ( MM_Vertex, 4, D3DVERTEX, 32 );
		ALIGNED_STATIC_ARRAY ( MM_pNormal, 4, float, 8 );

#endif

	}

#endif



	if (WITHIN(prim, 261, 263))
	{
		//
		// This is a muzzle flash! They don't have any lighting!
		//

		for (i = sp; i < ep; i++)
		{
			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			pp = &POLY_buffer[POLY_buffer_upto++];

			POLY_transform_using_local_rotation(
				AENG_dx_prim_points[i].X,
				AENG_dx_prim_points[i].Y,
				AENG_dx_prim_points[i].Z,
				pp);

			pp->colour   = 0xff808080;
			pp->specular = 0xff000000;
		}


		for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
		{
			p_f4 = &prim_faces4[i];

			p0 = p_f4->Points[0] - sp;
			p1 = p_f4->Points[1] - sp;
			p2 = p_f4->Points[2] - sp;
			p3 = p_f4->Points[3] - sp;
			
			ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

			quad[0] = &POLY_buffer[p0];
			quad[1] = &POLY_buffer[p1];
			quad[2] = &POLY_buffer[p2];
			quad[3] = &POLY_buffer[p3];

			if (POLY_valid_quad(quad))
			{
				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);
														
				quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
				quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);
														
				quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
				quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

				quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
				quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

				page   = p_f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f4->TexturePage;

				if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
				{
					page=alt_texture[page-10*64]+tex_page_offset-1;
				}
				else
					page+=FACE_PAGE_OFFSET;

				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}

		for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
		{
			p_f3 = &prim_faces3[i];

			p0 = p_f3->Points[0] - sp;
			p1 = p_f3->Points[1] - sp;
			p2 = p_f3->Points[2] - sp;
			
			ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

			tri[0] = &POLY_buffer[p0];
			tri[1] = &POLY_buffer[p1];
			tri[2] = &POLY_buffer[p2];

			if (POLY_valid_triangle(tri))
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
														
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
														
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;

				if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
				{
					page=alt_texture[page-10*64]+tex_page_offset-1;
				}
				else
					page+=FACE_PAGE_OFFSET;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}

		LOG_EXIT ( Figure_Draw_Prim_Tween )
		return;
	}
	else
	{



#if USE_TOMS_ENGINE_PLEASE_BOB

		if ( !MM_bLightTableAlreadySetUp )
		{

			Pyro *p = NULL;
			if (p_thing->Class == CLASS_PERSON && p_thing->Genus.Person->BurnIndex)
			{
				p = TO_PYRO(p_thing->Genus.Person->BurnIndex-1);
				if (p->PyroType != PYRO_IMMOLATE)
				{
					p = NULL;
				}
			}
			BuildMMLightingTable ( p, colour_and );

		}



		LOG_ENTER ( Figure_Build_Matrices )

		extern float POLY_cam_matrix_comb[9];
		extern float POLY_cam_off_x;
		extern float POLY_cam_off_y;
		extern float POLY_cam_off_z;


		extern D3DMATRIX g_matProjection;
		extern D3DMATRIX g_matWorld;
		extern D3DVIEWPORT2 g_viewData;


		D3DMATRIX matTemp;

		{
			//_Multiply4dM((float *)pResultMatrix, (float *)g_matWorld, (float *)g_matProjection);

			matTemp._11 = g_matWorld._11*g_matProjection._11 + g_matWorld._12*g_matProjection._21 + g_matWorld._13*g_matProjection._31 + g_matWorld._14*g_matProjection._41;
			matTemp._12 = g_matWorld._11*g_matProjection._12 + g_matWorld._12*g_matProjection._22 + g_matWorld._13*g_matProjection._32 + g_matWorld._14*g_matProjection._42;
			matTemp._13 = g_matWorld._11*g_matProjection._13 + g_matWorld._12*g_matProjection._23 + g_matWorld._13*g_matProjection._33 + g_matWorld._14*g_matProjection._43;
			matTemp._14 = g_matWorld._11*g_matProjection._14 + g_matWorld._12*g_matProjection._24 + g_matWorld._13*g_matProjection._34 + g_matWorld._14*g_matProjection._44;

			matTemp._21 = g_matWorld._21*g_matProjection._11 + g_matWorld._22*g_matProjection._21 + g_matWorld._23*g_matProjection._31 + g_matWorld._24*g_matProjection._41;
			matTemp._22 = g_matWorld._21*g_matProjection._12 + g_matWorld._22*g_matProjection._22 + g_matWorld._23*g_matProjection._32 + g_matWorld._24*g_matProjection._42;
			matTemp._23 = g_matWorld._21*g_matProjection._13 + g_matWorld._22*g_matProjection._23 + g_matWorld._23*g_matProjection._33 + g_matWorld._24*g_matProjection._43;
			matTemp._24 = g_matWorld._21*g_matProjection._14 + g_matWorld._22*g_matProjection._24 + g_matWorld._23*g_matProjection._34 + g_matWorld._24*g_matProjection._44;

			matTemp._31 = g_matWorld._31*g_matProjection._11 + g_matWorld._32*g_matProjection._21 + g_matWorld._33*g_matProjection._31 + g_matWorld._34*g_matProjection._41;
			matTemp._32 = g_matWorld._31*g_matProjection._12 + g_matWorld._32*g_matProjection._22 + g_matWorld._33*g_matProjection._32 + g_matWorld._34*g_matProjection._42;
			matTemp._33 = g_matWorld._31*g_matProjection._13 + g_matWorld._32*g_matProjection._23 + g_matWorld._33*g_matProjection._33 + g_matWorld._34*g_matProjection._43;
			matTemp._34 = g_matWorld._31*g_matProjection._14 + g_matWorld._32*g_matProjection._24 + g_matWorld._33*g_matProjection._34 + g_matWorld._34*g_matProjection._44;

			matTemp._41 = g_matWorld._41*g_matProjection._11 + g_matWorld._42*g_matProjection._21 + g_matWorld._43*g_matProjection._31 + g_matWorld._44*g_matProjection._41;
			matTemp._42 = g_matWorld._41*g_matProjection._12 + g_matWorld._42*g_matProjection._22 + g_matWorld._43*g_matProjection._32 + g_matWorld._44*g_matProjection._42;
			matTemp._43 = g_matWorld._41*g_matProjection._13 + g_matWorld._42*g_matProjection._23 + g_matWorld._43*g_matProjection._33 + g_matWorld._44*g_matProjection._43;
			matTemp._44 = g_matWorld._41*g_matProjection._14 + g_matWorld._42*g_matProjection._24 + g_matWorld._43*g_matProjection._34 + g_matWorld._44*g_matProjection._44;
		}   







		// Now make up the matrices.

#if 0
		// Officially correct version.
		DWORD dwWidth = g_viewData.dwWidth >> 1;
		DWORD dwHeight = g_viewData.dwHeight >> 1;
		DWORD dwX = g_viewData.dwX;
		DWORD dwY = g_viewData.dwY;
#else
		// Version that knows about the letterbox mode hack.
extern DWORD g_dw3DStuffHeight;
extern DWORD g_dw3DStuffY;
		DWORD dwWidth = g_viewData.dwWidth >> 1;
		DWORD dwHeight = g_dw3DStuffHeight >> 1;
		DWORD dwX = g_viewData.dwX;
		DWORD dwY = g_dw3DStuffY;
#endif
		MM_pMatrix[0]._11 = 0.0f;
		MM_pMatrix[0]._12 = matTemp._11 *   (float)dwWidth  + matTemp._14 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._13 = matTemp._12 * - (float)dwHeight + matTemp._14 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._14 = matTemp._14;
		MM_pMatrix[0]._21 = 0.0f;
		MM_pMatrix[0]._22 = matTemp._21 *   (float)dwWidth  + matTemp._24 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._23 = matTemp._22 * - (float)dwHeight + matTemp._24 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._24 = matTemp._24;
		MM_pMatrix[0]._31 = 0.0f;
		MM_pMatrix[0]._32 = matTemp._31 *   (float)dwWidth  + matTemp._34 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._33 = matTemp._32 * - (float)dwHeight + matTemp._34 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._34 = matTemp._34;
		// Validation magic number.
		unsigned long EVal = 0xe0001000;
		MM_pMatrix[0]._41 = *(float *)&EVal;
		MM_pMatrix[0]._42 = matTemp._41 *   (float)dwWidth  + matTemp._44 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._43 = matTemp._42 * - (float)dwHeight + matTemp._44 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._44 = matTemp._44;



		// 251 is a magic number for the DIP call!
		const float fNormScale = 251.0f;

		// Transform the lighting direction(s) by the inverse object matrix to get it into object space.
		// Assume inverse=transpose.
		D3DVECTOR vTemp;
		vTemp.x = MM_vLightDir.x * fmatrix[0] + MM_vLightDir.y * fmatrix[3] + MM_vLightDir.z * fmatrix[6];
		vTemp.y = MM_vLightDir.x * fmatrix[1] + MM_vLightDir.y * fmatrix[4] + MM_vLightDir.z * fmatrix[7];
		vTemp.z = MM_vLightDir.x * fmatrix[2] + MM_vLightDir.y * fmatrix[5] + MM_vLightDir.z * fmatrix[8];

		MM_pNormal[0] = 0.0f;
		MM_pNormal[1] = vTemp.x * fNormScale;
		MM_pNormal[2] = vTemp.y * fNormScale;
		MM_pNormal[3] = vTemp.z * fNormScale;

		LOG_EXIT ( Figure_Build_Matrices )


#else //#if USE_TOMS_ENGINE_PLEASE_BOB


		Pyro *p = NULL;

		if (p_thing->Class == CLASS_PERSON && p_thing->Genus.Person->BurnIndex)
		{
			p = TO_PYRO(p_thing->Genus.Person->BurnIndex-1);

			if (p->PyroType != PYRO_IMMOLATE)
			{
				p = NULL;
			}
		}


		for (i = sp; i < ep; i++)
		{
			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			pp = &POLY_buffer[POLY_buffer_upto++];

			POLY_transform_using_local_rotation(
				AENG_dx_prim_points[i].X,
				AENG_dx_prim_points[i].Y,
				AENG_dx_prim_points[i].Z,
				pp);

			//
			// Do the lighting...
			//

			{
				nx = prim_normal[i].X;
				ny = prim_normal[i].Y;
				nz = prim_normal[i].Z;

				FMATRIX_MUL(
					imatrix,
					nx,
					ny,
					nz);

				dprod = 
					nx * NIGHT_amb_norm_x + 
					ny * NIGHT_amb_norm_y + 
					nz * NIGHT_amb_norm_z;

				r = NIGHT_amb_red   << 0;
				g = NIGHT_amb_green << 0;
				b = NIGHT_amb_blue  << 0;

#ifndef TARGET_DC
				extern UBYTE sw_hack;
#endif

				if (dprod > 0)
				{
					dr = NIGHT_amb_red   * dprod >> 15;
					dg = NIGHT_amb_green * dprod >> 15;
					db = NIGHT_amb_blue  * dprod >> 15;

					r += dr;// + (dr >> 1);
					g += dg;// + (dg >> 1);
					b += db;// + (db >> 1);
				}

				//
				// Now for the lights...
				//

				for (j = 0; j < NIGHT_found_upto; j++)
				{
					nf = &NIGHT_found[j];

					dprod = 
						nx * nf->dx + 
						ny * nf->dy + 
						nz * nf->dz;

					if (dprod < 0)
					{
						dr = nf->r * dprod >> 15;
						dg = nf->g * dprod >> 15;
						db = nf->b * dprod >> 15;

						r -= dr;// + (dr >> 1);
						g -= dg;// + (dg >> 1);
						b -= db;// + (db >> 1);
					}
				}

				if (p)
				{
					r = (r > p->counter) ? (r - p->counter) : 10; 
					g = (g > p->counter) ? (g - p->counter) :  4;
					b = (b > p->counter) ? (b - p->counter) :  3;
				}

#ifndef TARGET_DC
				if (sw_hack)
				{
					r <<= 1;
					g <<= 1;
					b <<= 1;
				}
#endif

				SATURATE(r, 0, 255);
				SATURATE(g, 0, 255);
				SATURATE(b, 0, 255);

				pp->colour   = (r << 16) | (g << 8) | (b << 0);
				pp->colour  |= FIGURE_alpha << 24;
				pp->specular = 0xff000000;
			}
		}
#endif //#else //#if USE_TOMS_ENGINE_PLEASE_BOB

	}




#if USE_TOMS_ENGINE_PLEASE_BOB



#if 1

#if 1

	// The wonderful NEW system!


	LOG_ENTER ( Figure_Draw_Polys )

#if 1
	// The MM stuff doesn't like specular to be enabled.
	(the_display.lp_D3D_Device)->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, FALSE );
#endif


	// For now, just calculate as-and-when.
	TomsPrimObject *pPrimObj = &(D3DObj[prim]);
	if ( pPrimObj->wNumMaterials == 0 )
	{
		// Not initialised. Do so.
		// It's not fair to count this as part of the drawing! :-)
		LOG_EXIT ( Figure_Draw_Polys )

		FIGURE_generate_D3D_object ( prim );
		LOG_ENTER ( Figure_Draw_Polys )
	}

	// Tell the LRU cache we used this one.
	FIGURE_touch_LRU_of_object ( pPrimObj );

	ASSERT ( pPrimObj->pD3DVertices != NULL );
	ASSERT ( pPrimObj->pMaterials != NULL );
	ASSERT ( pPrimObj->pwListIndices != NULL );
	ASSERT ( pPrimObj->pwStripIndices != NULL );
	ASSERT ( pPrimObj->wNumMaterials != 0 );

	PrimObjectMaterial *pMat = pPrimObj->pMaterials;

	D3DMULTIMATRIX d3dmm;
	d3dmm.lpd3dMatrices = MM_pMatrix;
	d3dmm.lpvLightDirs = MM_pNormal;

	D3DVERTEX *pVertex = (D3DVERTEX *)pPrimObj->pD3DVertices;
	UWORD *pwListIndices = pPrimObj->pwListIndices;
	UWORD *pwStripIndices = pPrimObj->pwStripIndices;
	for ( int iMatNum = pPrimObj->wNumMaterials; iMatNum > 0; iMatNum-- )
	{
		// Set up the right texture for this material.

		UWORD wPage = pMat->wTexturePage;
		UWORD wRealPage = wPage & TEXTURE_PAGE_MASK;

		if ( wPage & TEXTURE_PAGE_FLAG_JACKET )
		{
			// Find the real jacket page.
			wRealPage = jacket_lookup [ wRealPage ][GET_SKILL(p_thing)>>2];
			wRealPage += FACE_PAGE_OFFSET;
		}
		else if ( wPage & TEXTURE_PAGE_FLAG_OFFSET )
		{
			// An "offset" texture. This will be offset by a certain amount to
			// allow each prim to have different coloured clothes on.
			if ( tex_page_offset == 0 )
			{
				// No lookup offset.
				// This has not been offset yet.
				wRealPage += FACE_PAGE_OFFSET;
			}
			else
			{
				// Look this up.
				wRealPage = alt_texture[wRealPage-(10*64)]+tex_page_offset-1;
			}
		}

#ifdef DEBUG
		if ( wPage & TEXTURE_PAGE_FLAG_NOT_TEXTURED )
		{
			ASSERT ( wRealPage == POLY_PAGE_COLOUR );
		}
#endif

extern D3DMATRIX g_matWorld;

		PolyPage *pa = &(POLY_Page[wRealPage]);
		// Not sure if I'm using character_scalef correctly...
		ASSERT ( ( character_scalef < 1.2f ) && ( character_scalef > 0.8f ) );
		if ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) &&
			 ( ( ( g_matWorld._43 * 32768.0f ) - ( pPrimObj->fBoundingSphereRadius * character_scalef ) ) > ( POLY_ZCLIP_PLANE * 32768.0f ) ) )
		{
			// Non-alpha path.
			if ( wPage & TEXTURE_PAGE_FLAG_TINT )
			{
				// Tinted colours.
				d3dmm.lpLightTable = MM_pcFadeTableTint;
			}
			else
			{
				// Normal.
				d3dmm.lpLightTable = MM_pcFadeTable;
			}
			d3dmm.lpvVertices = pVertex;



#if 1



#ifdef DEBUG
static int iCounter = 0;
			if ( iCounter != 0 )
			{
				iCounter--;
				ASSERT ( iCounter != 0 );
			}
#endif

			// Fast as lightning.
			LOG_ENTER ( Figure_Set_RenderState )
			pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();
			LOG_EXIT ( Figure_Set_RenderState )
			LOG_ENTER ( Figure_DrawIndPrimMM )


#if 0
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			//TRACE("Drew %i vertices, %i indices\n", (int)( pMat->wNumVertices ), (int)( pMat->wNumStripIndices ) );
#else
			// Use platform-independent version.

			HRESULT hres;




#define SHOW_ME_FIGURE_DEBUGGING_PLEASE_BOB defined

#ifdef SHOW_ME_FIGURE_DEBUGGING_PLEASE_BOB
#ifdef DEBUG
#ifdef TARGET_DC
#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)
extern DIJOYSTATE the_state;
			bool bShowDebug = FALSE;
			if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) && BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) )
			{
				DWORD dwColour = (DWORD)pwStripIndices;
				dwColour = ( dwColour >> 2 ) ^ ( dwColour >> 6 ) ^ ( dwColour ) ^ ( dwColour << 3 );
				dwColour = ( dwColour << 9 ) ^ ( dwColour << 19 ) ^ ( dwColour ) ^ ( dwColour << 29 ) ^ ( dwColour >> 3 );
				dwColour &= 0x7f7f7f7f;
				for ( int i = 0; i < 128; i++ )
				{
					d3dmm.lpLightTable[i] = dwColour;
				}

				// And NULL texture (i.e. white).
				the_display.lp_D3D_Device->SetTexture ( 0, NULL );
			}
#endif
#endif
#endif



			//if (pMat->wNumVertices &&
			//	pMat->wNumStripIndices)
			{
				//TRACE ( "S4" );
				hres = DrawIndPrimMM (
						(the_display.lp_D3D_Device),
						D3DFVF_VERTEX,
						&d3dmm,
						pMat->wNumVertices,
						pwStripIndices,
						pMat->wNumStripIndices );
				//TRACE ( "F4" );
			}
#endif




#else

			// Do some performance tracing.
#ifndef DTRACE
#error Don't use this codepath unless DTRACING, fool!
#endif

			LOG_ENTER ( Figure_Set_RenderState )
			pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();

			WORD wTempVerts[4];
			wTempVerts[0] = 0;
			wTempVerts[1] = 1;
			wTempVerts[2] = 2;
			wTempVerts[3] = -1;
			(the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					3,
					wTempVerts,
					4,
					D3DDP_MULTIMATRIX );
			LOG_EXIT ( Figure_Set_RenderState )

			LOG_ENTER ( Figure_DrawIndPrimMM )
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			//TRACE("Drew %i vertices, %i indices\n", (int)( pMat->wNumVertices ), (int)( pMat->wNumStripIndices ) );

#endif


//			ASSERT ( SUCCEEDED ( hres ) );  //triggers all the time when inside on start of RTA
#if 0
			// Can we have 4x polys for free? Oh go on....
			hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
#endif
			LOG_EXIT ( Figure_DrawIndPrimMM )

		}
		else
		{
			// Alpha/clipped path - do with standard non-MM calls.
			// FIXME. Needs to be done.

			// Actually, the fast-accept works very well, and it's only when the camera somehow gets REALLY close
			// that this happens. And actually a pop-reject seems a bit better than a clip. Certainly
			// there is no visually "right" thing to do. So leave it for now until someone complains. ATF.
			//TRACE ( "Tried to draw an alpha/clipped prim!" );
		}
		


		// Next material
		pVertex += pMat->wNumVertices;
		pwListIndices += pMat->wNumListIndices;
		pwStripIndices += pMat->wNumStripIndices;

		pMat++;
	}

#if 1
	// The MM stuff doesn't like specular to be enabled.
	(the_display.lp_D3D_Device)->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, TRUE );
#endif

	LOG_EXIT ( Figure_Draw_Polys )


#endif

#else

	//
	// The quads.
	//


	float fMyU[4], fMyV[4];

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		fMyU[0] = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
		fMyV[0] = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);
											
		fMyU[1] = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
		fMyV[1] = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);
											
		fMyU[2] = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
		fMyV[2] = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

		fMyU[3] = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
		fMyV[3] = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

		page   = p_f4->UV[0][0] & 0xc0;
		page <<= 2;
		page  |= p_f4->TexturePage;

		if(p_f4->FaceFlags&FACE_FLAG_THUG_JACKET)
		{
			switch(page)
			{
				case	64+21:
				case	10*64+2:
				case	10*64+32:
					page=jacket_lookup[0][GET_SKILL(p_thing)>>2];
					break;
				case	64+22:
				case	10*64+3:
				case	10*64+33:
					page=jacket_lookup[1][GET_SKILL(p_thing)>>2];
					break;
				case	64+24:
				case	10*64+4:
				case	10*64+36:
					page=jacket_lookup[2][GET_SKILL(p_thing)>>2];
					break;
				case	64+25:
				case	10*64+5:
				case	10*64+37:
					page=jacket_lookup[3][GET_SKILL(p_thing)>>2];
					break;
				default:
//							ASSERT(0);
					break;
			}
			page+=FACE_PAGE_OFFSET;
		}
		else

		if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
//				if(alt_texture[page-512]&&page<1024)
		{
			page=alt_texture[page-10*64]+tex_page_offset-1;
		}
		else
			page+=FACE_PAGE_OFFSET;

#ifndef TARGET_DC
		extern UBYTE sw_hack;
#endif

		PolyPage *pa = &(POLY_Page[page]);
		if ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) )
		{
			// Unlit untransformed vertices!
			WORD wIndices[5] = {0,1,2,3,-1};
			for ( int i = 0; i < 4; i++ )
			{
				const float fNormScale = 1.0f / 256.0f;

				int pt = p_f4->Points[i];
				ASSERT(WITHIN(pt, sp, ep));

				MM_Vertex[i].dvX = AENG_dx_prim_points[pt].X;
				MM_Vertex[i].dvY = AENG_dx_prim_points[pt].Y;
				MM_Vertex[i].dvZ = AENG_dx_prim_points[pt].Z;
				MM_Vertex[i].dvNX = prim_normal[pt].X * fNormScale;
				MM_Vertex[i].dvNY = prim_normal[pt].Y * fNormScale;
				MM_Vertex[i].dvNZ = prim_normal[pt].Z * fNormScale;
				MM_Vertex[i].dvTU = fMyU[i];
				MM_Vertex[i].dvTV = fMyV[i];
				// This must be done after setting the rest up, since it blats the 12th byte of the vertex.
				SET_MM_INDEX ( (MM_Vertex[i]), 0 );
			}

			D3DMULTIMATRIX d3dmm;
			d3dmm.lpd3dMatrices = MM_pMatrix;
			d3dmm.lpvLightDirs = MM_pNormal;
			if (p_f4->FaceFlags & FACE_FLAG_TINT)
			{
				// Tinted colours.
				d3dmm.lpLightTable = MM_pcFadeTableTint;
			}
			else
			{
				// Normal.
				d3dmm.lpLightTable = MM_pcFadeTable;
			}
			d3dmm.lpvVertices = MM_Vertex;



			// Although in theory this might happen, I've never actually seen it.
			ASSERT ( ( p_f4->DrawFlags & POLY_FLAG_TEXTURED ) != 0 );



			//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, (void *)&d3dmm, 4, wIndices, 5, D3DDP_MULTIMATRIX );
			ASSERT ( SUCCEEDED ( hres ) );
		}
		else
		{
			// Alpha-blended, clipped, or otherwise special-cased. Ignore for now, but need to be handled.
		}
	}


	//
	// The tris.
	//


	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		fMyU[0] = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
		fMyV[0] = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
											
		fMyU[1] = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
		fMyV[1] = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
											
		fMyU[2] = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
		fMyV[2] = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

		page   = p_f3->UV[0][0] & 0xc0;
		page <<= 2;
		page  |= p_f3->TexturePage;

		if(p_f3->FaceFlags&FACE_FLAG_THUG_JACKET)
		{
			switch(page)
			{
				case	64+21:
				case	10*64+2:
				case	10*64+32:
					page=jacket_lookup[0][GET_SKILL(p_thing)>>2];
					break;
				case	64+22:
				case	10*64+3:
				case	10*64+33:
					page=jacket_lookup[1][GET_SKILL(p_thing)>>2];
					break;
				case	64+24:
				case	10*64+4:
				case	10*64+36:
					page=jacket_lookup[2][GET_SKILL(p_thing)>>2];
					break;
				case	64+25:
				case	10*64+5:
				case	10*64+37:
					page=jacket_lookup[3][GET_SKILL(p_thing)>>2];
					break;
				default:
//							ASSERT(0);
					break;
			}
			page+=FACE_PAGE_OFFSET;
		}
		else

		if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
//				if(alt_texture[page-512]&&page<1024)
		{
			page=alt_texture[page-10*64]+tex_page_offset-1;
		}
		else
			page+=FACE_PAGE_OFFSET;

#ifndef TARGET_DC
		extern UBYTE sw_hack;
#endif

		PolyPage *pa = &(POLY_Page[page]);
		if ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) )
		{
			// Unlit untransformed vertices!
			WORD wIndices[4] = {0,1,2,-1};
			for ( int i = 0; i < 3; i++ )
			{
				const float fNormScale = 1.0f / 256.0f;

				int pt = p_f3->Points[i];
				ASSERT(WITHIN(pt, sp, ep));

				MM_Vertex[i].dvX = AENG_dx_prim_points[pt].X;
				MM_Vertex[i].dvY = AENG_dx_prim_points[pt].Y;
				MM_Vertex[i].dvZ = AENG_dx_prim_points[pt].Z;
				MM_Vertex[i].dvNX = prim_normal[pt].X * fNormScale;
				MM_Vertex[i].dvNY = prim_normal[pt].Y * fNormScale;
				MM_Vertex[i].dvNZ = prim_normal[pt].Z * fNormScale;
				MM_Vertex[i].dvTU = fMyU[i];
				MM_Vertex[i].dvTV = fMyV[i];
				// This must be done after setting the rest up, since it blats the 12th byte of the vertex.
				SET_MM_INDEX ( (MM_Vertex[i]), 0 );
			}

			D3DMULTIMATRIX d3dmm;
			d3dmm.lpd3dMatrices = MM_pMatrix;
			d3dmm.lpvLightDirs = MM_pNormal;
			if (p_f3->FaceFlags & FACE_FLAG_TINT)
			{
				// Tinted colours.
				d3dmm.lpLightTable = MM_pcFadeTableTint;
			}
			else
			{
				// Normal.
				d3dmm.lpLightTable = MM_pcFadeTable;
			}
			d3dmm.lpvVertices = MM_Vertex;



			// Although in theory this might happen, I've never actually seen it.
			ASSERT ( ( p_f3->DrawFlags & POLY_FLAG_TEXTURED ) != 0 );



			//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, (void *)&d3dmm, 3, wIndices, 4, D3DDP_MULTIMATRIX );
			ASSERT ( SUCCEEDED ( hres ) );
		}
		else
		{
			// Alpha-blended, clipped, or otherwise special-cased. Ignore for now, but need to be handled.
		}
	}


#endif


#else //#if USE_TOMS_ENGINE_PLEASE_BOB

	//
	// The quads.
	//

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - sp;
		p1 = p_f4->Points[1] - sp;
		p2 = p_f4->Points[2] - sp;
		p3 = p_f4->Points[3] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));
/*
		p_f4->Bright[0]=store_dprod[p0];
		p_f4->Bright[1]=store_dprod[p1];
		p_f4->Bright[2]=store_dprod[p2];
		p_f4->Bright[4]=store_dprod[p3];
*/


		quad[0] = &POLY_buffer[p0];
		quad[1] = &POLY_buffer[p1];
		quad[2] = &POLY_buffer[p2];
		quad[3] = &POLY_buffer[p3];

		if (POLY_valid_quad(quad))
		{
			if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
			{
				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);
												        
				quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
				quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);
												        
				quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
				quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

				quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
				quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

				if (p_f4->FaceFlags & FACE_FLAG_TINT)
				{
					qc0 = quad[0]->colour;
					qc1 = quad[1]->colour;
					qc2 = quad[2]->colour;
					qc3 = quad[3]->colour;

					quad[0]->colour &= colour_and;
					quad[1]->colour &= colour_and;
					quad[2]->colour &= colour_and;
					quad[3]->colour &= colour_and;
				}

				page   = p_f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f4->TexturePage;

				if(p_f4->FaceFlags&FACE_FLAG_THUG_JACKET)
				{
					switch(page)
					{
						case	64+21:
						case	10*64+2:
						case	10*64+32:
							page=jacket_lookup[0][GET_SKILL(p_thing)>>2];
							break;
						case	64+22:
						case	10*64+3:
						case	10*64+33:
							page=jacket_lookup[1][GET_SKILL(p_thing)>>2];
							break;
						case	64+24:
						case	10*64+4:
						case	10*64+36:
							page=jacket_lookup[2][GET_SKILL(p_thing)>>2];
							break;
						case	64+25:
						case	10*64+5:
						case	10*64+37:
							page=jacket_lookup[3][GET_SKILL(p_thing)>>2];
							break;
						default:
//							ASSERT(0);
							break;
					}
					page+=FACE_PAGE_OFFSET;
				}
				else

				if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
//				if(alt_texture[page-512]&&page<1024)
				{
					page=alt_texture[page-10*64]+tex_page_offset-1;
				}
				else
					page+=FACE_PAGE_OFFSET;

#ifndef TARGET_DC
				extern UBYTE sw_hack;
#endif

				PolyPage *pa = &(POLY_Page[page]);
#if USE_TOMS_ENGINE_PLEASE_BOB
				if ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) )
				{
					// Do an immediate render, not a deferred one.

					//
					// and render the polygons
					//

#if 0
					WORD wIndices[6] = {0,1,2,2,1,3};
					D3DTLVERTEX tlVertex[4];
					for ( int i = 0; i < 4; i++ )
					{
						tlVertex[i].dvSX = quad[i]->X * PolyPage::s_XScale;
						tlVertex[i].dvSY = quad[i]->Y * PolyPage::s_YScale;
						tlVertex[i].dvSZ = 1.0f - quad[i]->Z;
						tlVertex[i].dvTU = quad[i]->u;
						tlVertex[i].dvTV = quad[i]->v;
						tlVertex[i].dcColor = quad[i]->colour;
						tlVertex[i].dcSpecular = quad[i]->specular;
						tlVertex[i].dvRHW = quad[i]->Z;
					}

					HRESULT hres;

					if (FALSE && the_display.GetDeviceInfo()->AdamiLightingSupported())
					{
						// Draw lighting triangle
						PolyPage *tpa = &(POLY_Page[POLY_PAGE_COLOUR]);
						//tpa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
						tpa->RS.SetChanged();
						hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, tlVertex, 4, wIndices, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
					}

					//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
					pa->RS.SetChanged();
					pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
					pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
					hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, tlVertex, 4, wIndices, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
#else

					static iCount = 0;

					if ( FALSE )
					{

						// LVertex.
						WORD wIndices[6] = {0,1,2,2,1,3};
						D3DLVERTEX lVertex[4];
						for ( int i = 0; i < 4; i++ )
						{

							int pt = p_f4->Points[i];
							ASSERT(WITHIN(pt, sp, ep));
							lVertex[i].dvX = AENG_dx_prim_points[pt].X;
							lVertex[i].dvY = AENG_dx_prim_points[pt].Y;
							lVertex[i].dvZ = AENG_dx_prim_points[pt].Z;
							lVertex[i].dvTU = quad[i]->u;
							lVertex[i].dvTV = quad[i]->v;
							lVertex[i].dcColor = quad[i]->colour;
							lVertex[i].dcSpecular = quad[i]->specular;
						}

						HRESULT hres;

						if (FALSE && the_display.GetDeviceInfo()->AdamiLightingSupported())
						{
							// Draw lighting triangle
							PolyPage *tpa = &(POLY_Page[POLY_PAGE_COLOUR]);
							tpa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
							tpa->RS.SetChanged();
							hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, lVertex, 4, wIndices, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
						}

						//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
						pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
						pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
						pa->RS.SetChanged();
						hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, lVertex, 4, wIndices, 6, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
					}
#ifdef TARGET_DC
					else
					{

						// Use the MM stuff.
						
						// Unlit untransformed vertices!
						WORD wIndices[5] = {0,1,2,3,-1};
						for ( int i = 0; i < 4; i++ )
						{
							const float fNormScale = 1.0f / 256.0f;

							int pt = p_f4->Points[i];
							ASSERT(WITHIN(pt, sp, ep));

							MM_Vertex[i].dvX = AENG_dx_prim_points[pt].X;
							MM_Vertex[i].dvY = AENG_dx_prim_points[pt].Y;
							MM_Vertex[i].dvZ = AENG_dx_prim_points[pt].Z;
							MM_Vertex[i].dvNX = prim_normal[pt].X * fNormScale;
							MM_Vertex[i].dvNY = prim_normal[pt].Y * fNormScale;
							MM_Vertex[i].dvNZ = prim_normal[pt].Z * fNormScale;
							MM_Vertex[i].dvTU = quad[i]->u;
							MM_Vertex[i].dvTV = quad[i]->v;
							// This must be done after setting the rest up, since it blats the 12th byte of the vertex.
							SET_MM_INDEX ( (MM_Vertex[i]), 0 );
						}

#if 0
						if ( FALSE && the_display.GetDeviceInfo()->AdamiLightingSupported() )
						{
							// Draw lighting triangle
							PolyPage *tpa = &(POLY_Page[POLY_PAGE_COLOUR]);
							//tpa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
							tpa->RS.SetChanged();
							hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, MM_Vertex, 3, wIndices, 3, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
						}
#endif

						D3DMULTIMATRIX d3dmm;
						d3dmm.lpd3dMatrices = MM_pMatrix;
						d3dmm.lpvLightDirs = MM_pNormal;
						d3dmm.lpLightTable = MM_pcFadeTable;
						d3dmm.lpvVertices = MM_Vertex;

						//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
						pa->RS.SetChanged();
						pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
						pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
						HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, (void *)&d3dmm, 4, wIndices, 5, D3DDP_MULTIMATRIX );
						ASSERT ( SUCCEEDED ( hres ) );



					}
#endif
#endif
				}
				else
#endif //#if USE_TOMS_ENGINE_PLEASE_BOB
				{
					if (FIGURE_alpha != 255)
					{
						POLY_Page[page].RS.SetTempTransparent();
					}
	#ifndef TARGET_DC
					else if (the_display.GetDeviceInfo()->AdamiLightingSupported() && !sw_hack)
	#else
					else if (the_display.GetDeviceInfo()->AdamiLightingSupported())
	#endif
					{
						// draw lighting quad
						POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
					}

					// draw texture quad
					POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
				}

				if (p_f4->FaceFlags & FACE_FLAG_TINT)
				{
					quad[0]->colour = qc0;
					quad[1]->colour = qc1;
					quad[2]->colour = qc2;
					quad[3]->colour = qc3;
				}
			}
			else
			{
				#if 0

				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f4->Col2].red;
				g = ENGINE_palette[p_f4->Col2].green;
				b = ENGINE_palette[p_f4->Col2].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);
#ifdef TARGET_DC
				face_colour |= 0xff000000;
#endif

				quad[0]->colour = face_colour;
				quad[1]->colour = face_colour;
				quad[2]->colour = face_colour;
				quad[3]->colour = face_colour;

				if (FIGURE_alpha != 255)
				{
					ASSERT(0);
				}

				POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));

#ifdef TARGET_DC
				colour |= 0xff000000;
#endif
				quad[0]->colour = colour;
				quad[1]->colour = colour;
				quad[2]->colour = colour;
				quad[3]->colour = colour;

				#endif
			}
		}
	}

	//
	// The triangles.
	//

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - sp;
		p1 = p_f3->Points[1] - sp;
		p2 = p_f3->Points[2] - sp;

/*
		p_f3->Bright[0]=store_dprod[p0];
		p_f3->Bright[1]=store_dprod[p1];
		p_f3->Bright[2]=store_dprod[p2];
*/
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

		tri[0] = &POLY_buffer[p0];
		tri[1] = &POLY_buffer[p1];
		tri[2] = &POLY_buffer[p2];

		if (POLY_valid_triangle(tri))
		{
			if (p_f3->DrawFlags & POLY_FLAG_TEXTURED)
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												       
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												       
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				if (p_f3->FaceFlags & FACE_FLAG_TINT)
				{
					qc0 = tri[0]->colour;
					qc1 = tri[1]->colour;
					qc2 = tri[2]->colour;

					tri[0]->colour &= colour_and;
					tri[1]->colour &= colour_and;
					tri[2]->colour &= colour_and;
				}

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;
				if(p_f3->FaceFlags&FACE_FLAG_THUG_JACKET)
				{
					switch(page)
					{
						case	64+21:
						case	10*64+2:
						case	10*64+32:
							page=jacket_lookup[0][GET_SKILL(p_thing)>>2];
							break;
						case	64+22:
						case	10*64+3:
						case	10*64+33:
							page=jacket_lookup[1][GET_SKILL(p_thing)>>2];
							break;
						case	64+24:
						case	10*64+4:
						case	10*64+36:
							page=jacket_lookup[2][GET_SKILL(p_thing)>>2];
							break;
						case	64+25:
						case	10*64+5:
						case	10*64+37:
							page=jacket_lookup[3][GET_SKILL(p_thing)>>2];
							break;
						default:
//							ASSERT(0);
							break;
					}
					page+=FACE_PAGE_OFFSET;
				}
				else
				if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
				{
					page=alt_texture[page-10*64]+tex_page_offset-1;
				}
				else
					page+=FACE_PAGE_OFFSET;
				//ASSERT(TEXTURE_dontexist[page]==0);

#ifndef TARGET_DC
				extern UBYTE sw_hack;
#endif



				PolyPage *pa = &(POLY_Page[page]);

#if USE_TOMS_ENGINE_PLEASE_BOB
				if ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) )
				{
					// Do an immediate render, not a deferred one.

					//
					// and render the polygons
					//

#if 0
					// TLVERTEX stuff.
					WORD wIndices[3] = {0,1,2};
					D3DTLVERTEX tlVertex[3];
					bool bBinned = FALSE;
					for ( int i = 0; i < 3; i++ )
					{
						tlVertex[i].dvSX = tri[i]->X * PolyPage::s_XScale;
						tlVertex[i].dvSY = tri[i]->Y * PolyPage::s_YScale;
						tlVertex[i].dvSZ = 1.0f - tri[i]->Z;
						tlVertex[i].dvTU = tri[i]->u;
						tlVertex[i].dvTV = tri[i]->v;
						tlVertex[i].dcColor = tri[i]->colour;
						tlVertex[i].dcSpecular = tri[i]->specular;
						tlVertex[i].dvRHW = tri[i]->Z;

						if ( ( tri[i]->Z <= 0.0f ) || ( tri[i]->Z >= 1.0f ) )
						{
							bBinned = TRUE;
						}
					}

					if ( !bBinned )
					{
						HRESULT hres;

						if ( FALSE && the_display.GetDeviceInfo()->AdamiLightingSupported())
						{
							// Draw lighting triangle
							PolyPage *tpa = &(POLY_Page[POLY_PAGE_COLOUR]);
							//tpa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
							tpa->RS.SetChanged();
							hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, tlVertex, 3, wIndices, 3, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
						}

						//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
						pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
						pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
						pa->RS.SetChanged();
						hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_TLVERTEX, tlVertex, 3, wIndices, 3, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
					}
#else

					static int iCount = 0;
					if ( FALSE )
					{
						// LVertex.
						WORD wIndices[3] = {0,1,2};
						D3DLVERTEX lVertex[3];
						for ( int i = 0; i < 3; i++ )
						{

							int pt = p_f3->Points[i];
							ASSERT(WITHIN(pt, sp, ep));

							lVertex[i].dvX = AENG_dx_prim_points[pt].X;
							lVertex[i].dvY = AENG_dx_prim_points[pt].Y;
							lVertex[i].dvZ = AENG_dx_prim_points[pt].Z;
							lVertex[i].dvTU = tri[i]->u;
							lVertex[i].dvTV = tri[i]->v;
							lVertex[i].dcColor = tri[i]->colour;
							lVertex[i].dcSpecular = tri[i]->specular;
						}
						//lVertex[2].dcColor = ( ( lVertex[2].dcColor + 0x00400000 ) & 0x00ff0000 ) | ( lVertex[2].dcColor & ~0x00ff0000 );

						HRESULT hres;

						if ( FALSE && the_display.GetDeviceInfo()->AdamiLightingSupported())
						{
							// Draw lighting triangle
							PolyPage *tpa = &(POLY_Page[POLY_PAGE_COLOUR]);
							//tpa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
							tpa->RS.SetChanged();
							hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, lVertex, 3, wIndices, 3, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
						}

						//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
						pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
						pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
						pa->RS.SetChanged();
						hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, lVertex, 3, wIndices, 3, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
					}
#ifdef TARGET_DC
					else
					{


						// Use the MM stuff.
						
						// Unlit untransformed vertices!
						WORD wIndices[4] = {0,1,2,-1};						
						for ( int i = 0; i < 3; i++ )
						{
							const float fNormScale = 1.0f / 256.0f;

							int pt = p_f3->Points[i];
							ASSERT(WITHIN(pt, sp, ep));

							MM_Vertex[i].dvX = AENG_dx_prim_points[pt].X;
							MM_Vertex[i].dvY = AENG_dx_prim_points[pt].Y;
							MM_Vertex[i].dvZ = AENG_dx_prim_points[pt].Z;
							MM_Vertex[i].dvNX = prim_normal[pt].X * fNormScale;
							MM_Vertex[i].dvNY = prim_normal[pt].Y * fNormScale;
							MM_Vertex[i].dvNZ = prim_normal[pt].Z * fNormScale;
							MM_Vertex[i].dvTU = tri[i]->u;
							MM_Vertex[i].dvTV = tri[i]->v;
							// This must be done after setting the rest up, since it blats the 12th byte of the vertex.
							SET_MM_INDEX ( (MM_Vertex[i]), 0 );
						}

#if 0
						if ( FALSE && the_display.GetDeviceInfo()->AdamiLightingSupported() )
						{
							// Draw lighting triangle
							PolyPage *tpa = &(POLY_Page[POLY_PAGE_COLOUR]);
							//tpa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
							tpa->RS.SetChanged();
							hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, MM_Vertex, 3, wIndices, 3, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTLIGHT );
						}
#endif

						D3DMULTIMATRIX d3dmm;
						d3dmm.lpd3dMatrices = MM_pMatrix;
						d3dmm.lpvLightDirs = MM_pNormal;
						d3dmm.lpLightTable = MM_pcFadeTable;
						d3dmm.lpvVertices = MM_Vertex;

						//pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
						pa->RS.SetChanged();
						pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
						pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
						HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive ( D3DPT_TRIANGLELIST, D3DFVF_VERTEX, (void *)&d3dmm, 3, wIndices, 4, D3DDP_MULTIMATRIX );
						ASSERT ( SUCCEEDED ( hres ) );

					}
#endif
#endif
				}
				else

#endif //#if USE_TOMS_ENGINE_PLEASE_BOB
				{
					if (FIGURE_alpha != 255)
					{
						POLY_Page[page].RS.SetTempTransparent();
					}


	#ifndef TARGET_DC
					else if (the_display.GetDeviceInfo()->AdamiLightingSupported() && !sw_hack)
	#else
					else if (the_display.GetDeviceInfo()->AdamiLightingSupported())
	#endif
					{
						// add lighting triangle
						POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
					}

					POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
				}


				if (p_f3->FaceFlags & FACE_FLAG_TINT)
				{
					tri[0]->colour = qc0;
					tri[1]->colour = qc1;
					tri[2]->colour = qc2;
				}
			}
			else
			{
				#if 0

				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f3->Col2].red;
				g = ENGINE_palette[p_f3->Col2].green;
				b = ENGINE_palette[p_f3->Col2].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

				tri[0]->colour = face_colour;
				tri[1]->colour = face_colour;
				tri[2]->colour = face_colour;

				if (FIGURE_alpha != 255)
				{
					ASSERT(0);
				}

				POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));

				tri[0]->colour = colour;
				tri[1]->colour = colour;
				tri[2]->colour = colour;

				#endif
			}
		}
	}
#endif //#else //#if USE_TOMS_ENGINE_PLEASE_BOB


#if USE_TOMS_ENGINE_PLEASE_BOB
	// Not done yet.
#else //#if USE_TOMS_ENGINE_PLEASE_BOB

	//
	// Environment mapping!
	//

	if (p_thing && p_thing->Class == CLASS_VEHICLE)
	{
		float nx;
		float ny;
		float nz;

		float dx;
		float dy;
		float dz;

		float comb[9];
		float cam_matrix[9];

		SLONG num_points = ep - sp;

		extern float AENG_cam_yaw;
		extern float AENG_cam_pitch;
		extern float AENG_cam_roll;

		MATRIX_calc(cam_matrix, AENG_cam_yaw, AENG_cam_pitch, AENG_cam_roll);
		MATRIX_3x3mul(comb, cam_matrix, fmatrix);

		//
		// Environment map the van. Work out the uv coords at all the points.
		//

		for (i = 0; i < num_points; i++)
		{
			nx = prim_normal[sp + i].X * (2.0F / 256.0F);
			ny = prim_normal[sp + i].Y * (2.0F / 256.0F);
			nz = prim_normal[sp + i].Z * (2.0F / 256.0F);

			MATRIX_MUL(
				comb,
				nx,
				ny,
				nz);

			dx = POLY_buffer[i].x;
			dy = POLY_buffer[i].y;
			dz = POLY_buffer[i].z;

			POLY_buffer[i].u = (nx * 0.5F) + 0.5F;
			POLY_buffer[i].v = (ny * 0.5F) + 0.5F;

			POLY_buffer[i].colour = 0xff888888;
		}

		//
		// Add the triangles and quads.
		//

		//
		// The quads.
		//

		for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
		{
			p_f4 = &prim_faces4[i];

			if (p_f4->FaceFlags & (FACE_FLAG_ENVMAP|FACE_FLAG_TINT))
			{
				p0 = p_f4->Points[0] - sp;
				p1 = p_f4->Points[1] - sp;
				p2 = p_f4->Points[2] - sp;
				p3 = p_f4->Points[3] - sp;
				
				ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

				quad[0] = &POLY_buffer[p0];
				quad[1] = &POLY_buffer[p1];
				quad[2] = &POLY_buffer[p2];
				quad[3] = &POLY_buffer[p3];

				if (POLY_valid_quad(quad))
				{
					if ((p_f4->FaceFlags & (FACE_FLAG_ENVMAP|FACE_FLAG_TINT)) == (FACE_FLAG_ENVMAP|FACE_FLAG_TINT))
					{
						page = POLY_PAGE_WINMAP;
					}
					else
					{
						page = POLY_PAGE_ENVMAP;
					}

					POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
				}
			}
		}

		//
		// The triangles.
		//

		for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
		{
			p_f3 = &prim_faces3[i];

			if (p_f3->FaceFlags & (FACE_FLAG_ENVMAP|FACE_FLAG_TINT))
			{
				p0 = p_f3->Points[0] - sp;
				p1 = p_f3->Points[1] - sp;
				p2 = p_f3->Points[2] - sp;
				
				ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
				ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

				tri[0] = &POLY_buffer[p0];
				tri[1] = &POLY_buffer[p1];
				tri[2] = &POLY_buffer[p2];

				if (POLY_valid_triangle(tri))
				{
					if ((p_f3->FaceFlags & (FACE_FLAG_ENVMAP|FACE_FLAG_TINT)) == (FACE_FLAG_ENVMAP|FACE_FLAG_TINT))
					{
						page = POLY_PAGE_WINMAP;
					}
					else
					{
						page = POLY_PAGE_ENVMAP;
					}

					POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
				}
			}
		}
	}
#endif //#else //#if USE_TOMS_ENGINE_PLEASE_BOB


#if USE_TOMS_ENGINE_PLEASE_BOB
	if ( !MM_bLightTableAlreadySetUp )
	{
#if 0
		// Clean up after ourselves.
		MM_pcFadeTable = NULL;
		MM_pcFadeTableTint = NULL;
		MM_pMatrix = NULL;
		MM_Vertex = NULL;
		MM_pNormal = NULL;
#endif
	}
#endif //#if USE_TOMS_ENGINE_PLEASE_BOB

	LOG_EXIT ( Figure_Draw_Prim_Tween )

}

void FIGURE_draw_prim_tween_warped(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG tween,
		struct GameKeyFrameElement *anim_info,
		struct GameKeyFrameElement *anim_info_next,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		ULONG colour,
		ULONG specular,
		Thing *p_thing)
{
	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	ULONG red;
	ULONG green;
	ULONG blue;
	ULONG r;
	ULONG g;
	ULONG b;
	ULONG face_colour;

	SLONG page;

	Matrix31  offset;
	Matrix33  mat2;
	float local_mat[9];
	float angle,sint,cost,tmpx,tmpz;

	SVector temp;
	SVector_F temp2;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	red   = (colour >> 16) & 0xff;
	green = (colour >>  8) & 0xff;
	blue  = (colour >>  0) & 0xff;

	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	offset.M[0] = anim_info->OffsetX + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8);
	offset.M[1] = anim_info->OffsetY + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8);
	offset.M[2] = anim_info->OffsetZ + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8);

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);

	SLONG	character_scale  = person_get_scale(p_thing);

	temp.X = (temp.X * character_scale) / 256;
	temp.Y = (temp.Y * character_scale) / 256;
	temp.Z = (temp.Z * character_scale) / 256;

	x += temp.X;
	y += temp.Y;
	z += temp.Z;	

	//
	// Create a temporary "tween" matrix between current and next
	//
	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	build_tween_matrix(&mat2, &m1, &m2, tween);
	normalise_matrix(&mat2);
	
	mat2.M[0][0] = (mat2.M[0][0] * character_scale) / 256;
	mat2.M[0][1] = (mat2.M[0][1] * character_scale) / 256;
	mat2.M[0][2] = (mat2.M[0][2] * character_scale) / 256;
	mat2.M[1][0] = (mat2.M[1][0] * character_scale) / 256;
	mat2.M[1][1] = (mat2.M[1][1] * character_scale) / 256;
	mat2.M[1][2] = (mat2.M[1][2] * character_scale) / 256;
	mat2.M[2][0] = (mat2.M[2][0] * character_scale) / 256;
	mat2.M[2][1] = (mat2.M[2][1] * character_scale) / 256;
	mat2.M[2][2] = (mat2.M[2][2] * character_scale) / 256;
	//
	// Apply local rotation matrix to get mat_final that rotates
	// the point into world space.
	//
//	matrix_mult33(&mat_final, rot_mat, &mat2);
	// ^- we don't do this.
	// instead we need to apply mat2 seperately, then apply the xform, then pass onwards

	//
	// Do everything in floats.
	//

	float off_x;
	float off_y;
	float off_z;
	float fmatrix[9];

	off_x = float(x);
	off_y = float(y);
	off_z = float(z);

	// scale!

	fmatrix[0] = float(rot_mat->M[0][0]) * (1.0F / 32768.0F);
	fmatrix[1] = float(rot_mat->M[0][1]) * (1.0F / 32768.0F);
	fmatrix[2] = float(rot_mat->M[0][2]) * (1.0F / 32768.0F);
	fmatrix[3] = float(rot_mat->M[1][0]) * (1.0F / 32768.0F);
	fmatrix[4] = float(rot_mat->M[1][1]) * (1.0F / 32768.0F);
	fmatrix[5] = float(rot_mat->M[1][2]) * (1.0F / 32768.0F);
	fmatrix[6] = float(rot_mat->M[2][0]) * (1.0F / 32768.0F);
	fmatrix[7] = float(rot_mat->M[2][1]) * (1.0F / 32768.0F);
	fmatrix[8] = float(rot_mat->M[2][2]) * (1.0F / 32768.0F);

	local_mat[0] = float(mat2.M[0][0]) * (1.0F / 32768.0F);
	local_mat[1] = float(mat2.M[0][1]) * (1.0F / 32768.0F);
	local_mat[2] = float(mat2.M[0][2]) * (1.0F / 32768.0F);
	local_mat[3] = float(mat2.M[1][0]) * (1.0F / 32768.0F);
	local_mat[4] = float(mat2.M[1][1]) * (1.0F / 32768.0F);
	local_mat[5] = float(mat2.M[1][2]) * (1.0F / 32768.0F);
	local_mat[6] = float(mat2.M[2][0]) * (1.0F / 32768.0F);
	local_mat[7] = float(mat2.M[2][1]) * (1.0F / 32768.0F);
	local_mat[8] = float(mat2.M[2][2]) * (1.0F / 32768.0F);

	POLY_set_local_rotation(
		off_x,
		off_y,
		off_z,
		fmatrix);

	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;

	for (i = sp; i < ep; i++)
	{
		ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

		temp2=AENG_dx_prim_points[i];


//		temp2.X-=offset.M[0];
//		temp2.Y-=offset.M[1];
//		temp2.Z-=offset.M[2];

		angle=(temp2.Z+offset.M[2])*2;
		angle=angle*PI/180;
		sint=sin(angle);
		cost=cos(angle);

		tmpx = (temp2.X*cost)+(temp2.Z*sint);
		tmpz =-(temp2.X*sint)+(temp2.Z*cost);
		temp2.X=tmpx; temp2.Z=tmpz;

//		temp2.X+=offset.M[0];
//		temp2.Y+=offset.M[1];
//		temp2.Z+=offset.M[2];




		// apply the local (tweening) matrix

		MATRIX_MUL(local_mat,temp2.X,temp2.Y,temp2.Z);

		// do some testy gnasty warp effect

		//...
/*		
		angle=(temp2.Z+offset.M[2])*2;
		angle=angle*PI/180;
		sint=sin(angle);
		cost=cos(angle);

		tmpx = (temp2.X*cost)+(temp2.Z*sint);
		tmpz =-(temp2.X*sint)+(temp2.Z*cost);
		temp2.X=tmpx; temp2.Z=tmpz;
  */
		//temp2.X+=(temp2.Z-off_z)/4.0;



		// do the global matrix (ofs_x/y/z, rot_mat and camera)

		pp = &POLY_buffer[POLY_buffer_upto++];

		POLY_transform_using_local_rotation(temp2.X, temp2.Y, temp2.Z, pp);

#ifdef TARGET_DC
		colour |= 0xff000000;
#endif
		pp->colour   = colour;
//		use_global_cloud(&pp->colour);
		pp->specular = specular;
	}

	//
	// The quads.
	//

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - sp;
		p1 = p_f4->Points[1] - sp;
		p2 = p_f4->Points[2] - sp;
		p3 = p_f4->Points[3] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

		quad[0] = &POLY_buffer[p0];
		quad[1] = &POLY_buffer[p1];
		quad[2] = &POLY_buffer[p2];
		quad[3] = &POLY_buffer[p3];

		if (POLY_valid_quad(quad))
		{
			if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
			{
				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);
												        
				quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
				quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);
												        
				quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
				quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

				quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
				quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

				page   = p_f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f4->TexturePage;
				page+=FACE_PAGE_OFFSET;


				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f4->Col2].red;
				g = ENGINE_palette[p_f4->Col2].green;
				b = ENGINE_palette[p_f4->Col2].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

#ifdef TARGET_DC
				face_colour |= 0xff000000;
				colour |= 0xff000000;
#endif
				quad[0]->colour = face_colour;
				quad[1]->colour = face_colour;
				quad[2]->colour = face_colour;
				quad[3]->colour = face_colour;

				POLY_add_quad(quad, POLY_PAGE_COLOUR, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));

				quad[0]->colour = colour;
				quad[1]->colour = colour;
				quad[2]->colour = colour;
				quad[3]->colour = colour;
			}
		}
	}

	//
	// The triangles.
	//

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - sp;
		p1 = p_f3->Points[1] - sp;
		p2 = p_f3->Points[2] - sp;
		
		ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
		ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

		tri[0] = &POLY_buffer[p0];
		tri[1] = &POLY_buffer[p1];
		tri[2] = &POLY_buffer[p2];

		if (POLY_valid_triangle(tri))
		{
			if (p_f3->DrawFlags & POLY_FLAG_TEXTURED)
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												       
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												       
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;
				page+=FACE_PAGE_OFFSET;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
			else
			{
				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f3->Col2].red;
				g = ENGINE_palette[p_f3->Col2].green;
				b = ENGINE_palette[p_f3->Col2].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

#ifdef TARGET_DC
				face_colour |= 0xff000000;
				colour |= 0xff000000;
#endif
				tri[0]->colour = face_colour;
				tri[1]->colour = face_colour;
				tri[2]->colour = face_colour;

				POLY_add_triangle(tri, POLY_PAGE_COLOUR, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));

				tri[0]->colour = colour;
				tri[1]->colour = colour;
				tri[2]->colour = colour;
			}
		}
	}
}

//*************************************************************************************************
//JCL - heirarchical bones drawing...


// global structure to prevent actual recursion...

#define MAX_RECURSION 5

struct structFIGURE_dhpr_data
{
	SLONG					start_object;
	BodyDef					*body_def;
	Matrix31				*world_pos;
	SLONG					tween;
	GameKeyFrameElement		*ae1;
	GameKeyFrameElement		*ae2;
	Matrix33				*world_mat;
	Matrix33				*world_mat2;
	SLONG					dx;
	SLONG					dy;
	SLONG					dz;
	ULONG					colour;
	ULONG					specular;
	BYTE					bPersonType;
	BYTE					bPersonID;
} FIGURE_dhpr_data;

#if 1

// Got to split this up - the compiler is getting very confused.
struct structFIGURE_dhpr_rdata1
{
	SLONG					part_number;
	SLONG					current_child_number;
	CMatrix33				*parent_base_mat;
	Matrix31				*parent_base_pos;
	Matrix33				*parent_current_mat;
	Matrix31				*parent_current_pos;
	Matrix31				pos;
	//Matrix31				end_pos;
	//Matrix33				end_mat;

} FIGURE_dhpr_rdata1[MAX_RECURSION];

struct structFIGURE_dhpr_rdata2
{
	//SLONG					part_number;
	//SLONG					current_child_number;
	//CMatrix33				*parent_base_mat;
	//Matrix31				*parent_base_pos;
	//Matrix33				*parent_current_mat;
	//Matrix31				*parent_current_pos;
	//Matrix31				pos;
	Matrix31				end_pos;
	Matrix33				end_mat;

} FIGURE_dhpr_rdata2[MAX_RECURSION];


#else
struct
{
	SLONG					part_number;
	CMatrix33				*parent_base_mat;
	Matrix31				*parent_base_pos;
	Matrix33				*parent_current_mat;
	Matrix31				*parent_current_pos;
	Matrix33				end_mat;
	Matrix31				end_pos;
	Matrix31				pos;

	SLONG					current_child_number;
} FIGURE_dhpr_rdata[MAX_RECURSION];
#endif


#define	PART_FACE		1
#define	PART_TROUSERS	2
#define	PART_JACKET		3
#define	PART_SHOES		4
#define	PART_PELVIS		5
#define	PART_HANDS		6

UBYTE	part_type[]=
{
	2,//5,//"pelvis",
	2,//"lfemur",
	2,//"ltibia",
	4,//"lfoot",
	3,//"torso",
	3,//"rhumorus",
	3,//"rradius",
	6,//"rhand",
	3,//"lhumorus",
	3,//"lradius",
	6,//"lhand",
	1,//"skull",
	2,//"rfemur",
	2,//"rtibia",
	4//"rfoot",
};


ULONG	local_seed;

SLONG	mandom(void)
{
	local_seed=local_seed*123456789+314159265;

	return(local_seed);

}

void	local_set_seed(SLONG seed)
{
	seed=local_seed;
}

ULONG jacket_col;
ULONG leg_col;





#if 1
// Optimised version


//volatile int m_iRecurseLevelDebugging;


// Set to 1 for the all-in-one method of drawing things.
#define DRAW_WHOLE_PERSON_AT_ONCE 1



#define MAX_NUM_BODY_PARTS_AT_ONCE 20



#if DRAW_WHOLE_PERSON_AT_ONCE
// Static arrays of the things we need for each part of the body.
ALIGNED_STATIC_ARRAY ( static D3DMATRIX *, MMBodyParts_pMatrix,	MAX_NUM_BODY_PARTS_AT_ONCE, D3DMATRIX, 32 );
ALIGNED_STATIC_ARRAY ( static float *, MMBodyParts_pNormal,		MAX_NUM_BODY_PARTS_AT_ONCE*4, float, 8 );

void FIGURE_draw_hierarchical_prim_recurse_individual_cull ( Thing *p_person );
#endif


#if DRAW_WHOLE_PERSON_AT_ONCE
void	FIGURE_draw_hierarchical_prim_recurse(Thing *p_person)
{
	SLONG	recurse_level = 0;
	SLONG	dx,dy,dz;
	UWORD f1,f2;
	SLONG	civ_flag=0,legs,body,shoes,face,hands,pelvis;
	//SLONG  limb;
	struct Matrix33 *rot_mat;


	LOG_ENTER ( Figure_Draw_Hierarchical )


#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB
	// Do I need to toggle inflation?
extern int g_iCheatNumber;
	// Well, 0x10f1a7e sort of spells "inflate" :-)
	if ( g_iCheatNumber == 0x10f1a7e )
	{
		if ( m_bPleaseInflatePeople )
		{
			CONSOLE_text("Looks like the inflation is wearing off now D'arci.");
			m_bPleaseInflatePeople = FALSE;
		}
		else
		{
			CONSOLE_text("The Illinois Enema Bandit has inflated everyone D'arci!");
			m_bPleaseInflatePeople = TRUE;
		}
		// And rebuild all prims.
		FIGURE_clean_all_LRU_slots();
		g_iCheatNumber = 0;
	}
#endif


	f1=p_person->Draw.Tweened->CurrentFrame->Flags;
	f2=p_person->Draw.Tweened->NextFrame->Flags;

	dx=((f1&0xe)<<(28))>>21;
	dx-=((f2&0xe)<<(28))>>21;

	dy=((f1&0xff00)<<(16))>>16;
	dy-=((f2&0xff00)<<(16))>>16;

	dz=((f1&0x70)<<(25))>>21;
	dz-=((f1&0x70)<<(25))>>21;


	ASSERT(p_person->Class == CLASS_PERSON);

	// Find the first "base" prim number and see if it's been created yet.
	// Eek - just thought - I hope none of these people share body parts, or it'll get very messy.
	// I should put some sort of ASSERT in to check that.

	// Ah - Mike thinks they probably do share stuff. Nads. Well, I only have a small cache, so I can just
	// do a brute-force search through it by body_def I guess. Or maybe just an array again.

	ASSERT ( recurse_level == 0 );
	ASSERT ( FIGURE_dhpr_rdata1[0].current_child_number == 0 );


	// Make the index of the thing.
	int iIndex = (int)( FIGURE_dhpr_data.bPersonType );
	ASSERT ( FIGURE_dhpr_data.bPersonType >= 0 );
	ASSERT ( FIGURE_dhpr_data.bPersonType < PERSON_NUM_TYPES );
	if ( FIGURE_dhpr_data.bPersonType == PERSON_CIV )
	{
		// Some variation of civs is done by using bPersonID.
		iIndex = (int)( FIGURE_dhpr_data.bPersonID & 0x1f ) + PERSON_NUM_TYPES;
	}
	else if ( FIGURE_dhpr_data.bPersonType == PERSON_ROPER )
	{
		// Treat each version of Roper (with each type of weapon) as a separate mesh, since
		// his weapons are not done the same as Darci or Mako. Hope this works.

		// Should be only 6 weapons: none, knife, bat, M16, pistols, shotty. Grenades are done a different way.
		// But "weapon" eight is his hip-flask, which he drinks from occasionally.
		ASSERT ( ( FIGURE_dhpr_data.bPersonID >> 5 ) < NUM_ROPERS_THINGIES );

		iIndex = (int)( FIGURE_dhpr_data.bPersonID >> 5 ) + PERSON_NUM_TYPES + 32;
	}


	// Save this for possible restoration later.
	structFIGURE_dhpr_rdata1	FIGURE_dhpr_rdata1_0_copy	= FIGURE_dhpr_rdata1[0];
	structFIGURE_dhpr_data		FIGURE_dhpr_data_copy		= FIGURE_dhpr_data_copy;


	ASSERT ( iIndex < MAX_NUMBER_D3D_PEOPLE );
	ASSERT ( ( PERSON_NUM_TYPES + 32 + NUM_ROPERS_THINGIES ) <= MAX_NUMBER_D3D_PEOPLE );
	TomsPrimObject *pPrimObj = &(D3DPeopleObj[iIndex]);
	if ( pPrimObj->wNumMaterials == 0 )
	{
		// Meshes have not been created yet - do so.
		
		// Set up the object.
#if 0
		// Use thrash slot 1 because the gun may bump it out later if it's
		// in slot 0!
		FIGURE_TPO_init_3d_object ( pPrimObj, 1 );
#else
		FIGURE_TPO_init_3d_object ( pPrimObj );
#endif
		int iTPOPartNumber = 0;



		recurse_level = 0;
		while (recurse_level >= 0)
		{
			//m_iRecurseLevelDebugging = recurse_level;
			structFIGURE_dhpr_rdata1 *pDHPR1	= FIGURE_dhpr_rdata1 + recurse_level;
			int iPartNumber = pDHPR1->part_number;
			if (pDHPR1->current_child_number == 0)
			{
				SLONG	body_part;
				// draw this level.

				ASSERT ( iPartNumber >= 0 );
				ASSERT ( iPartNumber <= 14 );

				body_part=FIGURE_dhpr_data.body_def->BodyPart[iPartNumber];

				// Draw thing with:
				SLONG prim = FIGURE_dhpr_data.start_object + body_part;

				// Add this prim to it.
				PrimObject  *p_obj = &prim_objects[prim];
				//FIGURE_TPO_add_prim_to_current_object ( p_obj, iTPOPartNumber );
				FIGURE_TPO_add_prim_to_current_object ( prim, iTPOPartNumber );
				iTPOPartNumber++;



				// Then draw the weapon & muzzle flash if present. However, these are 
				// not part of the compiled person - they will be drawn individually
				// after drawing the person.
			}

			// and do children

			if (body_part_children[iPartNumber][pDHPR1->current_child_number] != -1)
			{
				// Broken up, or the compiler gets very confused.
				structFIGURE_dhpr_rdata1 *pDHPR1Inc	= FIGURE_dhpr_rdata1 + recurse_level + 1;

				pDHPR1Inc->current_child_number = 0;

				ASSERT ( iPartNumber >= 0 );
				ASSERT ( iPartNumber <= 14 );
				pDHPR1Inc->part_number = body_part_children[iPartNumber][pDHPR1->current_child_number];

				pDHPR1Inc->current_child_number = 0;

				pDHPR1->current_child_number ++;


				recurse_level ++;
			}
			else
			{
				recurse_level --;
			}
		}

#if 0
		// Compile the whole object now.
		FIGURE_TPO_finish_3d_object ( pPrimObj );
#else
		// Compile the whole object now.
		// Use thrash slot 1 because the gun may bump it out later if it's
		// in slot 0!
		FIGURE_TPO_finish_3d_object ( pPrimObj, 1 );
#endif

		// Restore the saved data.
		FIGURE_dhpr_rdata1[0] = FIGURE_dhpr_rdata1_0_copy;
		FIGURE_dhpr_data_copy = FIGURE_dhpr_data_copy;


	}


	
	// Gets pre-incremented to 0 before use.
	int iTPOPartNumber = -1;
	bool bWholePersonVisible = TRUE;
	bool bBitsOfPersonVisible = FALSE;


	recurse_level = 0;
	while (recurse_level >= 0)
	{

		//m_iRecurseLevelDebugging = recurse_level;
		structFIGURE_dhpr_rdata1 *pDHPR1	= FIGURE_dhpr_rdata1 + recurse_level;
		int iPartNumber = pDHPR1->part_number;

		if (pDHPR1->current_child_number == 0)
		{
			{
				SLONG	body_part;
				SLONG	id;
				// draw this level.

				ASSERT ( iPartNumber >= 0 );
				ASSERT ( iPartNumber <= 14 );


				//limb=part_type[iPartNumber];


				body_part=FIGURE_dhpr_data.body_def->BodyPart[iPartNumber];
				rot_mat=FIGURE_dhpr_data.world_mat;

				iTPOPartNumber++;
				ASSERT ( iTPOPartNumber < MAX_NUM_BODY_PARTS_AT_ONCE );
				bool bVisible = FIGURE_draw_prim_tween_person_only_just_set_matrix (
										iTPOPartNumber,									
										FIGURE_dhpr_data.start_object + body_part,
										rot_mat,
										FIGURE_dhpr_data.dx+dx,
										FIGURE_dhpr_data.dy+dy,
										FIGURE_dhpr_data.dz+dz,
										recurse_level,
									    p_person
									);

				bWholePersonVisible &= bVisible;
				bBitsOfPersonVisible |= bVisible;

//
// draw a weapon in hand
//
				if(p_person->Genus.Person->PersonType!=PERSON_ROPER)
				if((id=(p_person->Draw.Tweened->PersonID>>5)))
				{

					SLONG	hand;
					if(id==2)
						hand=SUB_OBJECT_RIGHT_HAND;
					else
						hand=SUB_OBJECT_LEFT_HAND;

					if(iPartNumber==hand)
					{
						if(p_person->Draw.Tweened->Flags&DT_FLAG_GUNFLASH)
						{
							SLONG	prim;
							bool bDrawMuzzleFlash = FALSE;
							p_person->Draw.Tweened->Flags&=~DT_FLAG_GUNFLASH;
							switch(p_person->Draw.Tweened->PersonID>>5)
							{
								case	1:	// Pistol
									prim = 261;
									bDrawMuzzleFlash = TRUE;
									break;

								case	3:	// Shotgun
									prim = 262;
									bDrawMuzzleFlash = TRUE;
									break;

								case	5:	// AK
									prim = 263;
									bDrawMuzzleFlash = TRUE;
									break;

								default:
									break;
							}

							// Why does the DC hate GOTOs so much.
							if ( bDrawMuzzleFlash )
							{
#if 0
								FIGURE_draw_prim_tween( prim, 
														FIGURE_dhpr_data.world_pos->M[0],
														FIGURE_dhpr_data.world_pos->M[1],
														FIGURE_dhpr_data.world_pos->M[2],
														FIGURE_dhpr_data.tween,
													   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
													   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
														FIGURE_dhpr_data.world_mat,
														FIGURE_dhpr_data.dx+dx,
														FIGURE_dhpr_data.dy+dy,
														FIGURE_dhpr_data.dz+dz,
														FIGURE_dhpr_data.colour, 
														FIGURE_dhpr_data.specular,
														FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
														FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
														FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
														FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
													   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
													   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
														p_person,
														FIGURE_dhpr_rdata1[recurse_level].part_number);
#else
								// Muzzle flashes are always drawn "normally"
								FIGURE_draw_prim_tween_person_only(
														prim, 
														FIGURE_dhpr_data.world_mat,
														FIGURE_dhpr_data.dx+dx,
														FIGURE_dhpr_data.dy+dy,
														FIGURE_dhpr_data.dz+dz,
														recurse_level,
														p_person
														);
#endif
							}
						}


#if 0
						FIGURE_draw_prim_tween(
												255+(p_person->Draw.Tweened->PersonID>>5), 
												FIGURE_dhpr_data.world_pos->M[0],
												FIGURE_dhpr_data.world_pos->M[1],
												FIGURE_dhpr_data.world_pos->M[2],
												FIGURE_dhpr_data.tween,
											   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
											   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
												FIGURE_dhpr_data.world_mat,
												FIGURE_dhpr_data.dx+dx,
												FIGURE_dhpr_data.dy+dy,
												FIGURE_dhpr_data.dz+dz,
												FIGURE_dhpr_data.colour, 
												FIGURE_dhpr_data.specular,
												FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
												FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
												FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
												FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
											   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
											   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
												p_person,
												FIGURE_dhpr_rdata1[recurse_level].part_number);
#else
						// Weapons are always drawn "normally"
						FIGURE_draw_prim_tween_person_only(
												255+(p_person->Draw.Tweened->PersonID>>5), 
												FIGURE_dhpr_data.world_mat,
												FIGURE_dhpr_data.dx+dx,
												FIGURE_dhpr_data.dy+dy,
												FIGURE_dhpr_data.dz+dz,
												recurse_level,
												p_person
												);
#endif

					}

				}
			}
		}

		// and do children

		if (body_part_children[iPartNumber][pDHPR1->current_child_number] != -1)
		{

			// Broken up, or the compiler gets very confused.
			//structFIGURE_dhpr_rdata1 *pDHPR1	= FIGURE_dhpr_rdata1 + recurse_level;
			structFIGURE_dhpr_rdata1 *pDHPR1Inc	= FIGURE_dhpr_rdata1 + recurse_level + 1;

			pDHPR1Inc->current_child_number = 0;

			// really only need to do these next 3 lines once...
			pDHPR1->pos.M[0] = FIGURE_dhpr_data.ae1[iPartNumber].OffsetX;
			pDHPR1->pos.M[1] = FIGURE_dhpr_data.ae1[iPartNumber].OffsetY;
			pDHPR1->pos.M[2] = FIGURE_dhpr_data.ae1[iPartNumber].OffsetZ;

			ASSERT ( iPartNumber >= 0 );
			ASSERT ( iPartNumber <= 14 );
			pDHPR1Inc->part_number = body_part_children[iPartNumber][pDHPR1->current_child_number];
			CMatrix33 tmat;
			GetCMatrix(&FIGURE_dhpr_data.ae1[iPartNumber], &tmat);
			pDHPR1Inc->parent_base_mat = &tmat;
			pDHPR1Inc->parent_base_pos = &(pDHPR1->pos);
			pDHPR1Inc->parent_current_mat = &FIGURE_dhpr_rdata2[recurse_level].end_mat;
			pDHPR1Inc->parent_current_pos = &FIGURE_dhpr_rdata2[recurse_level].end_pos;
			pDHPR1Inc->current_child_number = 0;

			pDHPR1->current_child_number ++;
			recurse_level ++;
		}
		else
		{
			recurse_level --;
		}
	};


	// And now draw the whole person using the matrices and light vectors set up above.

	if ( !bWholePersonVisible )
	{
		if ( bBitsOfPersonVisible )
		{
			// Draw the person the slow way.
			TRACE ( "Partial person drawn\n" );
			// Restore the saved data.
			FIGURE_dhpr_rdata1[0] = FIGURE_dhpr_rdata1_0_copy;
			FIGURE_dhpr_data_copy = FIGURE_dhpr_data_copy;
			FIGURE_draw_hierarchical_prim_recurse_individual_cull ( p_person );
		}
		else
		{
			// None of the person is visible. Don't draw anything.
			//TRACE ( "Person not drawn\n" );
		}
		return;
	}

	//TRACE ( "Full person drawn\n" );


	// This many parts.
	iTPOPartNumber++;
	ASSERT ( iTPOPartNumber <= MAX_NUM_BODY_PARTS_AT_ONCE );




	SLONG face_colour;

	SLONG page;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;

	ULONG qc0;
	ULONG qc1;
	ULONG qc2;
	ULONG qc3;

	SVector temp;
	
	PrimFace4   *p_f4;
	PrimFace3   *p_f3;
	PrimObject  *p_obj;
	NIGHT_Found *nf;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];
	SLONG	tex_page_offset;


	LOG_ENTER ( Figure_Draw_Prim_Tween )

	tex_page_offset=p_person->Genus.Person->pcom_colour&0x3;


#if !USE_TOMS_ENGINE_PLEASE_BOB
#error Dont use this rout if USE_TOMS_ENGINE_PLEASE_BOB is not 1
#endif


	ASSERT ( MM_bLightTableAlreadySetUp );


	// The wonderful NEW system!


	LOG_ENTER ( Figure_Draw_Polys )

#if 1
	// The MM stuff doesn't like specular to be enabled.
	(the_display.lp_D3D_Device)->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, FALSE );
#endif


	// Tell the LRU cache we used this one.
	FIGURE_touch_LRU_of_object ( pPrimObj );


	ASSERT ( pPrimObj->pD3DVertices != NULL );
	ASSERT ( pPrimObj->pMaterials != NULL );
	ASSERT ( pPrimObj->pwListIndices != NULL );
	ASSERT ( pPrimObj->pwStripIndices != NULL );
	ASSERT ( pPrimObj->wNumMaterials != 0 );

	PrimObjectMaterial *pMat = pPrimObj->pMaterials;

	D3DMULTIMATRIX d3dmm;
	d3dmm.lpd3dMatrices = MMBodyParts_pMatrix;
	d3dmm.lpvLightDirs = MMBodyParts_pNormal;


	D3DVERTEX *pVertex = (D3DVERTEX *)pPrimObj->pD3DVertices;
	UWORD *pwListIndices = pPrimObj->pwListIndices;
	UWORD *pwStripIndices = pPrimObj->pwStripIndices;
	for ( int iMatNum = pPrimObj->wNumMaterials; iMatNum > 0; iMatNum-- )
	{
		// Set up the right texture for this material.

		UWORD wPage = pMat->wTexturePage;
		UWORD wRealPage = wPage & TEXTURE_PAGE_MASK;

		if ( wPage & TEXTURE_PAGE_FLAG_JACKET )
		{
			// Find the real jacket page.
			wRealPage = jacket_lookup [ wRealPage ][GET_SKILL(p_person)>>2];
			wRealPage += FACE_PAGE_OFFSET;
		}
		else if ( wPage & TEXTURE_PAGE_FLAG_OFFSET )
		{
			// An "offset" texture. This will be offset by a certain amount to
			// allow each prim to have different coloured clothes on.
			if ( tex_page_offset == 0 )
			{
				// No lookup offset.
				// This has not been offset yet.
				wRealPage += FACE_PAGE_OFFSET;
			}
			else
			{
				// Look this up.
				wRealPage = alt_texture[wRealPage-(10*64)]+tex_page_offset-1;
			}
		}

#ifdef DEBUG
		if ( wPage & TEXTURE_PAGE_FLAG_NOT_TEXTURED )
		{
			ASSERT ( wRealPage == POLY_PAGE_COLOUR );
		}
#endif

extern D3DMATRIX g_matWorld;

		PolyPage *pa = &(POLY_Page[wRealPage]);
#if 0
		// Near-plane culling! Must implement!

		// Not sure if I'm using character_scalef correctly...
		ASSERT ( ( character_scalef < 1.2f ) && ( character_scalef > 0.8f ) );
		ASSERT ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) );
		if ( ( ( ( g_matWorld._43 * 32768.0f ) - ( pPrimObj->fBoundingSphereRadius * character_scalef ) ) > ( POLY_ZCLIP_PLANE * 32768.0f ) ) )
#endif
		{
			// Non-alpha path.
			if ( wPage & TEXTURE_PAGE_FLAG_TINT )
			{
				// Tinted colours.
				d3dmm.lpLightTable = MM_pcFadeTableTint;
			}
			else
			{
				// Normal.
				d3dmm.lpLightTable = MM_pcFadeTable;
			}
			d3dmm.lpvVertices = pVertex;


#if 1



#ifdef DEBUG
static int iCounter = 0;
			if ( iCounter != 0 )
			{
				iCounter--;
				ASSERT ( iCounter != 0 );
			}
#endif

			// Fast as lightning.
			LOG_ENTER ( Figure_Set_RenderState )
			pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();
			LOG_EXIT ( Figure_Set_RenderState )
			LOG_ENTER ( Figure_DrawIndPrimMM )


			HRESULT hres;

#define SHOW_ME_FIGURE_DEBUGGING_PLEASE_BOB defined

#ifdef SHOW_ME_FIGURE_DEBUGGING_PLEASE_BOB
#ifdef DEBUG
#ifdef TARGET_DC
#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)
extern DIJOYSTATE the_state;
			bool bShowDebug = FALSE;
			if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) && BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) )
			{
				DWORD dwColour = (DWORD)pwStripIndices;
				dwColour = ( dwColour >> 2 ) ^ ( dwColour >> 6 ) ^ ( dwColour ) ^ ( dwColour << 3 );
				dwColour = ( dwColour << 9 ) ^ ( dwColour << 19 ) ^ ( dwColour ) ^ ( dwColour << 29 ) ^ ( dwColour >> 3 );
				dwColour &= 0x7f7f7f7f;
				for ( int i = 0; i < 128; i++ )
				{
					d3dmm.lpLightTable[i] = dwColour;
				}

				// And NULL texture (i.e. white).
				the_display.lp_D3D_Device->SetTexture ( 0, NULL );
			}
#endif
#endif
#endif



			//if (pMat->wNumVertices &&
			//	pMat->wNumStripIndices)
			{
				//TRACE ( "S4" );
				hres = DrawIndPrimMM (
						(the_display.lp_D3D_Device),
						D3DFVF_VERTEX,
						&d3dmm,
						pMat->wNumVertices,
						pwStripIndices,
						pMat->wNumStripIndices );
				//TRACE ( "F4" );
			}




#else

			// Do some performance tracing.
#ifndef DTRACE
#error Don't use this codepath unless DTRACING, fool!
#endif

			LOG_ENTER ( Figure_Set_RenderState )
			pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();

			WORD wTempVerts[4];
			wTempVerts[0] = 0;
			wTempVerts[1] = 1;
			wTempVerts[2] = 2;
			wTempVerts[3] = -1;
			(the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					3,
					wTempVerts,
					4,
					D3DDP_MULTIMATRIX );
			LOG_EXIT ( Figure_Set_RenderState )

			LOG_ENTER ( Figure_DrawIndPrimMM )
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			//TRACE("Drew %i vertices, %i indices\n", (int)( pMat->wNumVertices ), (int)( pMat->wNumStripIndices ) );

#endif

//			ASSERT ( SUCCEEDED ( hres ) );  //triggers all the time when inside on start of RTA
			LOG_EXIT ( Figure_DrawIndPrimMM )

		}
#if 0
		else
		{
			// Alpha/clipped path - do with standard non-MM calls.
			// FIXME. Needs to be done.

			// Actually, the fast-accept works very well, and it's only when the camera somehow gets REALLY close
			// that this happens. And actually a pop-reject seems a bit better than a clip. Certainly
			// there is no visually "right" thing to do. So leave it for now until someone complains. ATF.
			//TRACE ( "Tried to draw an alpha/clipped prim!" );
		}
#endif
		


		// Next material
		pVertex += pMat->wNumVertices;
		pwListIndices += pMat->wNumListIndices;
		pwStripIndices += pMat->wNumStripIndices;

		pMat++;
	}

#if 1
	// The MM stuff doesn't like specular to be enabled.
	(the_display.lp_D3D_Device)->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, TRUE );
#endif

	LOG_EXIT ( Figure_Draw_Polys )



	// No environment mapping.
	ASSERT ( p_person && ( p_person->Class != CLASS_VEHICLE ) );

	ASSERT ( MM_bLightTableAlreadySetUp );

	LOG_EXIT ( Figure_Draw_Prim_Tween )

#endif

	// Person drawn!

	LOG_EXIT ( Figure_Draw_Hierarchical )

}






// The slower version that can cull individual chunks.
#if DRAW_WHOLE_PERSON_AT_ONCE
void	FIGURE_draw_hierarchical_prim_recurse_individual_cull ( Thing *p_person )
#else
// There can be only one...
void	FIGURE_draw_hierarchical_prim_recurse ( Thing *p_person )
#endif
{
	SLONG	recurse_level = 0;
	SLONG	dx,dy,dz;
	UWORD f1,f2;
	SLONG	civ_flag=0,legs,body,shoes,face,hands,pelvis;
	//SLONG  limb;
	struct Matrix33 *rot_mat;


	LOG_ENTER ( Figure_Draw_Hierarchical )


#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB
	// Do I need to toggle inflation?
extern int g_iCheatNumber;
	// Well, 0x10f1a7e sort of spells "inflate" :-)
	if ( g_iCheatNumber == 0x10f1a7e )
	{
		if ( m_bPleaseInflatePeople )
		{
			CONSOLE_text("Looks like the inflation is wearing off now D'arci.");
			m_bPleaseInflatePeople = FALSE;
		}
		else
		{
			CONSOLE_text("The Illinois Enema Bandit has inflated everyone D'arci!");
			m_bPleaseInflatePeople = TRUE;
		}
		// And rebuild all prims.
		FIGURE_clean_all_LRU_slots();
		g_iCheatNumber = 0;
	}
#endif


	f1=p_person->Draw.Tweened->CurrentFrame->Flags;
	f2=p_person->Draw.Tweened->NextFrame->Flags;

	dx=((f1&0xe)<<(28))>>21;
	dx-=((f2&0xe)<<(28))>>21;

	dy=((f1&0xff00)<<(16))>>16;
	dy-=((f2&0xff00)<<(16))>>16;

	dz=((f1&0x70)<<(25))>>21;
	dz-=((f1&0x70)<<(25))>>21;


	ASSERT(p_person->Class == CLASS_PERSON);

	recurse_level = 0;
	while (recurse_level >= 0)
	{

		//m_iRecurseLevelDebugging = recurse_level;
		structFIGURE_dhpr_rdata1 *pDHPR1	= FIGURE_dhpr_rdata1 + recurse_level;
		int iPartNumber = pDHPR1->part_number;

		if (pDHPR1->current_child_number == 0)
		{
			{
				SLONG	body_part;
				SLONG	id;
				// draw this level.

				ASSERT ( iPartNumber >= 0 );
				ASSERT ( iPartNumber <= 14 );


				//limb=part_type[iPartNumber];


				body_part=FIGURE_dhpr_data.body_def->BodyPart[iPartNumber];
				rot_mat=FIGURE_dhpr_data.world_mat;

#if 0
				FIGURE_draw_prim_tween( FIGURE_dhpr_data.start_object + body_part, //FIGURE_dhpr_data.body_def->BodyPart[FIGURE_dhpr_rdata1[recurse_level].part_number],
										FIGURE_dhpr_data.world_pos->M[0],
										FIGURE_dhpr_data.world_pos->M[1],
										FIGURE_dhpr_data.world_pos->M[2],
										FIGURE_dhpr_data.tween,
									   &FIGURE_dhpr_data.ae1[pDHPR1->part_number],
									   &FIGURE_dhpr_data.ae2[pDHPR1->part_number],
										rot_mat,
										FIGURE_dhpr_data.dx+dx,
										FIGURE_dhpr_data.dy+dy,
										FIGURE_dhpr_data.dz+dz,
										FIGURE_dhpr_data.colour, //limb==PART_JACKET?jacket_col:(limb==PART_TROUSERS?leg_col:FIGURE_dhpr_data.colour), 
										FIGURE_dhpr_data.specular,
										FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
										FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
										FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
										FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
									   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
									   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
									    p_person,
									    FIGURE_dhpr_rdata1[recurse_level].part_number);
#else
				FIGURE_draw_prim_tween_person_only (
										FIGURE_dhpr_data.start_object + body_part, //FIGURE_dhpr_data.body_def->BodyPart[FIGURE_dhpr_rdata1[recurse_level].part_number],
										//FIGURE_dhpr_data.world_pos->M[0],
										//FIGURE_dhpr_data.world_pos->M[1],
										//FIGURE_dhpr_data.world_pos->M[2],
										//FIGURE_dhpr_data.tween,
									   //&FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
									   //&FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
										rot_mat,
										FIGURE_dhpr_data.dx+dx,
										FIGURE_dhpr_data.dy+dy,
										FIGURE_dhpr_data.dz+dz,
										//FIGURE_dhpr_data.colour, //limb==PART_JACKET?jacket_col:(limb==PART_TROUSERS?leg_col:FIGURE_dhpr_data.colour), 
										//FIGURE_dhpr_data.specular,
										recurse_level,
										//FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
										//FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
										//FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
										//FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
									   //&FIGURE_dhpr_rdata2[recurse_level].end_mat,
									   //&FIGURE_dhpr_rdata2[recurse_level].end_pos,
									    p_person
									    //FIGURE_dhpr_rdata1[recurse_level].part_number
									);
#endif

//
// draw a weapon in hand
//
				if(p_person->Genus.Person->PersonType!=PERSON_ROPER)
				if((id=(p_person->Draw.Tweened->PersonID>>5)))
				{

					SLONG	hand;
					if(id==2)
						hand=SUB_OBJECT_RIGHT_HAND;
					else
						hand=SUB_OBJECT_LEFT_HAND;

					if(iPartNumber==hand)
					{
						if(p_person->Draw.Tweened->Flags&DT_FLAG_GUNFLASH)
						{
							SLONG	prim;
							bool bDrawMuzzleFlash = FALSE;
							p_person->Draw.Tweened->Flags&=~DT_FLAG_GUNFLASH;
							switch(p_person->Draw.Tweened->PersonID>>5)
							{
								case	1:	// Pistol
									prim = 261;
									bDrawMuzzleFlash = TRUE;
									break;

								case	3:	// Shotgun
									prim = 262;
									bDrawMuzzleFlash = TRUE;
									break;

								case	5:	// AK
									prim = 263;
									bDrawMuzzleFlash = TRUE;
									break;

								default:
									break;
							}

							// Why does the DC hate GOTOs so much.
							if ( bDrawMuzzleFlash )
							{
#if 0
								FIGURE_draw_prim_tween( prim, 
														FIGURE_dhpr_data.world_pos->M[0],
														FIGURE_dhpr_data.world_pos->M[1],
														FIGURE_dhpr_data.world_pos->M[2],
														FIGURE_dhpr_data.tween,
													   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
													   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
														FIGURE_dhpr_data.world_mat,
														FIGURE_dhpr_data.dx+dx,
														FIGURE_dhpr_data.dy+dy,
														FIGURE_dhpr_data.dz+dz,
														FIGURE_dhpr_data.colour, 
														FIGURE_dhpr_data.specular,
														FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
														FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
														FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
														FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
													   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
													   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
														p_person,
														FIGURE_dhpr_rdata1[recurse_level].part_number);
#else
								// Muzzle flashes are always drawn "normally"
								FIGURE_draw_prim_tween_person_only(
														prim, 
														//FIGURE_dhpr_data.world_pos->M[0],
														//FIGURE_dhpr_data.world_pos->M[1],
														//FIGURE_dhpr_data.world_pos->M[2],
														//FIGURE_dhpr_data.tween,
													   //&FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
													   //&FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
														FIGURE_dhpr_data.world_mat,
														FIGURE_dhpr_data.dx+dx,
														FIGURE_dhpr_data.dy+dy,
														FIGURE_dhpr_data.dz+dz,
														//FIGURE_dhpr_data.colour, 
														//FIGURE_dhpr_data.specular,
														recurse_level,
														//FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
														//FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
														//FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
														//FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
													   //&FIGURE_dhpr_rdata2[recurse_level].end_mat,
													   //&FIGURE_dhpr_rdata2[recurse_level].end_pos,
														p_person
														//FIGURE_dhpr_rdata1[recurse_level].part_number
														);
#endif
							}
						}


#if 0
						FIGURE_draw_prim_tween(
												255+(p_person->Draw.Tweened->PersonID>>5), 
												FIGURE_dhpr_data.world_pos->M[0],
												FIGURE_dhpr_data.world_pos->M[1],
												FIGURE_dhpr_data.world_pos->M[2],
												FIGURE_dhpr_data.tween,
											   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
											   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
												FIGURE_dhpr_data.world_mat,
												FIGURE_dhpr_data.dx+dx,
												FIGURE_dhpr_data.dy+dy,
												FIGURE_dhpr_data.dz+dz,
												FIGURE_dhpr_data.colour, 
												FIGURE_dhpr_data.specular,
												FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
												FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
												FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
												FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
											   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
											   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
												p_person,
												FIGURE_dhpr_rdata1[recurse_level].part_number);
#else
						// Weapons are always drawn "normally"
						FIGURE_draw_prim_tween_person_only(
												255+(p_person->Draw.Tweened->PersonID>>5), 
												//FIGURE_dhpr_data.world_pos->M[0],
												//FIGURE_dhpr_data.world_pos->M[1],
												//FIGURE_dhpr_data.world_pos->M[2],
												//FIGURE_dhpr_data.tween,
											   //&FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
											   //&FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
												FIGURE_dhpr_data.world_mat,
												FIGURE_dhpr_data.dx+dx,
												FIGURE_dhpr_data.dy+dy,
												FIGURE_dhpr_data.dz+dz,
												//FIGURE_dhpr_data.colour, 
												//FIGURE_dhpr_data.specular,
												recurse_level,
												//FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
												//FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
												//FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
												//FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
											   //&FIGURE_dhpr_rdata2[recurse_level].end_mat,
											   //&FIGURE_dhpr_rdata2[recurse_level].end_pos,
												p_person
												//FIGURE_dhpr_rdata1[recurse_level].part_number
												);
#endif

					}

				}
			}
		}

		// and do children

		if (body_part_children[iPartNumber][pDHPR1->current_child_number] != -1)
		{

#if 0
			FIGURE_dhpr_rdata1[recurse_level + 1].current_child_number = 0;

			// really only need to do these next 3 lines once...
			FIGURE_dhpr_rdata1[recurse_level].pos.M[0] = FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number].OffsetX;
			FIGURE_dhpr_rdata1[recurse_level].pos.M[1] = FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number].OffsetY;
			FIGURE_dhpr_rdata1[recurse_level].pos.M[2] = FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number].OffsetZ;
			ASSERT ( FIGURE_dhpr_rdata1[recurse_level].part_number >= 0 );
			ASSERT ( FIGURE_dhpr_rdata1[recurse_level].part_number <= 14 );
			FIGURE_dhpr_rdata1[recurse_level + 1].part_number = body_part_children[FIGURE_dhpr_rdata1[recurse_level].part_number][FIGURE_dhpr_rdata1[recurse_level].current_child_number];
			CMatrix33 tmat;
			GetCMatrix(&FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number], &tmat);
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_base_mat = &tmat;
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_base_pos = &FIGURE_dhpr_rdata1[recurse_level].pos;
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_current_mat = &FIGURE_dhpr_rdata2[recurse_level].end_mat;
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_current_pos = &FIGURE_dhpr_rdata2[recurse_level].end_pos;
			FIGURE_dhpr_rdata1[recurse_level + 1].current_child_number = 0;

			FIGURE_dhpr_rdata1[recurse_level].current_child_number ++ ;
#else
			// Broken up, or the compiler gets very confused.
			//structFIGURE_dhpr_rdata1 *pDHPR1	= FIGURE_dhpr_rdata1 + recurse_level;
			structFIGURE_dhpr_rdata1 *pDHPR1Inc	= FIGURE_dhpr_rdata1 + recurse_level + 1;

			pDHPR1Inc->current_child_number = 0;

			// really only need to do these next 3 lines once...
			pDHPR1->pos.M[0] = FIGURE_dhpr_data.ae1[iPartNumber].OffsetX;
			pDHPR1->pos.M[1] = FIGURE_dhpr_data.ae1[iPartNumber].OffsetY;
			pDHPR1->pos.M[2] = FIGURE_dhpr_data.ae1[iPartNumber].OffsetZ;

			ASSERT ( iPartNumber >= 0 );
			ASSERT ( iPartNumber <= 14 );
			pDHPR1Inc->part_number = body_part_children[iPartNumber][pDHPR1->current_child_number];
			CMatrix33 tmat;
			GetCMatrix(&FIGURE_dhpr_data.ae1[iPartNumber], &tmat);
			pDHPR1Inc->parent_base_mat = &tmat;
			pDHPR1Inc->parent_base_pos = &(pDHPR1->pos);
			pDHPR1Inc->parent_current_mat = &FIGURE_dhpr_rdata2[recurse_level].end_mat;
			pDHPR1Inc->parent_current_pos = &FIGURE_dhpr_rdata2[recurse_level].end_pos;
			pDHPR1Inc->current_child_number = 0;

			pDHPR1->current_child_number ++;
#endif
			recurse_level ++;
		}
		else
		{
			recurse_level --;
		}
	};


	// Person drawn!

	LOG_EXIT ( Figure_Draw_Hierarchical )

}




#else
// Old version
void	FIGURE_draw_hierarchical_prim_recurse(Thing *p_person)
{
	SLONG	recurse_level = 0;
	SLONG	dx,dy,dz;
	UWORD f1,f2;
	SLONG	civ_flag=0,legs,body,shoes,face,hands,pelvis;
	//SLONG limb;
	struct Matrix33 *rot_mat;




#ifdef HIGH_REZ_PEOPLE_PLEASE_BOB
	// Do I need to toggle inflation?
extern int g_iCheatNumber;
	// Well, 0x10f1a7e sort of spells "inflate" :-)
	if ( g_iCheatNumber == 0x10f1a7e )
	{
		if ( m_bPleaseInflatePeople )
		{
			CONSOLE_text("Looks like the inflation is wearing off now D'arci.");
			m_bPleaseInflatePeople = FALSE;
		}
		else
		{
			CONSOLE_text("The Illinois Enema Bandit has inflated everyone D'arci!");
			m_bPleaseInflatePeople = TRUE;
		}
		// And rebuild all prims.
		FIGURE_clean_all_LRU_slots();
		g_iCheatNumber = 0;
	}
#endif





	LOG_ENTER ( Figure_Draw_Hierarchical )

	f1=p_person->Draw.Tweened->CurrentFrame->Flags;
	f2=p_person->Draw.Tweened->NextFrame->Flags;

	dx=((f1&0xe)<<(28))>>21;
	dx-=((f2&0xe)<<(28))>>21;

	dy=((f1&0xff00)<<(16))>>16;
	dy-=((f2&0xff00)<<(16))>>16;

	dz=((f1&0x70)<<(25))>>21;
	dz-=((f1&0x70)<<(25))>>21;
/*
	if(p_person->Genus.Person->PersonType==PERSON_CIV ||p_person->Genus.Person->PersonType==PERSON_CIV2)
	{
		local_set_seed(THING_NUMBER(p_person));
		legs=mandom()&3;
		body=mandom()&3;
		shoes=mandom()&3;
		face=mandom()&3;
		hands=mandom()&1;
		pelvis=legs; //Random()&3;
		if (GAME_FLAGS & GF_RAINING)
		{
			pelvis=(mandom()%3)+4;
			body=pelvis;
		}

		civ_flag=1;

	}
*/
	ASSERT(p_person->Class == CLASS_PERSON);

	while (recurse_level >= 0)
	{
		if (FIGURE_dhpr_rdata1[recurse_level].current_child_number == 0)
		{
			{
				SLONG	body_part;
				SLONG	id;
				// draw this level.
/*
				if(civ_flag)
				{
					switch(part_type[FIGURE_dhpr_rdata1[recurse_level].part_number])
					{
						case	PART_FACE:
							body_part=face;
							break;
						case	PART_HANDS:
							body_part=hands;
							break;
						case	PART_TROUSERS:
							body_part=legs;
							break;
						case	PART_JACKET:
							body_part=body;
							break;
						case	PART_SHOES:
							body_part=shoes;
							break;
						case	PART_PELVIS:
							body_part=pelvis;
							break;
						default:
							ASSERT(0);
							break;

					}
					body_part=0;
				}
				else
				{
					body_part=FIGURE_dhpr_data.body_def->BodyPart[FIGURE_dhpr_rdata1[recurse_level].part_number];
				}
*/
				//limb=part_type[FIGURE_dhpr_rdata1[recurse_level].part_number];

				body_part=FIGURE_dhpr_data.body_def->BodyPart[FIGURE_dhpr_rdata1[recurse_level].part_number];
/*				if(  (p_person->Flags&FLAGS_PERSON_AIM_AND_RUN) && body_part_upper[FIGURE_dhpr_rdata1[recurse_level].part_number])
				{
					rot_mat=FIGURE_dhpr_data.world_mat2;
				}
				else
*/
				{
					rot_mat=FIGURE_dhpr_data.world_mat;
				}

				FIGURE_draw_prim_tween( FIGURE_dhpr_data.start_object + body_part, //FIGURE_dhpr_data.body_def->BodyPart[FIGURE_dhpr_rdata1[recurse_level].part_number],
										FIGURE_dhpr_data.world_pos->M[0],
										FIGURE_dhpr_data.world_pos->M[1],
										FIGURE_dhpr_data.world_pos->M[2],
										FIGURE_dhpr_data.tween,
									   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
									   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
										rot_mat,
										FIGURE_dhpr_data.dx+dx,
										FIGURE_dhpr_data.dy+dy,
										FIGURE_dhpr_data.dz+dz,
										FIGURE_dhpr_data.colour, //limb==PART_JACKET?jacket_col:(limb==PART_TROUSERS?leg_col:FIGURE_dhpr_data.colour), 
										FIGURE_dhpr_data.specular,
										FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
										FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
										FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
										FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
									   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
									   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
									    p_person,
									    FIGURE_dhpr_rdata1[recurse_level].part_number);

//
// draw a weapon in hand
//
				if(p_person->Genus.Person->PersonType!=PERSON_ROPER)
				if((id=(p_person->Draw.Tweened->PersonID>>5)))
				{

					SLONG	hand;
					if(id==2)
						hand=SUB_OBJECT_RIGHT_HAND;
					else
						hand=SUB_OBJECT_LEFT_HAND;

					if(FIGURE_dhpr_rdata1[recurse_level].part_number==hand)
					{
						if(p_person->Draw.Tweened->Flags&DT_FLAG_GUNFLASH)
						{
							SLONG	prim;
							p_person->Draw.Tweened->Flags&=~DT_FLAG_GUNFLASH;
							switch(p_person->Draw.Tweened->PersonID>>5)
							{
								case	1:	// Pistol
									prim = 261;
									break;

								case	3:	// Shotgun
									prim = 262;
									break;

								case	5:	// AK
									prim = 263;
									break;

								default:
									goto dont_draw_the_muzzle_flash;
									break;
							}

							FIGURE_draw_prim_tween( prim, 
													FIGURE_dhpr_data.world_pos->M[0],
													FIGURE_dhpr_data.world_pos->M[1],
													FIGURE_dhpr_data.world_pos->M[2],
													FIGURE_dhpr_data.tween,
												   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
												   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
													FIGURE_dhpr_data.world_mat,
													FIGURE_dhpr_data.dx+dx,
													FIGURE_dhpr_data.dy+dy,
													FIGURE_dhpr_data.dz+dz,
													FIGURE_dhpr_data.colour, 
													FIGURE_dhpr_data.specular,
													FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
													FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
													FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
													FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
												   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
												   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
													p_person,
													FIGURE_dhpr_rdata1[recurse_level].part_number);

						  dont_draw_the_muzzle_flash:;

						}


						FIGURE_draw_prim_tween( 255+(p_person->Draw.Tweened->PersonID>>5), 
												FIGURE_dhpr_data.world_pos->M[0],
												FIGURE_dhpr_data.world_pos->M[1],
												FIGURE_dhpr_data.world_pos->M[2],
												FIGURE_dhpr_data.tween,
											   &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number],
											   &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number],
												FIGURE_dhpr_data.world_mat,
												FIGURE_dhpr_data.dx+dx,
												FIGURE_dhpr_data.dy+dy,
												FIGURE_dhpr_data.dz+dz,
												FIGURE_dhpr_data.colour, 
												FIGURE_dhpr_data.specular,
												FIGURE_dhpr_rdata1[recurse_level].parent_base_mat,
												FIGURE_dhpr_rdata1[recurse_level].parent_base_pos,
												FIGURE_dhpr_rdata1[recurse_level].parent_current_mat,
												FIGURE_dhpr_rdata1[recurse_level].parent_current_pos,
											   &FIGURE_dhpr_rdata2[recurse_level].end_mat,
											   &FIGURE_dhpr_rdata2[recurse_level].end_pos,
												p_person,
												FIGURE_dhpr_rdata1[recurse_level].part_number);

					}

				}
			}
		}

		// and do children

		if (body_part_children[FIGURE_dhpr_rdata1[recurse_level].part_number][FIGURE_dhpr_rdata1[recurse_level].current_child_number] != -1)
		{
			FIGURE_dhpr_rdata1[recurse_level + 1].current_child_number = 0;

			// really only need to do these next 3 lines once...
			FIGURE_dhpr_rdata1[recurse_level].pos.M[0] = FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number].OffsetX;
			FIGURE_dhpr_rdata1[recurse_level].pos.M[1] = FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number].OffsetY;
			FIGURE_dhpr_rdata1[recurse_level].pos.M[2] = FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number].OffsetZ;

			FIGURE_dhpr_rdata1[recurse_level + 1].part_number = body_part_children[FIGURE_dhpr_rdata1[recurse_level].part_number][FIGURE_dhpr_rdata1[recurse_level].current_child_number];
			CMatrix33 tmat;
			GetCMatrix(&FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number], &tmat);
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_base_mat = &tmat;
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_base_pos = &FIGURE_dhpr_rdata1[recurse_level].pos;
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_current_mat = &FIGURE_dhpr_rdata2[recurse_level].end_mat;
			FIGURE_dhpr_rdata1[recurse_level + 1].parent_current_pos = &FIGURE_dhpr_rdata2[recurse_level].end_pos;
			FIGURE_dhpr_rdata1[recurse_level + 1].current_child_number = 0;

			FIGURE_dhpr_rdata1[recurse_level].current_child_number ++ ;
			recurse_level ++;
		}
		else
			recurse_level --;
	};

	LOG_EXIT ( Figure_Draw_Hierarchical )

}
#endif


//*************************************************************************************************
SLONG get_sort_z_bodge(SLONG px,SLONG pz)
{
	SLONG	dx,dz;
	UBYTE	cap;
	dx=px-POLY_cam_x;
	dz=pz-POLY_cam_z;

	if(abs(dz)<abs(dx))
	{
		if(dx<0)
		{
			if((px&0xff)>128)
			{
				cap=MAV_get_caps(px>>8,pz>>8,MAV_DIR_XL);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(100);
				}
			}

		}
		else
		{
			if((px&0xff)<128)
			{
				cap=MAV_get_caps(px>>8,pz>>8,MAV_DIR_XS);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(100);
				}
			}

		}
		
	}
	else
	{
		if(dz<0)
		{
			if((pz&0xff)>128)
			{
				cap=MAV_get_caps(px>>8,pz>>8,MAV_DIR_ZL);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(100);
				}
			}

		}
		else
		{
			if((pz&0xff)<128)
			{
				cap=MAV_get_caps(px>>8,pz>>8,MAV_DIR_ZS);
				if(!(cap&MAV_CAPS_GOTO))
				{
					return(100);
				}
			}

		}
		
	}
	return(0);

}

struct	
{
	SWORD	r,g,b;
}peep_recol[]=
{
	{12,64,64},
	{64,28,64},
	{64,4,55},
	{54,32,22},

	{42,14,12},
	{32,42,64},
	{22,64,54},
	{64,14,32},

	{34,32,64},
	{0,32,64},
	{32,64,0},
	{64,0,32},

	{56,30,0},
	{0,40,56},
	{33,26,70},
	{56,36,0},


};

UBYTE kludge_shrink;







#ifndef PSX
// New faster version
void FIGURE_draw(Thing *p_thing)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG	wx,wy,wz;

	ULONG colour;
	ULONG specular;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;

	LOG_ENTER ( Figure_FIGURE_Draw )

//	calc_global_cloud(p_thing->WorldPos.X>>8,p_thing->WorldPos.Y>>8,p_thing->WorldPos.Z>>8);

	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		LOG_EXIT ( Figure_FIGURE_Draw )
		return;
	}

	//
	// The offset to keep the locked limb in the same place.
	//

	dx = 0;
	dy = 0;
	dz = 0;

	//
	// The rotation matrix of the whole object.
	//

	LOG_ENTER ( Figure_Rotate )

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		(dt->Roll+2048)&2047,
	   &r_matrix);

	LOG_EXIT ( Figure_Rotate )

	wx=0;
	wy=0;
	wz=0;

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

#ifdef DEBUG
	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");
		LOG_EXIT ( Figure_FIGURE_Draw )

		return;
	}
#endif

	//
	// What colour do we draw the figure?
	//

	//
	// All this is done in FIGURE_draw_prim_tween nowadays using the array
	// filled in by this call to NIGHT_find....
	//

	colour   = 0;
	specular = 0;

	SLONG lx;
	SLONG ly;
	SLONG lz;

	NIGHT_Colour col;

	LOG_ENTER ( Figure_Calc_Sub_Objs )

	calc_sub_objects_position(
		p_thing,
		dt->AnimTween,
		0,	// 0 is Pelvis
		&lx, &ly, &lz);

	lx += p_thing->WorldPos.X >> 8;
	ly += p_thing->WorldPos.Y >> 8;
	lz += p_thing->WorldPos.Z >> 8;

	LOG_EXIT ( Figure_Calc_Sub_Objs )

	LOG_ENTER ( Figure_NIGHT_Find )

	NIGHT_find(lx, ly, lz);

	LOG_EXIT ( Figure_NIGHT_Find )


#if USE_TOMS_ENGINE_PLEASE_BOB

	ASSERT ( !MM_bLightTableAlreadySetUp );

	// Set up some data for the MM rendering thing.

#if 0
//#define ALIGNED_STATIC_ARRAY(name,number,mytype,align)														\
//	static char c##name##mytype##align##StaticArray [ align + number * sizeof ( mytype ) ];						\
//	name = (mytype *)( ( (DWORD)c##name##mytype##align##StaticArray + (align-1) ) & ~(align-1) )

	ALIGNED_STATIC_ARRAY ( MM_pcFadeTable, 128, D3DCOLOR, 4 );
	ALIGNED_STATIC_ARRAY ( MM_pcFadeTableTint, 128, D3DCOLOR, 4 );
	ALIGNED_STATIC_ARRAY ( MM_pMatrix, 1, D3DMATRIX, 32 );
	ALIGNED_STATIC_ARRAY ( MM_Vertex, 4, D3DVERTEX, 32 );
	ALIGNED_STATIC_ARRAY ( MM_pNormal, 4, float, 8 );
#endif

//#undef ALIGNED_STATIC_ARRAY

	Pyro *p = NULL;
	if (p_thing->Class == CLASS_PERSON && p_thing->Genus.Person->BurnIndex)
	{
		p = TO_PYRO(p_thing->Genus.Person->BurnIndex-1);
		if (p->PyroType != PYRO_IMMOLATE)
		{
			p = NULL;
		}
	}

	BuildMMLightingTable ( p, 0 );

	MM_bLightTableAlreadySetUp = TRUE;


#endif



	//
	// Draw each body part.
	//

	//SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[dt->MeshID]].StartObject;

	if (ele_count == 15 )
	{
		Matrix31	pos;
		pos.M[0] =  (p_thing->WorldPos.X >> 8)+wx;
		pos.M[1] = (p_thing->WorldPos.Y >> 8) + wy;
		pos.M[2] =  (p_thing->WorldPos.Z >> 8)+wz;

		// assume we've got a person, and draw the object hierarchically

		// fill out data.

		FIGURE_dhpr_rdata1[0].part_number			= 0;
		FIGURE_dhpr_rdata1[0].current_child_number	= 0;
		FIGURE_dhpr_rdata1[0].parent_base_mat		= NULL;
		FIGURE_dhpr_rdata1[0].parent_base_pos		= NULL;
		FIGURE_dhpr_rdata1[0].parent_current_mat		= NULL;
		FIGURE_dhpr_rdata1[0].parent_current_pos		= NULL;

		FIGURE_dhpr_data.start_object			= start_object;
		if(p_thing->Genus.Person->PersonType==PERSON_ROPER)
			FIGURE_dhpr_data.body_def				= &dt->TheChunk->PeopleTypes[(dt->PersonID>>5)];
		else
			FIGURE_dhpr_data.body_def				= &dt->TheChunk->PeopleTypes[(dt->PersonID&0x1f)];
		FIGURE_dhpr_data.world_pos				= &pos;
		FIGURE_dhpr_data.tween					= dt->AnimTween;
		FIGURE_dhpr_data.ae1					= ae1;
		FIGURE_dhpr_data.ae2					= ae2;
		FIGURE_dhpr_data.world_mat				= &r_matrix;
//		FIGURE_dhpr_data.world_mat2				= &r_matrix2;
		FIGURE_dhpr_data.dx						= dx;
		FIGURE_dhpr_data.dy						= dy;
		FIGURE_dhpr_data.dz						= dz;
		FIGURE_dhpr_data.colour					= colour;
		FIGURE_dhpr_data.specular				= specular;

		// These two are used to find which unique mesh we are drawing.
		FIGURE_dhpr_data.bPersonType			= p_thing->Genus.Person->PersonType;
		FIGURE_dhpr_data.bPersonID				= dt->PersonID;

		FIGURE_draw_hierarchical_prim_recurse(p_thing);
	}
	else
	{
		for (int i = 0; i < ele_count; i++)
		{
			object_offset = dt->TheChunk->PeopleTypes[(dt->PersonID & 0x1f)].BodyPart[i];

			FIGURE_draw_prim_tween(
				start_object + object_offset,
				p_thing->WorldPos.X >> 8,
				(p_thing->WorldPos.Y >> 8),
				p_thing->WorldPos.Z >> 8,
				dt->AnimTween,
			   &ae1[i],
			   &ae2[i],
			   &r_matrix,
				dx,dy,dz,
				colour,
				specular,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				p_thing);
		}
	}

#if USE_TOMS_ENGINE_PLEASE_BOB
	// Clean up after ourselves.
	ASSERT ( MM_bLightTableAlreadySetUp );
	MM_bLightTableAlreadySetUp = FALSE;
#if 0
	MM_pcFadeTable = NULL;
	MM_pcFadeTableTint = NULL;
	MM_pMatrix = NULL;
	MM_Vertex = NULL;
	MM_pNormal = NULL;
#endif
#endif //#if USE_TOMS_ENGINE_PLEASE_BOB



	//
	// In case this thing ain't a person...
	//

	if (p_thing->Class == CLASS_PERSON)
	{

		//
		// Is this person carrying a grenade?
		//

		Thing *p_person = p_thing;

		if (p_person->Genus.Person->SpecialUse && TO_THING(p_person->Genus.Person->SpecialUse)->Genus.Special->SpecialType == SPECIAL_GRENADE)
		{
			SLONG px;
			SLONG py;
			SLONG pz;
			SLONG prim;

			calc_sub_objects_position(
				p_person,
				p_person->Draw.Tweened->AnimTween,
				SUB_OBJECT_LEFT_HAND,
			   &px,
			   &py,
			   &pz);

			px += p_person->WorldPos.X >> 8;
			py += p_person->WorldPos.Y >> 8;
			pz += p_person->WorldPos.Z >> 8;

			kludge_shrink = TRUE;

			MESH_draw_poly(
				PRIM_OBJ_ITEM_GRENADE,
				px,
				py,
				pz,
				0, 0, 0,
				NULL,0xff);

			kludge_shrink = FALSE;
		}
	}
	p_thing->Flags&=~FLAGS_PERSON_AIM_AND_RUN;

	LOG_EXIT ( Figure_FIGURE_Draw )

}


#else
// Old version

void FIGURE_draw(Thing *p_thing)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG	wx,wy,wz;

	ULONG colour;
	ULONG specular;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;

/*
	{
		static	SLONG	turn = 0;
		if(turn!=GAME_TURN)
		{
			if(Keys[KB_T])
			{
				peep_recol[0].r+=ShiftFlag?4:-4;
				SATURATE(peep_recol[0].r,0,256);
				PANEL_new_text(p_thing,200," peep r %d g %d b %d ",peep_recol[0].r,peep_recol[0].g,peep_recol[0].b);
			}
			if(Keys[KB_Y])
			{
				peep_recol[0].g+=ShiftFlag?4:-4;
				SATURATE(peep_recol[0].g,0,256);
				PANEL_new_text(p_thing,200," peep r %d g %d b %d ",peep_recol[0].r,peep_recol[0].g,peep_recol[0].b);
			}
			if(Keys[KB_U])
			{
				peep_recol[0].b+=ShiftFlag?4:-4;
				SATURATE(peep_recol[0].b,0,256);
				PANEL_new_text(p_thing,200," peep r %d g %d b %d ",peep_recol[0].r,peep_recol[0].g,peep_recol[0].b);
			}
		}
		turn=GAME_TURN;
	}
*/



	LOG_ENTER ( Figure_FIGURE_Draw )


//	calc_global_cloud(p_thing->WorldPos.X>>8,p_thing->WorldPos.Y>>8,p_thing->WorldPos.Z>>8);

#ifdef	PSX
	//
	// test some code out for PSX
	//
	if(get_sort_z_bodge(p_thing->WorldPos.X>>8,p_thing->WorldPos.Z>>8))
	{
		AENG_world_line(
			p_thing->WorldPos.X >> 8,
			p_thing->WorldPos.Y >> 8,
			p_thing->WorldPos.Z >> 8,
			32, 0xffffff,
			p_thing->WorldPos.X          >> 8,
			p_thing->WorldPos.Y + 0xc000 >> 8,
			p_thing->WorldPos.Z          >> 8,
			0, 0xffffff,
			TRUE);

	}
#endif

	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		LOG_EXIT ( Figure_FIGURE_Draw )
		return;
	}

	//
	// The offset to keep the locked limb in the same place.
	//

//	if (dt->Locked)
	if(0)
	{

		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		//
		// Taken from temp.cpp
		//

		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   0, dt->Locked, &x1, &y1, &z1);
		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, 256, dt->Locked, &x2, &y2, &z2);

		dx = x1 - x2;
		dy = y1 - y2;
		dz = z1 - z2;
	}
	else
	{
		dx = 0;
		dy = 0;
		dz = 0;
	}


	//
	// The rotation matrix of the whole object.
	//

	LOG_ENTER ( Figure_Rotate )

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		(dt->Roll+2048)&2047,
	   &r_matrix);
/*
	FIGURE_rotate_obj(
		dt->Tilt,
		dt->AngleTo,
		(dt->Roll+2048)&2047,
	   &r_matrix2);
*/
	LOG_EXIT ( Figure_Rotate )


#ifdef	PSX_SIMULATION
	{
		SLONG	index1,index2;
		//
		// stuff added for more compression of anims
		//
extern	struct	PrimPoint	*anim_mids; //[256];

		index1=dt->CurrentFrame->XYZIndex;
		index2=dt->NextFrame->XYZIndex;

		if(index1!=index2)
		{
/*
//
// draw some debug lines
//
			SVector temp;
			Matrix31  offset;

			wx=anim_mids[index1].X;
			wy=anim_mids[index1].Y;
			wz=anim_mids[index1].Z;
			offset.M[0]=wx;
			offset.M[1]=wy;
			offset.M[2]=wz;
			matrix_transformZMY((struct Matrix31*)&temp,&r_matrix, &offset);
			wx=temp.X;
			wy=temp.Y;
			wz=temp.Z;

			AENG_world_line((p_thing->WorldPos.X>>8),(p_thing->WorldPos.Y>>8),(p_thing->WorldPos.Z>>8),6,0xff0000,
							(p_thing->WorldPos.X>>8)+wx,(p_thing->WorldPos.Y>>8)+wy,(p_thing->WorldPos.Z>>8)+wz,1,0xffffff,1);

			wx=anim_mids[index2].X;
			wy=anim_mids[index2].Y;
			wz=anim_mids[index2].Z;

			offset.M[0]=wx;
			offset.M[1]=wy;
			offset.M[2]=wz;
			matrix_transformZMY((struct Matrix31*)&temp,&r_matrix, &offset);
			wx=temp.X;
			wy=temp.Y;
			wz=temp.Z;
			AENG_world_line((p_thing->WorldPos.X>>8),(p_thing->WorldPos.Y>>8),(p_thing->WorldPos.Z>>8),6,0xff00,
							(p_thing->WorldPos.X>>8)+wx,(p_thing->WorldPos.Y>>8)+wy,(p_thing->WorldPos.Z>>8)+wz,1,0xffffff,1);
*/



			wx=anim_mids[index1].X+(((anim_mids[index2].X-anim_mids[index1].X)*dt->AnimTween)>>8);
			wy=anim_mids[index1].Y+(((anim_mids[index2].Y-anim_mids[index1].Y)*dt->AnimTween)>>8);
			wz=anim_mids[index1].Z+(((anim_mids[index2].Z-anim_mids[index1].Z)*dt->AnimTween)>>8);
		}
		else
		{
			wx=anim_mids[index1].X;
			wy=anim_mids[index1].Y;
			wz=anim_mids[index1].Z;
		}

		//
		// put offset into object space
		//
		{
			SVector temp;
			Matrix31  offset;
			offset.M[0]=wx;
			offset.M[1]=wy;
			offset.M[2]=wz;
			matrix_transformZMY((struct Matrix31*)&temp,&r_matrix, &offset);
			wx=temp.X;
			wy=temp.Y;
			wz=temp.Z;
/*
			AENG_world_line((p_thing->WorldPos.X>>8),(p_thing->WorldPos.Y>>8),(p_thing->WorldPos.Z>>8),6,0x808080,
							(p_thing->WorldPos.X>>8)+wx,(p_thing->WorldPos.Y>>8)+wy,(p_thing->WorldPos.Z>>8)+wz,1,0xffffff,1);
*/
		}


	}
#else
	wx=0;
	wy=0;
	wz=0;
#endif

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");
		LOG_EXIT ( Figure_FIGURE_Draw )

		return;
	}

	//
	// What colour do we draw the figure?
	//

	//
	// All this is done in FIGURE_draw_prim_tween nowadays using the array
	// filled in by this call to NIGHT_find....
	//

	colour   = 0;
	specular = 0;

	SLONG lx;
	SLONG ly;
	SLONG lz;

	NIGHT_Colour col;

	LOG_ENTER ( Figure_Calc_Sub_Objs )

	calc_sub_objects_position(
		p_thing,
		dt->AnimTween,
		0,	// 0 is Pelvis
		&lx, &ly, &lz);

	lx += p_thing->WorldPos.X >> 8;
	ly += p_thing->WorldPos.Y >> 8;
	lz += p_thing->WorldPos.Z >> 8;

	LOG_EXIT ( Figure_Calc_Sub_Objs )

	LOG_ENTER ( Figure_NIGHT_Find )

	NIGHT_find(lx, ly, lz);

	LOG_EXIT ( Figure_NIGHT_Find )

	/*

	SLONG lx;
	SLONG ly;
	SLONG lz;

	NIGHT_Colour col;

	calc_sub_objects_position(
		p_thing,
		dt->AnimTween,
		0,	// 0 is Pelvis
		&lx, &ly, &lz);

	lx += p_thing->WorldPos.X >> 8;
	ly += p_thing->WorldPos.Y >> 8;
	lz += p_thing->WorldPos.Z >> 8;

	col = NIGHT_get_light_at(lx, ly, lz);

	if (p_thing->Genus.Person->BurnIndex) {
		Pyro *p=TO_PYRO(p_thing->Genus.Person->BurnIndex-1);
		if (p->PyroType==PYRO_IMMOLATE) {
			col.red=  (col.red>p->counter)  ?col.red-p->counter:0; 
			col.green=(col.green>p->counter)?col.green-p->counter:0; 
			col.blue= (col.blue>p->counter) ?col.blue-p->counter:0; 
		}
	}

	if (ControlFlag)
	{
		//
		// Brighten up the person if he is going to be drawn too dark.
		//

		if (col.red   < 32) {col.red   += 32 - col.red   >> 1;}
		if (col.green < 32) {col.green += 32 - col.green >> 1;}
		if (col.blue  < 32) {col.blue  += 32 - col.blue  >> 1;}
	}

	*/

/*
	if(p_thing->Genus.Person->PersonType==PERSON_THUG_RED)
	{
		NIGHT_Colour temp;
		SLONG	index;
		index=THING_NUMBER(p_thing)&15;
//		index=0;

		temp.red=(col.red*peep_recol[index].r)>>8;
		temp.green=(col.green*peep_recol[index].g)>>8;
		temp.blue=(col.blue*peep_recol[index].b)>>8;
		NIGHT_get_d3d_colour(
			temp,
		   &jacket_col,&specular);
			jacket_col   &= ~POLY_colour_restrict;

		index=(PERSON_NUMBER(p_thing->Genus.Person)>>2)%9;

		temp.red=(col.red*peep_recol[index].r)>>8;
		temp.green=(col.green*peep_recol[index].g)>>8;
		temp.blue=(col.blue*peep_recol[index].b)>>8;
		NIGHT_get_d3d_colour(
			temp,
		   &leg_col,&specular);
			leg_col   &= ~POLY_colour_restrict;


	}
*/
	/*

	NIGHT_get_d3d_colour(
		col,
	   &colour,
	   &specular);

	*/
/*
	if(p_thing->Genus.Person->PersonType!=PERSON_THUG_RED)
	{
		jacket_col=colour;
		leg_col=colour;

	}
*/
	
/*	if (p_thing->Genus.Person->BurnIndex) {
		Pyro *p=TO_PYRO(p_thing->Genus.Person->BurnIndex-1);
		if (p->PyroType==PYRO_IMMOLATE) {
			colour=0;
		}
	}*/

	/*


	colour   &= ~POLY_colour_restrict;
	specular &= ~POLY_colour_restrict;

	*/

	#if WE_WANT_TO_DARKEN_PEOPLE_IN_SHADOW_ABRUPTLY

	if (SHADOW_in(
			(p_thing->WorldPos.X >> 8),
			(p_thing->WorldPos.Y >> 8),
			(p_thing->WorldPos.Z >> 8)))
	{
		colour >>= 1;
		colour  &= 0x7f7f7f7f;
		specular = 0xff000000;
	}

	#endif




#if USE_TOMS_ENGINE_PLEASE_BOB


	ASSERT ( !MM_bLightTableAlreadySetUp );

	// Set up some data for the MM rendering thing.

#if 0
//#define ALIGNED_STATIC_ARRAY(name,number,mytype,align)														\
//	static char c##name##mytype##align##StaticArray [ align + number * sizeof ( mytype ) ];						\
//	name = (mytype *)( ( (DWORD)c##name##mytype##align##StaticArray + (align-1) ) & ~(align-1) )

	ALIGNED_STATIC_ARRAY ( MM_pcFadeTable, 128, D3DCOLOR, 4 );
	ALIGNED_STATIC_ARRAY ( MM_pcFadeTableTint, 128, D3DCOLOR, 4 );
	ALIGNED_STATIC_ARRAY ( MM_pMatrix, 1, D3DMATRIX, 32 );
	ALIGNED_STATIC_ARRAY ( MM_Vertex, 4, D3DVERTEX, 32 );
	ALIGNED_STATIC_ARRAY ( MM_pNormal, 4, float, 8 );
#endif

//#undef ALIGNED_STATIC_ARRAY

	Pyro *p = NULL;
	if (p_thing->Class == CLASS_PERSON && p_thing->Genus.Person->BurnIndex)
	{
		p = TO_PYRO(p_thing->Genus.Person->BurnIndex-1);
		if (p->PyroType != PYRO_IMMOLATE)
		{
			p = NULL;
		}
	}

	BuildMMLightingTable ( p, 0 );

	MM_bLightTableAlreadySetUp = TRUE;


#endif



	//
	// Draw each body part.
	//

	//SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[dt->MeshID]].StartObject;

	if (ele_count == 15 )
	{
		Matrix31	pos;
		pos.M[0] =  (p_thing->WorldPos.X >> 8)+wx;
		pos.M[1] = (p_thing->WorldPos.Y >> 8) + wy;
		pos.M[2] =  (p_thing->WorldPos.Z >> 8)+wz;

		// assume we've got a person, and draw the object hierarchically

		// fill out data.

		FIGURE_dhpr_rdata1[0].part_number			= 0;
		FIGURE_dhpr_rdata1[0].current_child_number	= 0;
		FIGURE_dhpr_rdata1[0].parent_base_mat		= NULL;
		FIGURE_dhpr_rdata1[0].parent_base_pos		= NULL;
		FIGURE_dhpr_rdata1[0].parent_current_mat		= NULL;
		FIGURE_dhpr_rdata1[0].parent_current_pos		= NULL;

		FIGURE_dhpr_data.start_object			= start_object;
		if(p_thing->Genus.Person->PersonType==PERSON_ROPER)

			FIGURE_dhpr_data.body_def				= &dt->TheChunk->PeopleTypes[(dt->PersonID>>5)];
		else
			FIGURE_dhpr_data.body_def				= &dt->TheChunk->PeopleTypes[(dt->PersonID&0x1f)];
		FIGURE_dhpr_data.world_pos				= &pos;
		FIGURE_dhpr_data.tween					= dt->AnimTween;
		FIGURE_dhpr_data.ae1					= ae1;
		FIGURE_dhpr_data.ae2					= ae2;
		FIGURE_dhpr_data.world_mat				= &r_matrix;
//		FIGURE_dhpr_data.world_mat2				= &r_matrix2;
		FIGURE_dhpr_data.dx						= dx;
		FIGURE_dhpr_data.dy						= dy;
		FIGURE_dhpr_data.dz						= dz;
		FIGURE_dhpr_data.colour					= colour;
		FIGURE_dhpr_data.specular				= specular;

		// These two are used to find which unique mesh we are drawing.
		FIGURE_dhpr_data.bPersonType			= p_thing->Genus.Person->PersonType;
		FIGURE_dhpr_data.bPersonID				= dt->PersonID;

		FIGURE_draw_hierarchical_prim_recurse(p_thing);
	}
	else
	{
		for (int i = 0; i < ele_count; i++)
		{
			object_offset = dt->TheChunk->PeopleTypes[(dt->PersonID & 0x1f)].BodyPart[i];

			FIGURE_draw_prim_tween(
				start_object + object_offset,
				p_thing->WorldPos.X >> 8,
				(p_thing->WorldPos.Y >> 8),
				p_thing->WorldPos.Z >> 8,
				dt->AnimTween,
			   &ae1[i],
			   &ae2[i],
			   &r_matrix,
				dx,dy,dz,
				colour,
				specular,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				p_thing);
		}
	}

#if USE_TOMS_ENGINE_PLEASE_BOB
	// Clean up after ourselves.
	ASSERT ( MM_bLightTableAlreadySetUp );
	MM_bLightTableAlreadySetUp = FALSE;
#if 0
	MM_pcFadeTable = NULL;
	MM_pcFadeTableTint = NULL;
	MM_pMatrix = NULL;
	MM_Vertex = NULL;
	MM_pNormal = NULL;
#endif
#endif //#if USE_TOMS_ENGINE_PLEASE_BOB



	//
	// In case this thing ain't a person...
	//

	if (p_thing->Class == CLASS_PERSON)
	{
		/*

		Thing *p_person = p_thing;

		//
		// If this person is holding a coke-can...
		//

		if (p_person->Genus.Person->Flags & FLAG_PERSON_CANNING)
		{
			SLONG px;
			SLONG py;
			SLONG pz;
			SLONG prim;

			calc_sub_objects_position(
				p_person,
				p_person->Draw.Tweened->AnimTween,
				SUB_OBJECT_LEFT_HAND,
			   &px,
			   &py,
			   &pz);

			px += p_person->WorldPos.X >> 8;
			py += p_person->WorldPos.Y >> 8;
			pz += p_person->WorldPos.Z >> 8;

			if (p_person->Genus.Person->Flags & FLAG_PERSON_HEADING)
			{
				DIRT_Info ans;

				DIRT_get_info(p_person->Genus.Person->Hold, &ans);

				prim = ans.prim;
			}
			else
			{
				prim = PRIM_OBJ_CAN;
			}

			MESH_draw_poly(
				prim,
				px,
				py,
				pz,
				0, 0, 0,
				NULL,0);
		}

		*/

		//
		// Is this person carrying a grenade?
		//

		Thing *p_person = p_thing;

		if (p_person->Genus.Person->SpecialUse && TO_THING(p_person->Genus.Person->SpecialUse)->Genus.Special->SpecialType == SPECIAL_GRENADE)
		{
			SLONG px;
			SLONG py;
			SLONG pz;
			SLONG prim;

			calc_sub_objects_position(
				p_person,
				p_person->Draw.Tweened->AnimTween,
				SUB_OBJECT_LEFT_HAND,
			   &px,
			   &py,
			   &pz);

			px += p_person->WorldPos.X >> 8;
			py += p_person->WorldPos.Y >> 8;
			pz += p_person->WorldPos.Z >> 8;

			kludge_shrink = TRUE;

			MESH_draw_poly(
				PRIM_OBJ_ITEM_GRENADE,
				px,
				py,
				pz,
				0, 0, 0,
				NULL,0xff);

			kludge_shrink = FALSE;
		}
	}
	p_thing->Flags&=~FLAGS_PERSON_AIM_AND_RUN;

	LOG_EXIT ( Figure_FIGURE_Draw )

}

#endif




void ANIM_obj_draw(Thing *p_thing,DrawTween *dt)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	ULONG colour;
	ULONG specular;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

//	calc_global_cloud(p_thing->WorldPos.X>>8,p_thing->WorldPos.Y>>8,p_thing->WorldPos.Z>>8);

	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		return;
	}

	/*

	{
		//
		// Draw the bounding box.
		//

		AnimPrimBbox *apb = &anim_prim_bbox[p_thing->Index];

		AENG_world_line(
			apb->minx + (p_thing->WorldPos.X >> 8),
			apb->miny + (p_thing->WorldPos.Y >> 8),
			apb->minz + (p_thing->WorldPos.Z >> 8),
			0x16,
			0x00ffffff,
			apb->maxx + (p_thing->WorldPos.X >> 8),
			apb->maxy + (p_thing->WorldPos.Y >> 8),
			apb->maxz + (p_thing->WorldPos.Z >> 8),
			0x16,
			0x000ccccff,
			TRUE);
	}

	*/

	//
	// The offset to keep the locked limb in the same place.
	//

	dx = 0;
	dy = 0;
	dz = 0;

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		(dt->Roll+2048)&2047,
	   &r_matrix);

	//
	// What colour do we draw the figure?
	//

	colour   = 0;
	specular = 0;

	SLONG lx;
	SLONG ly;
	SLONG lz;

	NIGHT_Colour col;

	lx = (p_thing->WorldPos.X >> 8);
	ly = (p_thing->WorldPos.Y >> 8) + 0x60;
	lz = (p_thing->WorldPos.Z >> 8);

	NIGHT_find(lx, ly, lz);

	/*

	NIGHT_get_d3d_colour(
		NIGHT_get_light_at(
			(p_thing->WorldPos.X >> 8),
			(p_thing->WorldPos.Y >> 8),
			(p_thing->WorldPos.Z >> 8)),
	   &colour,
	   &specular);
	
	colour   &= ~POLY_colour_restrict;
	specular &= ~POLY_colour_restrict;

	*/

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = i; //dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];
		FIGURE_draw_prim_tween(
			start_object + object_offset,
			p_thing->WorldPos.X >> 8,
			(p_thing->WorldPos.Y >> 8),
			p_thing->WorldPos.Z >> 8,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			colour,
			specular,
//			p_thing);

			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			p_thing,
			i,
			0xffff00ff);

	}
}


void ANIM_obj_draw_warped(Thing *p_thing,DrawTween *dt)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	ULONG colour;
	ULONG specular;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;


	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure");
		return;
	}

	//
	// The offset to keep the locked limb in the same place.
	//

	dx = 0;
	dy = 0;
	dz = 0;

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		(dt->Roll+2048)&2047,
	   &r_matrix);

	//
	// What colour do we draw the figure?
	//

	NIGHT_get_d3d_colour(
		NIGHT_get_light_at(
			(p_thing->WorldPos.X >> 8),
			(p_thing->WorldPos.Y >> 8),
			(p_thing->WorldPos.Z >> 8)),
	   &colour,
	   &specular);
	
	colour   &= ~POLY_colour_restrict;
	specular &= ~POLY_colour_restrict;


	//
	// Draw each body part.
	//

	SLONG i;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[0]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = i; //dt->TheChunk->PeopleTypes[dt->PersonID].BodyPart[i];

		FIGURE_draw_prim_tween_warped(
			start_object + object_offset,
			p_thing->WorldPos.X >> 8,
			(p_thing->WorldPos.Y >> 8),
			p_thing->WorldPos.Z >> 8,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			colour,
			specular,
			p_thing);
	}
}



#ifndef TARGET_DC

// ========================================================
// 
// DRAWING THE REFLECTION
//
// ========================================================

//
// The bounding box of the reflected points on screen.
//

SLONG FIGURE_reflect_x1;
SLONG FIGURE_reflect_y1;
SLONG FIGURE_reflect_x2;
SLONG FIGURE_reflect_y2;

float FIGURE_reflect_height;

//
// The points used to create the reflection.
//

typedef struct
{
	union
	{
		float distance;
		ULONG clip;
	};

	POLY_Point pp;

} FIGURE_Rpoint;

#define FIGURE_MAX_RPOINTS 256

FIGURE_Rpoint FIGURE_rpoint[FIGURE_MAX_RPOINTS];
SLONG         FIGURE_rpoint_upto;

void FIGURE_draw_prim_tween_reflection(
		SLONG prim,
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG tween,
		struct GameKeyFrameElement *anim_info,
		struct GameKeyFrameElement *anim_info_next,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		ULONG colour,
		ULONG specular,
		Thing *p_thing)
{
	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG px;
	SLONG py;

	ULONG red;
	ULONG green;
	ULONG blue;
	ULONG r;
	ULONG g;
	ULONG b;
	ULONG face_colour;
	SLONG fog;

	float world_x;
	float world_y;
	float world_z;

	SLONG page;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;

	SVector temp;
	
	PrimFace4  *p_f4;
	PrimFace3  *p_f3;
	PrimObject *p_obj;

	POLY_Point *tri [3];
	POLY_Point *quad[4];

	FIGURE_Rpoint *frp;

	colour   >>= 1;
	specular >>= 1;

	colour   &= 0x7f7f7f7f;
	specular &= 0x7f7f7f7f;

	colour   |= 0xff000000;
	specular |= 0xff000000;

	red   = (colour >> 16) & 0xff;
	green = (colour >>  8) & 0xff;
	blue  = (colour >>  0) & 0xff;
  
	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	offset.M[0] = anim_info->OffsetX + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween >> 8);
	offset.M[1] = anim_info->OffsetY + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween >> 8);
	offset.M[2] = anim_info->OffsetZ + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween >> 8);

	matrix_transformZMY((struct Matrix31*)&temp,rot_mat, &offset);

	SLONG	character_scale  = person_get_scale(p_thing);
	temp.X = (temp.X * character_scale) / 256;
	temp.Y = (temp.Y * character_scale) / 256;
	temp.Z = (temp.Z * character_scale) / 256;

	x += temp.X;
	y += temp.Y;
	z += temp.Z;	

	//
	// Create a temporary "tween" matrix between current and next
	//

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	build_tween_matrix(&mat2, &m1, &m2 ,tween);
	normalise_matrix(&mat2);
	
	//
	// Apply local rotation matrix to get mat_final that rotates
	// the point into world space.
	//

	matrix_mult33(&mat_final, rot_mat, &mat2);

	mat_final.M[0][0] = (mat_final.M[0][0] * character_scale) / 256;
	mat_final.M[0][1] = (mat_final.M[0][1] * character_scale) / 256;
	mat_final.M[0][2] = (mat_final.M[0][2] * character_scale) / 256;
	mat_final.M[1][0] = (mat_final.M[1][0] * character_scale) / 256;
	mat_final.M[1][1] = (mat_final.M[1][1] * character_scale) / 256;
	mat_final.M[1][2] = (mat_final.M[1][2] * character_scale) / 256;
	mat_final.M[2][0] = (mat_final.M[2][0] * character_scale) / 256;
	mat_final.M[2][1] = (mat_final.M[2][1] * character_scale) / 256;
	mat_final.M[2][2] = (mat_final.M[2][2] * character_scale) / 256;

	//
	// Do everything in floats.
	//

	float off_x;
	float off_y;
	float off_z;
	float fmatrix[9];

	off_x = float(x);
	off_y = float(y);
	off_z = float(z);

	fmatrix[0] = float(mat_final.M[0][0]) * (1.0F / 32768.0F);
	fmatrix[1] = float(mat_final.M[0][1]) * (1.0F / 32768.0F);
	fmatrix[2] = float(mat_final.M[0][2]) * (1.0F / 32768.0F);
	fmatrix[3] = float(mat_final.M[1][0]) * (1.0F / 32768.0F);
	fmatrix[4] = float(mat_final.M[1][1]) * (1.0F / 32768.0F);
	fmatrix[5] = float(mat_final.M[1][2]) * (1.0F / 32768.0F);
	fmatrix[6] = float(mat_final.M[2][0]) * (1.0F / 32768.0F);
	fmatrix[7] = float(mat_final.M[2][1]) * (1.0F / 32768.0F);
	fmatrix[8] = float(mat_final.M[2][2]) * (1.0F / 32768.0F);

	POLY_set_local_rotation(
		off_x,
		off_y,
		off_z,
		fmatrix);

	//
	// How deep the reflection can be
	//

	#define FIGURE_MAX_DY					(128.0F)
	#define FIGURE_255_DIVIDED_BY_MAX_DY	(255.0F / FIGURE_MAX_DY)

	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	FIGURE_rpoint_upto = 0;

	for (i = sp; i < ep; i++)
	{
		ASSERT(WITHIN(FIGURE_rpoint_upto, 0, FIGURE_MAX_RPOINTS - 1));

		frp = &FIGURE_rpoint[FIGURE_rpoint_upto++];

		POLY_transform_using_local_rotation(
			AENG_dx_prim_points[i].X,
			AENG_dx_prim_points[i].Y,
			AENG_dx_prim_points[i].Z,
		   &frp->pp);

		if (frp->pp.MaybeValid())
		{
			frp->pp.colour   = colour;
			frp->pp.specular = specular;

			//
			// Work out the signed distance of this point from the reflection plane.
			//

			world_x = AENG_dx_prim_points[i].X;
			world_y = AENG_dx_prim_points[i].Y;
			world_z = AENG_dx_prim_points[i].Z;

			MATRIX_MUL(
				fmatrix,
				world_x,
				world_y,
				world_z);

			world_x += off_x;
			world_y += off_y;
			world_z += off_z;

			frp->distance = FIGURE_reflect_height - world_y;

			if (frp->distance >= 0.0F)
			{
				//
				// Update the bounding box.
				//

				px = SLONG(frp->pp.X);
				py = SLONG(frp->pp.Y);

				if (px < FIGURE_reflect_x1) {FIGURE_reflect_x1 = px;}
				if (px > FIGURE_reflect_x2) {FIGURE_reflect_x2 = px;}
				if (py < FIGURE_reflect_y1) {FIGURE_reflect_y1 = py;}
				if (py > FIGURE_reflect_y2) {FIGURE_reflect_y2 = py;}

				//
				// Fade out the point depending on distance from the reflection plane.
				//

				fog  = frp->pp.specular >> 24;
				fog += SLONG(frp->distance * FIGURE_255_DIVIDED_BY_MAX_DY);

				if (fog > 255) {fog = 255;}

				frp->pp.specular &= 0x00ffffff;
				frp->pp.specular |= fog << 24;
			}
			else
			{
				frp->pp.clip = 0;
			}
		}
	}

	//
	// The quads.
	//

	for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
	{
		p_f4 = &prim_faces4[i];

		p0 = p_f4->Points[0] - sp;
		p1 = p_f4->Points[1] - sp;
		p2 = p_f4->Points[2] - sp;
		p3 = p_f4->Points[3] - sp;
		
		ASSERT(WITHIN(p0, 0, FIGURE_rpoint_upto - 1));
		ASSERT(WITHIN(p1, 0, FIGURE_rpoint_upto - 1));
		ASSERT(WITHIN(p2, 0, FIGURE_rpoint_upto - 1));
		ASSERT(WITHIN(p3, 0, FIGURE_rpoint_upto - 1));

		//
		// Add the quads backwards...
		//

		quad[0] = &FIGURE_rpoint[p0].pp;
		quad[2] = &FIGURE_rpoint[p1].pp;
		quad[1] = &FIGURE_rpoint[p2].pp;
		quad[3] = &FIGURE_rpoint[p3].pp;

		if (POLY_valid_quad(quad))
		{
			if (p_f4->DrawFlags & POLY_FLAG_TEXTURED)
			{
				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);
												        
				quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
				quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);
												        
				quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
				quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

				quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
				quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

				page   = p_f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f4->TexturePage;
				page+=FACE_PAGE_OFFSET;

				if (the_display.GetDeviceInfo()->AdamiLightingSupported())
				{
					POLY_add_quad(quad, POLY_PAGE_COLOUR, TRUE);
				}
				POLY_add_quad(quad, page,             TRUE);
			}
			else
			{
				/*

				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f4->Col2].red;
				g = ENGINE_palette[p_f4->Col2].green;
				b = ENGINE_palette[p_f4->Col2].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

				quad[0]->colour = face_colour;
				quad[1]->colour = face_colour;
				quad[2]->colour = face_colour;
				quad[3]->colour = face_colour;

				POLY_add_quad(quad, POLY_PAGE_COLOUR, TRUE);

				quad[0]->colour = colour;
				quad[1]->colour = colour;
				quad[2]->colour = colour;
				quad[3]->colour = colour;

				*/
			}
		}
	}

	//
	// The triangles.
	//

	for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
	{
		p_f3 = &prim_faces3[i];

		p0 = p_f3->Points[0] - sp;
		p1 = p_f3->Points[1] - sp;
		p2 = p_f3->Points[2] - sp;
		
		ASSERT(WITHIN(p0, 0, FIGURE_rpoint_upto - 1));
		ASSERT(WITHIN(p1, 0, FIGURE_rpoint_upto - 1));
		ASSERT(WITHIN(p2, 0, FIGURE_rpoint_upto - 1));

		//
		// Add the triangle backwards!
		//

		tri[0] = &FIGURE_rpoint[p0].pp;
		tri[2] = &FIGURE_rpoint[p1].pp;
		tri[1] = &FIGURE_rpoint[p2].pp;

		if (POLY_valid_triangle(tri))
		{
			if (p_f3->DrawFlags & POLY_FLAG_TEXTURED)
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
												       
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
												       
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;
				page+=FACE_PAGE_OFFSET;

				if (the_display.GetDeviceInfo()->AdamiLightingSupported())
				{
					POLY_add_triangle(tri, POLY_PAGE_COLOUR, TRUE);
				}
				POLY_add_triangle(tri, page,             TRUE);
			}
			else
			{
				/*

				//
				// The colour of the face.
				// 

				r = ENGINE_palette[p_f3->Col2].red;
				g = ENGINE_palette[p_f3->Col2].green;
				b = ENGINE_palette[p_f3->Col2].blue;

				r = r * red   >> 8;
				g = g * green >> 8;
				b = b * blue  >> 8;

				face_colour = (r << 16) | (g << 8) | (b << 0);

				tri[0]->colour = face_colour;
				tri[1]->colour = face_colour;
				tri[2]->colour = face_colour;

				POLY_add_triangle(tri, POLY_PAGE_COLOUR, TRUE);

				tri[0]->colour = colour;
				tri[1]->colour = colour;
				tri[2]->colour = colour;

				*/
			}
		}
	}
}

void FIGURE_draw_reflection(Thing *p_thing, SLONG height)
{
	SLONG dx;
	SLONG dy;
	SLONG dz;

	ULONG colour;
	ULONG specular;

	POLY_Point *pp;

	Matrix33 r_matrix;

	GameKeyFrameElement *ae1;
	GameKeyFrameElement *ae2;

	DrawTween *dt = p_thing->Draw.Tweened;

	if (dt->CurrentFrame == 0 ||
		dt->NextFrame    == 0)
	{
		//
		// No frames to tween between.
		//

		MSG_add("!!!!!!!!!!!!!!!!!!!!!!!!ERROR FIGURE_draw_reflection");
		return;
	}

	//
	// The offset to keep the locked limb in the same place.
	//

	//if (dt->Locked)
	if (0)
	{
		SLONG x1, y1, z1;
		SLONG x2, y2, z2;

		//
		// Taken from temp.cpp
		//

		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame,   0, dt->Locked, &x1, &y1, &z1);
		calc_sub_objects_position_global(dt->CurrentFrame, dt->NextFrame, 256, dt->Locked, &x2, &y2, &z2);

		dx = x1 - x2;
		dy = y1 - y2;
		dz = z1 - z2;
	}
	else
	{
		dx = 0;
		dy = 0;
		dz = 0;
	}

	//
	// The animation elements for the two frames.
	//

	ae1 = dt->CurrentFrame->FirstElement;
	ae2 = dt->NextFrame   ->FirstElement;   

	if (!ae1 || !ae2)
	{
		MSG_add("!!!!!!!!!!!!!!!!!!!ERROR AENG_draw_figure has no animation elements");

		return;
	}

	//
	// The rotation matrix of the whole object.
	//

	FIGURE_rotate_obj(
		dt->Tilt,
		dt->Angle,
		- dt->Roll, // - = JCL
	   &r_matrix);

	SLONG posx = p_thing->WorldPos.X >> 8;
	SLONG posy = p_thing->WorldPos.Y >> 8;
	SLONG posz = p_thing->WorldPos.Z >> 8;

	//
	// Reflect about y = height.
	//

	posy =  height - (posy - height);
	dy   = -dy;

	r_matrix.M[0][1] = -r_matrix.M[0][1];
	r_matrix.M[1][1] = -r_matrix.M[1][1];
	r_matrix.M[2][1] = -r_matrix.M[2][1];
	
	//
	// Initialise the bounding box.
	//

	FIGURE_reflect_x1 = +INFINITY;
	FIGURE_reflect_y1 = +INFINITY;
	FIGURE_reflect_x2 = -INFINITY;
	FIGURE_reflect_y2 = -INFINITY;

	//
	// The height of the plane the body-parts are clipped against.
	//

	FIGURE_reflect_height = float(height);

	//
	// What colour do we draw the figure?
	//

	SLONG lx;
	SLONG ly;
	SLONG lz;

	NIGHT_Colour col;

	calc_sub_objects_position(
		p_thing,
		dt->AnimTween,
		0,	// 0 is Pelvis
		&lx, &ly, &lz);

	lx += p_thing->WorldPos.X >> 8;
	ly += p_thing->WorldPos.Y >> 8;
	lz += p_thing->WorldPos.Z >> 8;

	col = NIGHT_get_light_at(lx, ly, lz);

	if (!ControlFlag)
	{
		//
		// Brighten up the person if he is going to be drawn too dark.
		//

		if (col.red   < 32) {col.red   += 32 - col.red   >> 1;}
		if (col.green < 32) {col.green += 32 - col.green >> 1;}
		if (col.blue  < 32) {col.blue  += 32 - col.blue  >> 1;}
	}

	NIGHT_get_d3d_colour(
		col,
	   &colour,
	   &specular);
	
	colour   &= ~POLY_colour_restrict;
	specular &= ~POLY_colour_restrict;

	//
	// Draw each body part.
	//

	SLONG i;
	SLONG j;
	SLONG ele_count;
	SLONG start_object;
	SLONG object_offset;
	SLONG px;
	SLONG py;

	ele_count    = dt->TheChunk->ElementCount;
	start_object = prim_multi_objects[dt->TheChunk->MultiObject[dt->MeshID]].StartObject;

	for (i = 0; i < ele_count; i++)
	{
		object_offset = dt->TheChunk->PeopleTypes[dt->PersonID&0x1f].BodyPart[i];
//		object_offset = dt->TheChunk->PeopleTypes[0].BodyPart[i];

		FIGURE_draw_prim_tween_reflection(
			start_object + object_offset,
			posx,
			posy,
			posz,
			dt->AnimTween,
		   &ae1[i],
		   &ae2[i],
		   &r_matrix,
			dx,dy,dz,
			colour,
			specular,
			p_thing);
	}
}

#endif //#ifndef TARGET_DC







// Like FIGURE_draw_prim_tween, but optimised for the person-only case.
// Also assumes the lighting has been set up, etc.
// This just sets up the matrix and light vector it's asked to - it doesn't
// do anything else.
// Return value is TRUE if this body part is not clipped by the near-Z.
bool FIGURE_draw_prim_tween_person_only_just_set_matrix
(
		int iMatrixNum,
		SLONG prim,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		SLONG recurse_level,
		Thing    *p_thing
		)
{


	SLONG x										= FIGURE_dhpr_data.world_pos->M[0];
	SLONG y										= FIGURE_dhpr_data.world_pos->M[1];
	SLONG z										= FIGURE_dhpr_data.world_pos->M[2];
	SLONG tween									= FIGURE_dhpr_data.tween;
	struct GameKeyFrameElement *anim_info		= &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number];
	struct GameKeyFrameElement *anim_info_next	= &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number];
	ULONG colour								= FIGURE_dhpr_data.colour;
	ULONG specular								= FIGURE_dhpr_data.specular;
	CMatrix33 *parent_base_mat					= FIGURE_dhpr_rdata1[recurse_level].parent_base_mat;
	Matrix31 *parent_base_pos					= FIGURE_dhpr_rdata1[recurse_level].parent_base_pos;
	Matrix33 *parent_curr_mat					= FIGURE_dhpr_rdata1[recurse_level].parent_current_mat;
	Matrix31 *parent_curr_pos					= FIGURE_dhpr_rdata1[recurse_level].parent_current_pos;
	Matrix33 *end_mat							= &FIGURE_dhpr_rdata2[recurse_level].end_mat;
	Matrix31 *end_pos							= &FIGURE_dhpr_rdata2[recurse_level].end_pos;



	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG dprod;
	SLONG r;
	SLONG g;
	SLONG b;

	SLONG dr;
	SLONG dg;
	SLONG db;

	SLONG face_colour;

	SLONG page;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;

	ULONG qc0;
	ULONG qc1;
	ULONG qc2;
	ULONG qc3;

	SVector temp;
	
	PrimFace4   *p_f4;
	PrimFace3   *p_f3;
	PrimObject  *p_obj;
	NIGHT_Found *nf;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];
	SLONG	tex_page_offset;


	LOG_ENTER ( Figure_Draw_Prim_Tween )

	tex_page_offset=p_thing->Genus.Person->pcom_colour&0x3;

	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	if (parent_base_mat)
	{
		// we've got hierarchy info!
		
		Matrix31	p;
		p.M[0] = anim_info->OffsetX;
		p.M[1] = anim_info->OffsetY;
		p.M[2] = anim_info->OffsetZ;

		HIERARCHY_Get_Body_Part_Offset(&offset, &p,
									   parent_base_mat, parent_base_pos,
									   parent_curr_mat, parent_curr_pos);
		
		// pass data up the hierarchy
		if (end_pos)
			*end_pos = offset;
	}
	else
	{
		// process at highter resolution
		offset.M[0] = (anim_info->OffsetX << 8) + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween);
		offset.M[1] = (anim_info->OffsetY << 8) + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween);
		offset.M[2] = (anim_info->OffsetZ << 8) + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween);

		if (end_pos)
		{
			*end_pos = offset;
		}
	}


	// convert pos to floating point here to preserve accuracy and prevent overflow.
	// It's also a shitload faster on P2 and SH4.
	float	off_x = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[0][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[0][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[0][2]) / 32768.f);
	float	off_y = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[1][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[1][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[1][2]) / 32768.f);
	float	off_z = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[2][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[2][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[2][2]) / 32768.f);


	SLONG	character_scale  = person_get_scale(p_thing);
	float	character_scalef = float(character_scale) / 256.f;
	
	off_x *= character_scalef;
	off_y *= character_scalef;
	off_z *= character_scalef;

	off_x += float(x);
	off_y += float(y);
	off_z += float(z);

	//
	// Do everything in floats.
	//

	float fmatrix[9];
	//SLONG imatrix[9];

	//
	// Create a temporary "tween" matrix between current and next
	//

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	CQuaternion::BuildTween(&mat2, &m1, &m2, tween);

	// pass data up the hierarchy
	if (end_mat)
		*end_mat = mat2;

	//
	// Apply local rotation matrix to get mat_final that rotates
	// the point into world space.
	//

	matrix_mult33(&mat_final, rot_mat, &mat2);

	//!  yeehaw!
	mat_final.M[0][0] = (mat_final.M[0][0] * character_scale) / 256;
	mat_final.M[0][1] = (mat_final.M[0][1] * character_scale) / 256;
	mat_final.M[0][2] = (mat_final.M[0][2] * character_scale) / 256;
	mat_final.M[1][0] = (mat_final.M[1][0] * character_scale) / 256;
	mat_final.M[1][1] = (mat_final.M[1][1] * character_scale) / 256;
	mat_final.M[1][2] = (mat_final.M[1][2] * character_scale) / 256;
	mat_final.M[2][0] = (mat_final.M[2][0] * character_scale) / 256;
	mat_final.M[2][1] = (mat_final.M[2][1] * character_scale) / 256;
	mat_final.M[2][2] = (mat_final.M[2][2] * character_scale) / 256;

	fmatrix[0] = float(mat_final.M[0][0]) * (1.0F / 32768.0F);
	fmatrix[1] = float(mat_final.M[0][1]) * (1.0F / 32768.0F);
	fmatrix[2] = float(mat_final.M[0][2]) * (1.0F / 32768.0F);
	fmatrix[3] = float(mat_final.M[1][0]) * (1.0F / 32768.0F);
	fmatrix[4] = float(mat_final.M[1][1]) * (1.0F / 32768.0F);
	fmatrix[5] = float(mat_final.M[1][2]) * (1.0F / 32768.0F);
	fmatrix[6] = float(mat_final.M[2][0]) * (1.0F / 32768.0F);
	fmatrix[7] = float(mat_final.M[2][1]) * (1.0F / 32768.0F);
	fmatrix[8] = float(mat_final.M[2][2]) * (1.0F / 32768.0F);





	LOG_ENTER ( Figure_Set_Rotation )

	POLY_set_local_rotation(
		off_x,
		off_y,
		off_z,
		fmatrix);

	LOG_ENTER ( Figure_Set_Rotation )



	// Not 100% sure if I'm using character_scalef correctly...
	// ...but it seems to work OK.
	ASSERT ( ( character_scalef < 1.2f ) && ( character_scalef > 0.8f ) );
	//ASSERT ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) );
	if ( ( ( ( g_matWorld._43 * 32768.0f ) - ( (m_fObjectBoundingSphereRadius[prim]) * character_scalef ) ) < ( POLY_ZCLIP_PLANE * 32768.0f ) ) )
	{
		// Clipped by Z-plane. Don't set this matrix up, just return.
		return FALSE;
	}



	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;

	// Check for being a gun
	if (prim==256)
	{
		i=sp;		
	}
	else if (prim==258)
	{
		// Or a shotgun
		i=sp+15;
	}
	else if (prim==260)
	{
		// or an AK
		i=sp+32;
	}
	else
	{
		goto no_muzzle_calcs; // which skips...
	}


	//
	// this bit, which only executes if one of the above tests is true.
    //
	pp = &POLY_buffer[POLY_buffer_upto]; // no ++, so reused
	pp->x=AENG_dx_prim_points[i].X;
	pp->y=AENG_dx_prim_points[i].Y;
	pp->z=AENG_dx_prim_points[i].Z;
	MATRIX_MUL(
		fmatrix,
		pp->x,
		pp->y,
		pp->z);

	pp->x+=off_x; pp->y+=off_y; pp->z+=off_z;
	p_thing->Genus.Person->GunMuzzle.X=pp->x*256;
	p_thing->Genus.Person->GunMuzzle.Y=pp->y*256;
	p_thing->Genus.Person->GunMuzzle.Z=pp->z*256;
//	x=pp->x*256; y=pp->y*256; z=pp->z*256;


no_muzzle_calcs:



#if !USE_TOMS_ENGINE_PLEASE_BOB
#error Dont use this rout if USE_TOMS_ENGINE_PLEASE_BOB is not 1
#endif


	ASSERT ( MM_bLightTableAlreadySetUp );

	ASSERT (!WITHIN(prim, 261, 263));

	{

		ASSERT ( MM_bLightTableAlreadySetUp );

		LOG_ENTER ( Figure_Build_Matrices )

		extern float POLY_cam_matrix_comb[9];
		extern float POLY_cam_off_x;
		extern float POLY_cam_off_y;
		extern float POLY_cam_off_z;


		extern D3DMATRIX g_matProjection;
		extern D3DMATRIX g_matWorld;
		extern D3DVIEWPORT2 g_viewData;


		D3DMATRIX matTemp;

		matTemp._11 = g_matWorld._11*g_matProjection._11 + g_matWorld._12*g_matProjection._21 + g_matWorld._13*g_matProjection._31 + g_matWorld._14*g_matProjection._41;
		matTemp._12 = g_matWorld._11*g_matProjection._12 + g_matWorld._12*g_matProjection._22 + g_matWorld._13*g_matProjection._32 + g_matWorld._14*g_matProjection._42;
		matTemp._13 = g_matWorld._11*g_matProjection._13 + g_matWorld._12*g_matProjection._23 + g_matWorld._13*g_matProjection._33 + g_matWorld._14*g_matProjection._43;
		matTemp._14 = g_matWorld._11*g_matProjection._14 + g_matWorld._12*g_matProjection._24 + g_matWorld._13*g_matProjection._34 + g_matWorld._14*g_matProjection._44;

		matTemp._21 = g_matWorld._21*g_matProjection._11 + g_matWorld._22*g_matProjection._21 + g_matWorld._23*g_matProjection._31 + g_matWorld._24*g_matProjection._41;
		matTemp._22 = g_matWorld._21*g_matProjection._12 + g_matWorld._22*g_matProjection._22 + g_matWorld._23*g_matProjection._32 + g_matWorld._24*g_matProjection._42;
		matTemp._23 = g_matWorld._21*g_matProjection._13 + g_matWorld._22*g_matProjection._23 + g_matWorld._23*g_matProjection._33 + g_matWorld._24*g_matProjection._43;
		matTemp._24 = g_matWorld._21*g_matProjection._14 + g_matWorld._22*g_matProjection._24 + g_matWorld._23*g_matProjection._34 + g_matWorld._24*g_matProjection._44;

		matTemp._31 = g_matWorld._31*g_matProjection._11 + g_matWorld._32*g_matProjection._21 + g_matWorld._33*g_matProjection._31 + g_matWorld._34*g_matProjection._41;
		matTemp._32 = g_matWorld._31*g_matProjection._12 + g_matWorld._32*g_matProjection._22 + g_matWorld._33*g_matProjection._32 + g_matWorld._34*g_matProjection._42;
		matTemp._33 = g_matWorld._31*g_matProjection._13 + g_matWorld._32*g_matProjection._23 + g_matWorld._33*g_matProjection._33 + g_matWorld._34*g_matProjection._43;
		matTemp._34 = g_matWorld._31*g_matProjection._14 + g_matWorld._32*g_matProjection._24 + g_matWorld._33*g_matProjection._34 + g_matWorld._34*g_matProjection._44;

		matTemp._41 = g_matWorld._41*g_matProjection._11 + g_matWorld._42*g_matProjection._21 + g_matWorld._43*g_matProjection._31 + g_matWorld._44*g_matProjection._41;
		matTemp._42 = g_matWorld._41*g_matProjection._12 + g_matWorld._42*g_matProjection._22 + g_matWorld._43*g_matProjection._32 + g_matWorld._44*g_matProjection._42;
		matTemp._43 = g_matWorld._41*g_matProjection._13 + g_matWorld._42*g_matProjection._23 + g_matWorld._43*g_matProjection._33 + g_matWorld._44*g_matProjection._43;
		matTemp._44 = g_matWorld._41*g_matProjection._14 + g_matWorld._42*g_matProjection._24 + g_matWorld._43*g_matProjection._34 + g_matWorld._44*g_matProjection._44;







		// Now make up the matrices.

#if 0
		// Officially correct version.
		DWORD dwWidth = g_viewData.dwWidth >> 1;
		DWORD dwHeight = g_viewData.dwHeight >> 1;
		DWORD dwX = g_viewData.dwX;
		DWORD dwY = g_viewData.dwY;
#else
		// Version that knows about the letterbox mode hack.
extern DWORD g_dw3DStuffHeight;
extern DWORD g_dw3DStuffY;
		DWORD dwWidth = g_viewData.dwWidth >> 1;
		DWORD dwHeight = g_dw3DStuffHeight >> 1;
		DWORD dwX = g_viewData.dwX;
		DWORD dwY = g_dw3DStuffY;
#endif

		// Set up the matrix.
		D3DMATRIX *pmat = &(MMBodyParts_pMatrix[iMatrixNum]);
		pmat->_11 = 0.0f;
		pmat->_12 = matTemp._11 *   (float)dwWidth  + matTemp._14 * (float)( dwX + dwWidth  );
		pmat->_13 = matTemp._12 * - (float)dwHeight + matTemp._14 * (float)( dwY + dwHeight );
		pmat->_14 = matTemp._14;
		pmat->_21 = 0.0f;
		pmat->_22 = matTemp._21 *   (float)dwWidth  + matTemp._24 * (float)( dwX + dwWidth  );
		pmat->_23 = matTemp._22 * - (float)dwHeight + matTemp._24 * (float)( dwY + dwHeight );
		pmat->_24 = matTemp._24;
		pmat->_31 = 0.0f;
		pmat->_32 = matTemp._31 *   (float)dwWidth  + matTemp._34 * (float)( dwX + dwWidth  );
		pmat->_33 = matTemp._32 * - (float)dwHeight + matTemp._34 * (float)( dwY + dwHeight );
		pmat->_34 = matTemp._34;
		// Validation magic number.
		unsigned long EVal = 0xe0001000;
		pmat->_41 = *(float *)&EVal;
		pmat->_42 = matTemp._41 *   (float)dwWidth  + matTemp._44 * (float)( dwX + dwWidth  );
		pmat->_43 = matTemp._42 * - (float)dwHeight + matTemp._44 * (float)( dwY + dwHeight );
		pmat->_44 = matTemp._44;



		// 251 is a magic number for the DIP call!
		const float fNormScale = 251.0f;

		// Transform the lighting direction(s) by the inverse object matrix to get it into object space.
		// Assume inverse=transpose.
		D3DVECTOR vTemp;
		vTemp.x = MM_vLightDir.x * fmatrix[0] + MM_vLightDir.y * fmatrix[3] + MM_vLightDir.z * fmatrix[6];
		vTemp.y = MM_vLightDir.x * fmatrix[1] + MM_vLightDir.y * fmatrix[4] + MM_vLightDir.z * fmatrix[7];
		vTemp.z = MM_vLightDir.x * fmatrix[2] + MM_vLightDir.y * fmatrix[5] + MM_vLightDir.z * fmatrix[8];

		// Set up the lighting vector.
		float *pnorm = &(MMBodyParts_pNormal[iMatrixNum<<2]);
		pnorm[0] = 0.0f;
		pnorm[1] = vTemp.x * fNormScale;
		pnorm[2] = vTemp.y * fNormScale;
		pnorm[3] = vTemp.z * fNormScale;


		LOG_EXIT ( Figure_Build_Matrices )
	}


	// No environment mapping.
	ASSERT ( p_thing && ( p_thing->Class != CLASS_VEHICLE ) );

	ASSERT ( MM_bLightTableAlreadySetUp );

	LOG_EXIT ( Figure_Draw_Prim_Tween )

	return TRUE;

}







// Like FIGURE_draw_prim_tween, but optimised for the person-only case.
// Also assumes the lighting has been set up, etc.
void FIGURE_draw_prim_tween_person_only(
		SLONG prim,
		//SLONG x,
		//SLONG y,
		//SLONG z,
		//SLONG tween,
		//struct GameKeyFrameElement *anim_info,
		//struct GameKeyFrameElement *anim_info_next,
		struct Matrix33 *rot_mat,
		SLONG off_dx,
		SLONG off_dy,
		SLONG off_dz,
		//ULONG colour,
		//ULONG specular,
		SLONG recurse_level,
		//CMatrix33 *parent_base_mat,
		//Matrix31 *parent_base_pos,
		//Matrix33 *parent_curr_mat,
		//Matrix31 *parent_curr_pos,
		//Matrix33 *end_mat,
		//Matrix31 *end_pos,
		Thing    *p_thing
		//SLONG     part_number = 0xffffffff,
		//ULONG     colour_and  = 0xffffffff
		)
{


	SLONG x										= FIGURE_dhpr_data.world_pos->M[0];
	SLONG y										= FIGURE_dhpr_data.world_pos->M[1];
	SLONG z										= FIGURE_dhpr_data.world_pos->M[2];
	SLONG tween									= FIGURE_dhpr_data.tween;
	struct GameKeyFrameElement *anim_info		= &FIGURE_dhpr_data.ae1[FIGURE_dhpr_rdata1[recurse_level].part_number];
	struct GameKeyFrameElement *anim_info_next	= &FIGURE_dhpr_data.ae2[FIGURE_dhpr_rdata1[recurse_level].part_number];
	ULONG colour								= FIGURE_dhpr_data.colour;
	ULONG specular								= FIGURE_dhpr_data.specular;
	CMatrix33 *parent_base_mat					= FIGURE_dhpr_rdata1[recurse_level].parent_base_mat;
	Matrix31 *parent_base_pos					= FIGURE_dhpr_rdata1[recurse_level].parent_base_pos;
	Matrix33 *parent_curr_mat					= FIGURE_dhpr_rdata1[recurse_level].parent_current_mat;
	Matrix31 *parent_curr_pos					= FIGURE_dhpr_rdata1[recurse_level].parent_current_pos;
	Matrix33 *end_mat							= &FIGURE_dhpr_rdata2[recurse_level].end_mat;
	Matrix31 *end_pos							= &FIGURE_dhpr_rdata2[recurse_level].end_pos;
	SLONG     part_number						= FIGURE_dhpr_rdata1[recurse_level].part_number;
	//ULONG     colour_and  = 0xffffffff;




	SLONG i;
	SLONG j;

	SLONG sp;
	SLONG ep;

	SLONG p0;
	SLONG p1;
	SLONG p2;
	SLONG p3;

	SLONG nx;
	SLONG ny;
	SLONG nz;

	SLONG red;
	SLONG green;
	SLONG blue;
	SLONG dprod;
	SLONG r;
	SLONG g;
	SLONG b;

	SLONG dr;
	SLONG dg;
	SLONG db;

	SLONG face_colour;

	SLONG page;

	Matrix31  offset;
	Matrix33  mat2;
	Matrix33  mat_final;

	ULONG qc0;
	ULONG qc1;
	ULONG qc2;
	ULONG qc3;

	SVector temp;
	
	PrimFace4   *p_f4;
	PrimFace3   *p_f3;
	PrimObject  *p_obj;
	NIGHT_Found *nf;

	POLY_Point *pp;
	POLY_Point *ps;

	POLY_Point *tri [3];
	POLY_Point *quad[4];
	SLONG	tex_page_offset;


	LOG_ENTER ( Figure_Draw_Prim_Tween )

	tex_page_offset=p_thing->Genus.Person->pcom_colour&0x3;

	//
	// Matrix functions we use.
	// 

	void matrix_transform   (Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_transformZMY(Matrix31* result, Matrix33* trans, Matrix31* mat2);
	void matrix_mult33      (Matrix33* result, Matrix33* mat1,  Matrix33* mat2);
	
	if (parent_base_mat)
	{
		// we've got hierarchy info!
		
		Matrix31	p;
		p.M[0] = anim_info->OffsetX;
		p.M[1] = anim_info->OffsetY;
		p.M[2] = anim_info->OffsetZ;

		HIERARCHY_Get_Body_Part_Offset(&offset, &p,
									   parent_base_mat, parent_base_pos,
									   parent_curr_mat, parent_curr_pos);
		
		// pass data up the hierarchy
		if (end_pos)
			*end_pos = offset;
	}
	else
	{
		// process at highter resolution
		offset.M[0] = (anim_info->OffsetX << 8) + ((anim_info_next->OffsetX + off_dx - anim_info->OffsetX) * tween);
		offset.M[1] = (anim_info->OffsetY << 8) + ((anim_info_next->OffsetY + off_dy - anim_info->OffsetY) * tween);
		offset.M[2] = (anim_info->OffsetZ << 8) + ((anim_info_next->OffsetZ + off_dz - anim_info->OffsetZ) * tween);

		if (end_pos)
		{
			*end_pos = offset;
		}
	}


	// convert pos to floating point here to preserve accuracy and prevent overflow.
	// It's also a shitload faster on P2 and SH4.
	float	off_x = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[0][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[0][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[0][2]) / 32768.f);
	float	off_y = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[1][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[1][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[1][2]) / 32768.f);
	float	off_z = (float(offset.M[0]) / 256.f) * (float(rot_mat->M[2][0]) / 32768.f) +
				    (float(offset.M[1]) / 256.f) * (float(rot_mat->M[2][1]) / 32768.f) + 
				    (float(offset.M[2]) / 256.f) * (float(rot_mat->M[2][2]) / 32768.f);


	SLONG	character_scale  = person_get_scale(p_thing);
	float	character_scalef = float(character_scale) / 256.f;
	
	off_x *= character_scalef;
	off_y *= character_scalef;
	off_z *= character_scalef;

	off_x += float(x);
	off_y += float(y);
	off_z += float(z);

	//
	// Do everything in floats.
	//

	float fmatrix[9];
	//SLONG imatrix[9];

	//
	// Create a temporary "tween" matrix between current and next
	//

	CMatrix33	m1, m2;
	GetCMatrix(anim_info, &m1);
	GetCMatrix(anim_info_next, &m2);

	CQuaternion::BuildTween(&mat2, &m1, &m2, tween);

	// pass data up the hierarchy
	if (end_mat)
		*end_mat = mat2;

	//
	// Apply local rotation matrix to get mat_final that rotates
	// the point into world space.
	//

	matrix_mult33(&mat_final, rot_mat, &mat2);

	//!  yeehaw!
	mat_final.M[0][0] = (mat_final.M[0][0] * character_scale) / 256;
	mat_final.M[0][1] = (mat_final.M[0][1] * character_scale) / 256;
	mat_final.M[0][2] = (mat_final.M[0][2] * character_scale) / 256;
	mat_final.M[1][0] = (mat_final.M[1][0] * character_scale) / 256;
	mat_final.M[1][1] = (mat_final.M[1][1] * character_scale) / 256;
	mat_final.M[1][2] = (mat_final.M[1][2] * character_scale) / 256;
	mat_final.M[2][0] = (mat_final.M[2][0] * character_scale) / 256;
	mat_final.M[2][1] = (mat_final.M[2][1] * character_scale) / 256;
	mat_final.M[2][2] = (mat_final.M[2][2] * character_scale) / 256;

	fmatrix[0] = float(mat_final.M[0][0]) * (1.0F / 32768.0F);
	fmatrix[1] = float(mat_final.M[0][1]) * (1.0F / 32768.0F);
	fmatrix[2] = float(mat_final.M[0][2]) * (1.0F / 32768.0F);
	fmatrix[3] = float(mat_final.M[1][0]) * (1.0F / 32768.0F);
	fmatrix[4] = float(mat_final.M[1][1]) * (1.0F / 32768.0F);
	fmatrix[5] = float(mat_final.M[1][2]) * (1.0F / 32768.0F);
	fmatrix[6] = float(mat_final.M[2][0]) * (1.0F / 32768.0F);
	fmatrix[7] = float(mat_final.M[2][1]) * (1.0F / 32768.0F);
	fmatrix[8] = float(mat_final.M[2][2]) * (1.0F / 32768.0F);

#if 0
	imatrix[0] = mat_final.M[0][0] * 2;
	imatrix[1] = mat_final.M[0][1] * 2;
	imatrix[2] = mat_final.M[0][2] * 2;
	imatrix[3] = mat_final.M[1][0] * 2;
	imatrix[4] = mat_final.M[1][1] * 2;
	imatrix[5] = mat_final.M[1][2] * 2;
	imatrix[6] = mat_final.M[2][0] * 2;
	imatrix[7] = mat_final.M[2][1] * 2;
	imatrix[8] = mat_final.M[2][2] * 2;
#endif


	LOG_ENTER ( Figure_Set_Rotation )

	POLY_set_local_rotation(
		off_x,
		off_y,
		off_z,
		fmatrix);

	LOG_ENTER ( Figure_Set_Rotation )


	//
	// Rotate all the points into the POLY_buffer.
	//

	p_obj = &prim_objects[prim];

	sp = p_obj->StartPoint;
	ep = p_obj->EndPoint;

	POLY_buffer_upto = 0;

	// Check for being a gun
	if (prim==256)
	{
		i=sp;		
	}
	else if (prim==258)
	{
		// Or a shotgun
		i=sp+15;
	}
	else if (prim==260)
	{
		// or an AK
		i=sp+32;
	}
	else
	{
		goto no_muzzle_calcs; // which skips...
	}


	//
	// this bit, which only executes if one of the above tests is true.
    //
	pp = &POLY_buffer[POLY_buffer_upto]; // no ++, so reused
	pp->x=AENG_dx_prim_points[i].X;
	pp->y=AENG_dx_prim_points[i].Y;
	pp->z=AENG_dx_prim_points[i].Z;
	MATRIX_MUL(
		fmatrix,
		pp->x,
		pp->y,
		pp->z);

	pp->x+=off_x; pp->y+=off_y; pp->z+=off_z;
	p_thing->Genus.Person->GunMuzzle.X=pp->x*256;
	p_thing->Genus.Person->GunMuzzle.Y=pp->y*256;
	p_thing->Genus.Person->GunMuzzle.Z=pp->z*256;
//	x=pp->x*256; y=pp->y*256; z=pp->z*256;


no_muzzle_calcs:



#if !USE_TOMS_ENGINE_PLEASE_BOB
#error Dont use this rout if USE_TOMS_ENGINE_PLEASE_BOB is not 1
#endif


	ASSERT ( MM_bLightTableAlreadySetUp );

	if (WITHIN(prim, 261, 263))
	{
		//
		// This is a muzzle flash! They don't have any lighting!
		//

		for (i = sp; i < ep; i++)
		{
			ASSERT(WITHIN(POLY_buffer_upto, 0, POLY_BUFFER_SIZE - 1));

			pp = &POLY_buffer[POLY_buffer_upto++];

			POLY_transform_using_local_rotation(
				AENG_dx_prim_points[i].X,
				AENG_dx_prim_points[i].Y,
				AENG_dx_prim_points[i].Z,
				pp);

			pp->colour   = 0xff808080;
			pp->specular = 0xff000000;
		}


		for (i = p_obj->StartFace4; i < p_obj->EndFace4; i++)
		{
			p_f4 = &prim_faces4[i];

			p0 = p_f4->Points[0] - sp;
			p1 = p_f4->Points[1] - sp;
			p2 = p_f4->Points[2] - sp;
			p3 = p_f4->Points[3] - sp;
			
			ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p3, 0, POLY_buffer_upto - 1));

			quad[0] = &POLY_buffer[p0];
			quad[1] = &POLY_buffer[p1];
			quad[2] = &POLY_buffer[p2];
			quad[3] = &POLY_buffer[p3];

			if (POLY_valid_quad(quad))
			{
				quad[0]->u = float(p_f4->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				quad[0]->v = float(p_f4->UV[0][1]       ) * (1.0F / 32.0F);
														
				quad[1]->u = float(p_f4->UV[1][0]       ) * (1.0F / 32.0F);
				quad[1]->v = float(p_f4->UV[1][1]       ) * (1.0F / 32.0F);
														
				quad[2]->u = float(p_f4->UV[2][0]       ) * (1.0F / 32.0F);
				quad[2]->v = float(p_f4->UV[2][1]       ) * (1.0F / 32.0F);

				quad[3]->u = float(p_f4->UV[3][0]       ) * (1.0F / 32.0F);
				quad[3]->v = float(p_f4->UV[3][1]       ) * (1.0F / 32.0F);

				page   = p_f4->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f4->TexturePage;

				if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
				{
					page=alt_texture[page-10*64]+tex_page_offset-1;
				}
				else
					page+=FACE_PAGE_OFFSET;

				POLY_add_quad(quad, page, !(p_f4->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}

		for (i = p_obj->StartFace3; i < p_obj->EndFace3; i++)
		{
			p_f3 = &prim_faces3[i];

			p0 = p_f3->Points[0] - sp;
			p1 = p_f3->Points[1] - sp;
			p2 = p_f3->Points[2] - sp;
			
			ASSERT(WITHIN(p0, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p1, 0, POLY_buffer_upto - 1));
			ASSERT(WITHIN(p2, 0, POLY_buffer_upto - 1));

			tri[0] = &POLY_buffer[p0];
			tri[1] = &POLY_buffer[p1];
			tri[2] = &POLY_buffer[p2];

			if (POLY_valid_triangle(tri))
			{
				tri[0]->u = float(p_f3->UV[0][0] & 0x3f) * (1.0F / 32.0F);
				tri[0]->v = float(p_f3->UV[0][1]       ) * (1.0F / 32.0F);
														
				tri[1]->u = float(p_f3->UV[1][0]       ) * (1.0F / 32.0F);
				tri[1]->v = float(p_f3->UV[1][1]       ) * (1.0F / 32.0F);
														
				tri[2]->u = float(p_f3->UV[2][0]       ) * (1.0F / 32.0F);
				tri[2]->v = float(p_f3->UV[2][1]       ) * (1.0F / 32.0F);

				page   = p_f3->UV[0][0] & 0xc0;
				page <<= 2;
				page  |= p_f3->TexturePage;

				if(tex_page_offset && page>10*64 && alt_texture[page-10*64])
				{
					page=alt_texture[page-10*64]+tex_page_offset-1;
				}
				else
					page+=FACE_PAGE_OFFSET;

				POLY_add_triangle(tri, page, !(p_f3->DrawFlags & POLY_FLAG_DOUBLESIDED));
			}
		}

		LOG_EXIT ( Figure_Draw_Prim_Tween )
		return;
	}
	else
	{



		ASSERT ( MM_bLightTableAlreadySetUp );

		LOG_ENTER ( Figure_Build_Matrices )

		extern float POLY_cam_matrix_comb[9];
		extern float POLY_cam_off_x;
		extern float POLY_cam_off_y;
		extern float POLY_cam_off_z;


		extern D3DMATRIX g_matProjection;
		extern D3DMATRIX g_matWorld;
		extern D3DVIEWPORT2 g_viewData;


		D3DMATRIX matTemp;

		matTemp._11 = g_matWorld._11*g_matProjection._11 + g_matWorld._12*g_matProjection._21 + g_matWorld._13*g_matProjection._31 + g_matWorld._14*g_matProjection._41;
		matTemp._12 = g_matWorld._11*g_matProjection._12 + g_matWorld._12*g_matProjection._22 + g_matWorld._13*g_matProjection._32 + g_matWorld._14*g_matProjection._42;
		matTemp._13 = g_matWorld._11*g_matProjection._13 + g_matWorld._12*g_matProjection._23 + g_matWorld._13*g_matProjection._33 + g_matWorld._14*g_matProjection._43;
		matTemp._14 = g_matWorld._11*g_matProjection._14 + g_matWorld._12*g_matProjection._24 + g_matWorld._13*g_matProjection._34 + g_matWorld._14*g_matProjection._44;

		matTemp._21 = g_matWorld._21*g_matProjection._11 + g_matWorld._22*g_matProjection._21 + g_matWorld._23*g_matProjection._31 + g_matWorld._24*g_matProjection._41;
		matTemp._22 = g_matWorld._21*g_matProjection._12 + g_matWorld._22*g_matProjection._22 + g_matWorld._23*g_matProjection._32 + g_matWorld._24*g_matProjection._42;
		matTemp._23 = g_matWorld._21*g_matProjection._13 + g_matWorld._22*g_matProjection._23 + g_matWorld._23*g_matProjection._33 + g_matWorld._24*g_matProjection._43;
		matTemp._24 = g_matWorld._21*g_matProjection._14 + g_matWorld._22*g_matProjection._24 + g_matWorld._23*g_matProjection._34 + g_matWorld._24*g_matProjection._44;

		matTemp._31 = g_matWorld._31*g_matProjection._11 + g_matWorld._32*g_matProjection._21 + g_matWorld._33*g_matProjection._31 + g_matWorld._34*g_matProjection._41;
		matTemp._32 = g_matWorld._31*g_matProjection._12 + g_matWorld._32*g_matProjection._22 + g_matWorld._33*g_matProjection._32 + g_matWorld._34*g_matProjection._42;
		matTemp._33 = g_matWorld._31*g_matProjection._13 + g_matWorld._32*g_matProjection._23 + g_matWorld._33*g_matProjection._33 + g_matWorld._34*g_matProjection._43;
		matTemp._34 = g_matWorld._31*g_matProjection._14 + g_matWorld._32*g_matProjection._24 + g_matWorld._33*g_matProjection._34 + g_matWorld._34*g_matProjection._44;

		matTemp._41 = g_matWorld._41*g_matProjection._11 + g_matWorld._42*g_matProjection._21 + g_matWorld._43*g_matProjection._31 + g_matWorld._44*g_matProjection._41;
		matTemp._42 = g_matWorld._41*g_matProjection._12 + g_matWorld._42*g_matProjection._22 + g_matWorld._43*g_matProjection._32 + g_matWorld._44*g_matProjection._42;
		matTemp._43 = g_matWorld._41*g_matProjection._13 + g_matWorld._42*g_matProjection._23 + g_matWorld._43*g_matProjection._33 + g_matWorld._44*g_matProjection._43;
		matTemp._44 = g_matWorld._41*g_matProjection._14 + g_matWorld._42*g_matProjection._24 + g_matWorld._43*g_matProjection._34 + g_matWorld._44*g_matProjection._44;







		// Now make up the matrices.

#if 0
		// Officially correct version.
		DWORD dwWidth = g_viewData.dwWidth >> 1;
		DWORD dwHeight = g_viewData.dwHeight >> 1;
		DWORD dwX = g_viewData.dwX;
		DWORD dwY = g_viewData.dwY;
#else
		// Version that knows about the letterbox mode hack.
extern DWORD g_dw3DStuffHeight;
extern DWORD g_dw3DStuffY;
		DWORD dwWidth = g_viewData.dwWidth >> 1;
		DWORD dwHeight = g_dw3DStuffHeight >> 1;
		DWORD dwX = g_viewData.dwX;
		DWORD dwY = g_dw3DStuffY;
#endif
		MM_pMatrix[0]._11 = 0.0f;
		MM_pMatrix[0]._12 = matTemp._11 *   (float)dwWidth  + matTemp._14 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._13 = matTemp._12 * - (float)dwHeight + matTemp._14 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._14 = matTemp._14;
		MM_pMatrix[0]._21 = 0.0f;
		MM_pMatrix[0]._22 = matTemp._21 *   (float)dwWidth  + matTemp._24 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._23 = matTemp._22 * - (float)dwHeight + matTemp._24 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._24 = matTemp._24;
		MM_pMatrix[0]._31 = 0.0f;
		MM_pMatrix[0]._32 = matTemp._31 *   (float)dwWidth  + matTemp._34 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._33 = matTemp._32 * - (float)dwHeight + matTemp._34 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._34 = matTemp._34;
		// Validation magic number.
		unsigned long EVal = 0xe0001000;
		MM_pMatrix[0]._41 = *(float *)&EVal;
		MM_pMatrix[0]._42 = matTemp._41 *   (float)dwWidth  + matTemp._44 * (float)( dwX + dwWidth  );
		MM_pMatrix[0]._43 = matTemp._42 * - (float)dwHeight + matTemp._44 * (float)( dwY + dwHeight );
		MM_pMatrix[0]._44 = matTemp._44;



		// 251 is a magic number for the DIP call!
		const float fNormScale = 251.0f;

		// Transform the lighting direction(s) by the inverse object matrix to get it into object space.
		// Assume inverse=transpose.
		D3DVECTOR vTemp;
		vTemp.x = MM_vLightDir.x * fmatrix[0] + MM_vLightDir.y * fmatrix[3] + MM_vLightDir.z * fmatrix[6];
		vTemp.y = MM_vLightDir.x * fmatrix[1] + MM_vLightDir.y * fmatrix[4] + MM_vLightDir.z * fmatrix[7];
		vTemp.z = MM_vLightDir.x * fmatrix[2] + MM_vLightDir.y * fmatrix[5] + MM_vLightDir.z * fmatrix[8];

		MM_pNormal[0] = 0.0f;
		MM_pNormal[1] = vTemp.x * fNormScale;
		MM_pNormal[2] = vTemp.y * fNormScale;
		MM_pNormal[3] = vTemp.z * fNormScale;

		LOG_EXIT ( Figure_Build_Matrices )
	}



	// The wonderful NEW system!


	LOG_ENTER ( Figure_Draw_Polys )

#if 1
	// The MM stuff doesn't like specular to be enabled.
	(the_display.lp_D3D_Device)->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, FALSE );
#endif


	// For now, just calculate as-and-when.
	TomsPrimObject *pPrimObj = &(D3DObj[prim]);
	if ( pPrimObj->wNumMaterials == 0 )
	{
		// Not initialised. Do so.
		// It's not fair to count this as part of the drawing! :-)
		LOG_EXIT ( Figure_Draw_Polys )

		FIGURE_generate_D3D_object ( prim );
		LOG_ENTER ( Figure_Draw_Polys )
	}

	// Tell the LRU cache we used this one.
	FIGURE_touch_LRU_of_object ( pPrimObj );

	ASSERT ( pPrimObj->pD3DVertices != NULL );
	ASSERT ( pPrimObj->pMaterials != NULL );
	ASSERT ( pPrimObj->pwListIndices != NULL );
	ASSERT ( pPrimObj->pwStripIndices != NULL );
	ASSERT ( pPrimObj->wNumMaterials != 0 );

	PrimObjectMaterial *pMat = pPrimObj->pMaterials;

	D3DMULTIMATRIX d3dmm;
	d3dmm.lpd3dMatrices = MM_pMatrix;
	d3dmm.lpvLightDirs = MM_pNormal;

	D3DVERTEX *pVertex = (D3DVERTEX *)pPrimObj->pD3DVertices;
	UWORD *pwListIndices = pPrimObj->pwListIndices;
	UWORD *pwStripIndices = pPrimObj->pwStripIndices;
	for ( int iMatNum = pPrimObj->wNumMaterials; iMatNum > 0; iMatNum-- )
	{
		// Set up the right texture for this material.

		UWORD wPage = pMat->wTexturePage;
		UWORD wRealPage = wPage & TEXTURE_PAGE_MASK;

		if ( wPage & TEXTURE_PAGE_FLAG_JACKET )
		{
			// Find the real jacket page.
			wRealPage = jacket_lookup [ wRealPage ][GET_SKILL(p_thing)>>2];
			wRealPage += FACE_PAGE_OFFSET;
		}
		else if ( wPage & TEXTURE_PAGE_FLAG_OFFSET )
		{
			// An "offset" texture. This will be offset by a certain amount to
			// allow each prim to have different coloured clothes on.
			if ( tex_page_offset == 0 )
			{
				// No lookup offset.
				// This has not been offset yet.
				wRealPage += FACE_PAGE_OFFSET;
			}
			else
			{
				// Look this up.
				wRealPage = alt_texture[wRealPage-(10*64)]+tex_page_offset-1;
			}
		}

#ifdef DEBUG
		if ( wPage & TEXTURE_PAGE_FLAG_NOT_TEXTURED )
		{
			ASSERT ( wRealPage == POLY_PAGE_COLOUR );
		}
#endif

extern D3DMATRIX g_matWorld;

		PolyPage *pa = &(POLY_Page[wRealPage]);
		// Not sure if I'm using character_scalef correctly...
		ASSERT ( ( character_scalef < 1.2f ) && ( character_scalef > 0.8f ) );
		ASSERT ( !pa->RS.NeedsSorting() && ( FIGURE_alpha == 255 ) );
		if ( ( ( ( g_matWorld._43 * 32768.0f ) - ( pPrimObj->fBoundingSphereRadius * character_scalef ) ) > ( POLY_ZCLIP_PLANE * 32768.0f ) ) )
		{
			// Non-alpha path.
			if ( wPage & TEXTURE_PAGE_FLAG_TINT )
			{
				// Tinted colours.
				d3dmm.lpLightTable = MM_pcFadeTableTint;
			}
			else
			{
				// Normal.
				d3dmm.lpLightTable = MM_pcFadeTable;
			}
			d3dmm.lpvVertices = pVertex;



#if 1



#ifdef DEBUG
static int iCounter = 0;
			if ( iCounter != 0 )
			{
				iCounter--;
				ASSERT ( iCounter != 0 );
			}
#endif

			// Fast as lightning.
			LOG_ENTER ( Figure_Set_RenderState )
			pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();
			LOG_EXIT ( Figure_Set_RenderState )
			LOG_ENTER ( Figure_DrawIndPrimMM )


#if 0
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			//TRACE("Drew %i vertices, %i indices\n", (int)( pMat->wNumVertices ), (int)( pMat->wNumStripIndices ) );
#else
			// Use platform-independent version.

			HRESULT hres;




#define SHOW_ME_FIGURE_DEBUGGING_PLEASE_BOB defined

#ifdef SHOW_ME_FIGURE_DEBUGGING_PLEASE_BOB
#ifdef DEBUG
#ifdef TARGET_DC
#define BUTTON_IS_PRESSED(value) ((value&0x80)!=0)
extern DIJOYSTATE the_state;
			bool bShowDebug = FALSE;
			if ( BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_LTRIGGER] ) && BUTTON_IS_PRESSED ( the_state.rgbButtons[DI_DC_BUTTON_RTRIGGER] ) )
			{
				DWORD dwColour = (DWORD)pwStripIndices;
				dwColour = ( dwColour >> 2 ) ^ ( dwColour >> 6 ) ^ ( dwColour ) ^ ( dwColour << 3 );
				dwColour = ( dwColour << 9 ) ^ ( dwColour << 19 ) ^ ( dwColour ) ^ ( dwColour << 29 ) ^ ( dwColour >> 3 );
				dwColour &= 0x7f7f7f7f;
				for ( int i = 0; i < 128; i++ )
				{
					d3dmm.lpLightTable[i] = dwColour;
				}

				// And NULL texture (i.e. white).
				the_display.lp_D3D_Device->SetTexture ( 0, NULL );
			}
#endif
#endif
#endif



			//if (pMat->wNumVertices &&
			//	pMat->wNumStripIndices)
			{
				//TRACE ( "S4" );
				hres = DrawIndPrimMM (
						(the_display.lp_D3D_Device),
						D3DFVF_VERTEX,
						&d3dmm,
						pMat->wNumVertices,
						pwStripIndices,
						pMat->wNumStripIndices );
				//TRACE ( "F4" );
			}
#endif




#else

			// Do some performance tracing.
#ifndef DTRACE
#error Don't use this codepath unless DTRACING, fool!
#endif

			LOG_ENTER ( Figure_Set_RenderState )
			pa->RS.SetRenderState ( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );
			pa->RS.SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			pa->RS.SetRenderState ( D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
			pa->RS.SetChanged();

			WORD wTempVerts[4];
			wTempVerts[0] = 0;
			wTempVerts[1] = 1;
			wTempVerts[2] = 2;
			wTempVerts[3] = -1;
			(the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					3,
					wTempVerts,
					4,
					D3DDP_MULTIMATRIX );
			LOG_EXIT ( Figure_Set_RenderState )

			LOG_ENTER ( Figure_DrawIndPrimMM )
			HRESULT hres = (the_display.lp_D3D_Device)->DrawIndexedPrimitive (
					D3DPT_TRIANGLELIST,
					D3DFVF_VERTEX,
					(void *)&d3dmm,
					pMat->wNumVertices,
					pwStripIndices,
					pMat->wNumStripIndices,
					D3DDP_MULTIMATRIX );
			//TRACE("Drew %i vertices, %i indices\n", (int)( pMat->wNumVertices ), (int)( pMat->wNumStripIndices ) );

#endif

//			ASSERT ( SUCCEEDED ( hres ) );  //triggers all the time when inside on start of RTA
			LOG_EXIT ( Figure_DrawIndPrimMM )

		}
		else
		{
			// Alpha/clipped path - do with standard non-MM calls.
			// FIXME. Needs to be done.

			// Actually, the fast-accept works very well, and it's only when the camera somehow gets REALLY close
			// that this happens. And actually a pop-reject seems a bit better than a clip. Certainly
			// there is no visually "right" thing to do. So leave it for now until someone complains. ATF.
			//TRACE ( "Tried to draw an alpha/clipped prim!" );
		}
		


		// Next material
		pVertex += pMat->wNumVertices;
		pwListIndices += pMat->wNumListIndices;
		pwStripIndices += pMat->wNumStripIndices;

		pMat++;
	}

#if 1
	// The MM stuff doesn't like specular to be enabled.
	(the_display.lp_D3D_Device)->SetRenderState ( D3DRENDERSTATE_SPECULARENABLE, TRUE );
#endif

	LOG_EXIT ( Figure_Draw_Polys )



	// No environment mapping.
	ASSERT ( p_thing && ( p_thing->Class != CLASS_VEHICLE ) );

	ASSERT ( MM_bLightTableAlreadySetUp );

	LOG_EXIT ( Figure_Draw_Prim_Tween )

}








