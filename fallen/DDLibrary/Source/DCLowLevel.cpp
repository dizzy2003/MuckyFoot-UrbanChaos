// ========================================================
//
// THIS IS RIPPED FROM THE DC EXAMPLE CODE
//
// ========================================================


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1996, 1997 Microsoft Corporation

Module Name:

    DSUtil.cpp

Abstract:

   Contains routines for handling sounds from resources

-------------------------------------------------------------------*/


#include <tchar.h>
#include <windows.h>
#include "MFStdLib.h"
#include <dsound.h>
#include <math.h>
#ifdef TARGET_DC
#include <floatmathlib.h>

// On DC, it's called "pows" for some reason ("s" = "single"?)
#define powf pows

#endif

// ++++ Global Variables ++++++++++++++++++++++++++++++++++++++++++++
LPDIRECTSOUND			g_pds = NULL;              // The DirectSound object
LPDIRECTSOUND3DLISTENER g_pds3dl = NULL;
LPDIRECTSOUNDBUFFER		g_pdsbPrimary = NULL;

HRESULT                 g_errLast;

#define CheckError(anything) (FALSE)


#ifdef TARGET_DC
// Stream Yamaha ADPCM, not normal PCM.
#define STREAMING_BUFFER_IS_ADPCM defined

// Pants hack for the ADPCM buffer.
//#define CRAPPY_STREAMING_ADPCM_HACK defined
#endif


//extern HWND g_hwndApp;

extern volatile HWND hDDLibWindow;


bool m_bSoundHasActuallyStartedPlaying = FALSE;

#define DCLL_OUTPUT_STEREO_WAVS

#ifdef DCLL_OUTPUT_STEREO_WAVS
FILE *DCLL_handle;
#endif


#ifdef DEBUG
#define SHARON TRACE
//#define SHARON sizeof
#else
#define SHARON sizeof
#endif



inline DWORD ReadNonalignedDword ( void *addr )
{
	DWORD res;
	unsigned char *myaddr = (unsigned char *)addr;

	res  = ( ( *myaddr++ ) & 0xff );
	res |= ( ( *myaddr++ ) & 0xff ) << 8;
	res |= ( ( *myaddr++ ) & 0xff ) << 16;
	res |= ( ( *myaddr++ ) & 0xff ) << 24;

	return ( res );
}

inline WORD ReadNonalignedWord ( void *addr )
{
	WORD res;
	char *myaddr = (char *)addr;

	res  = ( ( *myaddr++ ) & 0xff );
	res |= ( ( *myaddr++ ) & 0xff ) << 8;

	return ( res );
}



void *DwordMemcpy ( void *pvDst, const void *pvSrc, size_t dwSize )
{

	//TRACE ( "DwordMemcpy arrived with 0x%x, 0x%x, 0x%x\n", pvDst, pvSrc, dwSize );

	if ( ( (DWORD)pvSrc & 0x3 ) == 0 )
	{
		// Source is DWORD-aligned.
		const DWORD *pdwSrc = (DWORD *)pvSrc;
		DWORD *pdwDst = (DWORD *)pvDst;
		ASSERT ( ( (DWORD)pdwSrc & 0x3 ) == 0 );
		ASSERT ( ( (DWORD)pdwDst & 0x3 ) == 0 );
		ASSERT ( ( dwSize & 0x3 ) == 0 );
		dwSize >>= 2;
		while ( dwSize > 0 )
		{
			*pdwDst++ = *pdwSrc++;
			dwSize--;
		}
	}
	else
	{

#ifdef DEBUG
		// Check endianness
		DWORD dwTest = 0x12345678;
		ASSERT ( *(((char*)&dwTest)+0) == 0x78 );
		ASSERT ( *(((char*)&dwTest)+1) == 0x56 );
		ASSERT ( *(((char*)&dwTest)+2) == 0x34 );
		ASSERT ( *(((char*)&dwTest)+3) == 0x12 );
#endif

		// Source is not dword aligned!
		const unsigned char *pdwSrc = (unsigned char *)pvSrc;
		DWORD *pdwDst = (DWORD *)pvDst;
		//ASSERT ( ( (DWORD)pdwSrc & 0x3 ) == 0 );
		ASSERT ( ( (DWORD)pdwDst & 0x3 ) == 0 );
		ASSERT ( ( dwSize & 0x3 ) == 0 );
		dwSize >>= 2;
		while ( dwSize > 0 )
		{
			DWORD dwTemp = (DWORD)*pdwSrc++;
			dwTemp |= ( (DWORD)(*pdwSrc++) ) << 8;
			dwTemp |= ( (DWORD)(*pdwSrc++) ) << 16;
			dwTemp |= ( (DWORD)(*pdwSrc++) ) << 24;
			*pdwDst++ = dwTemp;
			dwSize--;
		}
	}
	return pvDst;
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

    InitDirectSound

Description:

    Initialize the DirectSound object

Arguments:

    None

Return Value:

    TRUE on success, FALSE on failure.

-------------------------------------------------------------------*/
BOOL
InitDirectSound()
{
    // Create the DirectSound object
    g_errLast = DirectSoundCreate(NULL, &g_pds, NULL);

    if (CheckError(TEXT("DirectSoundCreate")))
        return FALSE;

    // Set the DirectSound cooperative level
    if (g_pds->SetCooperativeLevel(hDDLibWindow, DSSCL_NORMAL) != DS_OK)
	{
		ASSERT(0);
	}

    return TRUE;
}


BOOL
InitDirectSound3D()
{
    DSBUFFERDESC    dsbd;

    // Create the primary sound buffer (needed for the listener interface)
    memset(&dsbd, 0, sizeof(dsbd));
    dsbd.dwSize  = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
    g_errLast = g_pds->CreateSoundBuffer(&dsbd, &g_pdsbPrimary, NULL);
    if (CheckError(TEXT("Create SoundBuffer")))
        return FALSE;

    // Get a pointer to the IDirectSound3DListener interface
    g_errLast = g_pdsbPrimary->QueryInterface(IID_IDirectSound3DListener, (void **)&g_pds3dl);
    if (CheckError(TEXT("QueryInterface for IDirectSound3DListener interface")))
        return FALSE;

	//
	// How many metres in an UC block? 256 => 2 metres?
	//

	g_pds3dl->SetDistanceFactor(0.25F / 256.0F, DS3D_IMMEDIATE);

    // We no longer need the primary buffer, just the Listener interface
	// g_pdsbPrimary->Release();

    // Set the doppler factor to the maximum, so we can more easily notice it
    // g_errLast = g_pds3dl->SetDopplerFactor(DS3D_MAXDOPPLERFACTOR, DS3D_IMMEDIATE);

	//
	// Set the primary buffer playing all the time to avoid nasty clicks...
	//

	g_pdsbPrimary->Play(0,0,0);

    return TRUE;
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

    ParseWaveFile

Description:

    Get the Wave File header, size, and data pointer...

Arguments:

    void         *pvWaveFile     -  Pointer to the wav file to parse

    WAVEFORMATEX **ppWaveHeader  -  Fill this with pointer to wave header

    BYTE         **ppbWaveData   -  Fill this with pointer to wave data

    DWORD        **pcbWaveSize   -  Fill this with wave data size.

Return Value:

    TRUE on success, FALSE on failure.

-------------------------------------------------------------------*/
BOOL
ParseWaveFile(void *pvWaveFile, WAVEFORMATEX **ppWaveHeader, BYTE **ppbWaveData, DWORD *pcbWaveSize)
{
    DWORD *pdw;
    DWORD *pdwEnd;
    DWORD dwRiff;
    DWORD dwType;
    DWORD dwLength;

    if (ppWaveHeader)
        *ppWaveHeader = NULL;

    if (ppbWaveData)
        *ppbWaveData = NULL;

    if (pcbWaveSize)
        *pcbWaveSize = 0;

    pdw = (DWORD *)pvWaveFile;
    dwRiff   = *pdw++;
    dwLength = *pdw++;
    dwType   = *pdw++;

    // Check if it's a WAV format file
    if (dwType != mmioFOURCC('W', 'A', 'V', 'E'))
	{
		ASSERT ( FALSE );
        return FALSE;
	}

    // Check if it's a RIFF format file
    if (dwRiff != mmioFOURCC('R', 'I', 'F', 'F'))
	{
		ASSERT ( FALSE );
        return FALSE;
	}

    pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);

    while (pdw < pdwEnd)
    {
#if 0
        dwType = *pdw++;
        dwLength = *pdw++;
#else
		dwType = ReadNonalignedDword ( pdw );
		pdw++;
		dwLength = ReadNonalignedDword ( pdw );
		pdw++;
#endif

        switch (dwType)
        {
        case mmioFOURCC('f', 'm', 't', ' '):
            if (ppWaveHeader && !*ppWaveHeader)
            {
                if (dwLength < sizeof(WAVEFORMAT))
                    return FALSE;

                *ppWaveHeader = (WAVEFORMATEX *)pdw;

                if ((!ppbWaveData || *ppbWaveData) && (!pcbWaveSize || *pcbWaveSize))
                    return TRUE;
            }
            break;

        case mmioFOURCC('d', 'a', 't', 'a'):
            if ((ppbWaveData && !*ppbWaveData) || (pcbWaveSize && !*pcbWaveSize))
            {
                if (ppbWaveData)
                    *ppbWaveData = (LPBYTE)pdw;

                if (pcbWaveSize)
                    *pcbWaveSize = dwLength;

                if (!ppWaveHeader || *ppWaveHeader)
                    return TRUE;
            }
            break;
		default:
			ASSERT ( FALSE );
			return FALSE;
			break;
        }

        pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
    }

    return FALSE;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

    GetWaveResource

Description:

    Load a WAV file from the executable's Resource file or the specified file.

Arguments:

    LPCTSTR      tszName         -  Name of the WAV file to load

    WAVEFORMATEX **ppWaveHeader  -  Fill this with pointer to wave header

    BYTE         **ppbWaveData   -  Fill this with pointer to wave data

    DWORD        **pcbWaveSize   -  Fill this with wave data size.

Return Value:

    TRUE on success, FALSE on failure.

	NOTE! Return values of **ppWaveHeader, **ppbWaveData, **pcbWaveSize
	are owned by this rout. Do not do anything permanent to them, coz
	the memory blocks will be frazzed next time this is called.

-------------------------------------------------------------------*/


// For some bizarre reason, they keep failing every now and then.
#define USE_ALIGNED_LOADS 0

static DWORD dwSizeOfGWRBlock = 0;
static void *pvGWRBlock = NULL;

BOOL
GetWaveResource(char *szName, WAVEFORMATEX **ppWaveHeader,
                BYTE **ppbWaveData, DWORD *pcbWaveSize)
{
	//HRSRC			hResInfo;
	//HGLOBAL			hResData;
    //void			*pvRes;
    MFFileHandle	hFile;
    //static BYTE		*rgbyFileTemp = NULL;
    unsigned long	cbRead;
    DWORD			dwSize;

	/*

    // Find the specifed WAV resource
    hResInfo = FindResource(g_hinst, tszName, TEXT("WAV"));
    if (hResInfo == NULL)
        goto TryFile;

    // Load the Resource
    hResData = LoadResource(g_hinst, hResInfo);
    if (hResData == NULL)
        goto TryFile;

    // Lock the Resource
    pvRes = LockResource(hResData);
    if (pvRes == NULL)
        goto TryFile;

    // Read and parse the Resource
    if (ParseWaveFile(pvRes, ppWaveHeader, ppbWaveData, pcbWaveSize) != NULL)
        return TRUE;

	*/

//TryFile:

#ifdef TARGET_DC
	// GDWorkshop converts any non-alphanumerics to "_"
	char chTemp[128];
	char *pchSrc = szName;
	char *pchDst = chTemp;
	while ( TRUE )
	{
		if ( ( ( *pchSrc >= 'a' ) && ( *pchSrc <= 'z' ) ) ||
			 ( ( *pchSrc >= 'A' ) && ( *pchSrc <= 'Z' ) ) ||
			 ( ( *pchSrc >= '0' ) && ( *pchSrc <= '9' ) ) ||
			 ( *pchSrc == '.' ) ||
			 ( *pchSrc == '\\' )
			 )
		{
			*pchDst++ = *pchSrc++;
		}
		else if ( *pchSrc == '\0' )
		{
			*pchDst++ = '\0';
			break;
		}
		else
		{
			*pchDst++ = '_';
			pchSrc++;
		}
	}

	szName = chTemp;
#endif


#if 0
#ifdef DEBUG
	if ( 0 == stricmp ( szName, "data\\sfx\\1622dc\\heli.wav" ) )
	{
		// Replace it for the mo.
		szName = "data\\sfx\\1622dc\\helifucked64.wav";
	}
	else if ( 0 == stricmp ( szName, "data\\sfx\\1622dc\\sfx080799_\\search2.wav" ) )
	{
		// Replace it for the mo.
		szName = "data\\sfx\\1622dc\\sfx080799_\\search2fucked32.wav";
	}
#endif
#endif




    hFile = FileOpen ( szName );
    if ( hFile == FILE_OPEN_ERROR )
	{
		TRACE ( "Failed to load sound %s\n", szName );
        return FALSE;
	}

    dwSize = FileSize(hFile);

#if !USE_ALIGNED_LOADS
	// Free memory used in previous call.
	if ( pvGWRBlock != NULL )
	{
		MemFree ( pvGWRBlock );
		pvGWRBlock = NULL;
	}
    pvGWRBlock = (BYTE*) MemAlloc ( dwSize );
    if ( !pvGWRBlock )
	{
		ASSERT ( FALSE );
        return FALSE;
	}
#else
	if ( dwSizeOfGWRBlock < dwSize )
	{
		if ( pvGWRBlock != NULL )
		{
			//MemFree ( pvGWRBlock );
			VirtualFree ( pvGWRBlock, NULL, MEM_RELEASE );
		}
		// Grow slightly more than needed to prevent hammering.
		dwSizeOfGWRBlock = ( dwSize * 5 / 4 + 10240 );
		// Ensure it's 4k-aligned.
		dwSizeOfGWRBlock = ( ( dwSizeOfGWRBlock + 4095 ) & ~4095 );
		//pvGWRBlock = MemAlloc ( dwSizeOfGWRBlock );
		pvGWRBlock = VirtualAlloc ( NULL, dwSizeOfGWRBlock, MEM_COMMIT, PAGE_READWRITE );
		ASSERT ( pvGWRBlock != NULL );
	}
	//rgbyFileTemp = (BYTE *)pvGWRBlock;

#endif

#if !USE_ALIGNED_LOADS
	// Load in one go.
    cbRead = FileRead ( hFile, pvGWRBlock, dwSize );
#else
	// Use DMA load, then finish the rest.

	int iAlignedFileSize = dwSize & ( ~4095 );
	// DMA read
	if ( iAlignedFileSize > 0 )
	{
		cbRead = FileRead ( hFile, pvGWRBlock, iAlignedFileSize );
	}
	else
	{
		cbRead = 0;
	}
	// Finish off with PIO or whatever.
	if ( dwSize - iAlignedFileSize > 0 )
	{
		cbRead += FileRead ( hFile, (void *)( (char *)pvGWRBlock + iAlignedFileSize ), dwSize - iAlignedFileSize );
	}
#endif
    FileClose ( hFile );

#if 0

    hFile = CreateFile(tszName, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
	{
        return FALSE;
	}

    dwSize = GetFileSize(hFile, NULL);

    pvGWRBlock = (BYTE*) MemAlloc(dwSize);
    if (!pvGWRBlock)
        return FALSE;

    ReadFile (hFile, pvGWRBlock, dwSize, &cbRead, NULL);
    CloseHandle(hFile);

#endif

    if (cbRead != dwSize)
	{
		ASSERT ( FALSE );
#if !USE_ALIGNED_LOADS
		//MemFree ( pvGWRBlock );
#endif
        return FALSE;
	}

    if (ParseWaveFile(pvGWRBlock, ppWaveHeader, ppbWaveData, pcbWaveSize) == NULL)
	{
#if !USE_ALIGNED_LOADS
		//MemFree ( pvGWRBlock );
#endif
        return FALSE;
	}


	// This is NOT freed until next call, since it holds all the data that the pointer all point into.
#if !USE_ALIGNED_LOADS
	//MemFree ( pvGWRBlock );
#endif

    return TRUE;
}



// Call this when a bunch of sounds have finished loading.
// Can be called whenever you like really.
void DCLL_ProbablyDoneMostOfMySoundLoadingForAWhile ( void )
{
#if !USE_ALIGNED_LOADS
	if ( pvGWRBlock != NULL )
	{
		MemFree ( pvGWRBlock );
		pvGWRBlock = NULL;
	}
#else
	if ( pvGWRBlock != NULL )
	{
		//MemFree ( pvGWRBlock );
		VirtualFree ( pvGWRBlock, NULL, MEM_RELEASE );
		pvGWRBlock = NULL;
	}
#endif
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

    FillSoundBuffer

Description:

    Copies the Sound data to the specified DirecSoundBuffer's data file

Arguments:

    LPCTSTR      tszName         -  Name of the WAV file to load

    WAVEFORMATEX **ppWaveHeader  -  Fill this with pointer to wave header

    BYTE         **ppbWaveData   -  Fill this with pointer to wave data

    DWORD        **pcbWaveSize   -  Fill this with wave data size.

Return Value:

    TRUE on success, FALSE on failure.

-------------------------------------------------------------------*/
BOOL
FillSoundBuffer(IDirectSoundBuffer *pdsb, BYTE *pbWaveData, DWORD dwWaveSize)
{
    LPVOID pMem1, pMem2;
    DWORD  dwSize1, dwSize2;

    if (!pdsb || !pbWaveData || !dwWaveSize)
        return FALSE;

    g_errLast = pdsb->Lock(0, dwWaveSize, &pMem1, &dwSize1, &pMem2, &dwSize2, 0);

	switch(g_errLast)
	{
		case DS_OK:
			break;

		case DSERR_BUFFERLOST:		OutputDebugString("DSERR_BUFFERLOST:      \n"); break;
		case DSERR_INVALIDCALL:		OutputDebugString("DSERR_INVALIDCALL:     \n"); break;
		case DSERR_INVALIDPARAM:	OutputDebugString("DSERR_INVALIDPARAM:    \n"); break;
		case DSERR_PRIOLEVELNEEDED:	OutputDebugString("DSERR_PRIOLEVELNEEDED: \n"); break;

		default:
			ASSERT(0);
			break;
	}

    if (CheckError(TEXT("Lock SoundBuffer")))
        return FALSE;


// Have you learned nothing of what I have taught you, young Adami?
#if 1
    memcpy(pMem1, pbWaveData, dwSize1);

    if (dwSize2 != 0)
        memcpy(pMem2, pbWaveData+dwSize1, dwSize2);
#else
	//TRACE ( "DwordMemcpy  called with 0x%x, 0x%x, 0x%x\n", pMem1, pbWaveData, dwSize1 );
    DwordMemcpy(pMem1, pbWaveData, dwSize1);

    if (dwSize2 != 0)
	{
		//TRACE ( "DwordMemcpy  called with 0x%x, 0x%x, 0x%x\n", pMem2, pbWaveData+dwSize1, dwSize2 );
        DwordMemcpy(pMem2, pbWaveData+dwSize1, dwSize2);
	}

#endif

    pdsb->Unlock(pMem1, dwSize1, pMem2, dwSize2);

    return TRUE;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

    LoadSoundBuffer

Description:

    Creates a DirectSoundBuffer and loads the specified file into it.

Arguments:

    LPCTSTR      tszName         -  Name of the WAV file to load

Return Value:

    TRUE on success, FALSE on failure.

-------------------------------------------------------------------*/

SLONG DCLL_bytes_of_sound_memory_used = 0;

#ifdef DEBUG
DWORD dwTotalSampleSize = 0;
#endif

IDirectSoundBuffer *
LoadSoundBuffer(char *szName, SLONG is_3d)
{
    IDirectSoundBuffer *pdsb = NULL;
    DSBUFFERDESC dsbd = {0};
    BYTE *pbWaveData;

	DCLL_bytes_of_sound_memory_used = 0;



#if 0
	// Just testing.
	return NULL;
#else



    if (GetWaveResource(szName, &dsbd.lpwfxFormat, &pbWaveData, &dsbd.dwBufferBytes))
    {
        dsbd.dwSize = sizeof(dsbd);
        dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2;

		#ifdef DCLL_OUTPUT_STEREO_WAVS

		if (dsbd.lpwfxFormat->nChannels == 2)
		{
			//
			// This is a stereo sample.
			//

			fprintf(DCLL_handle, szName);
			fprintf(DCLL_handle, "\r\n");

			fflush(DCLL_handle);
		}

		#endif

		if (is_3d)
		{
			//dsbd.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
		}

#ifdef TARGET_DC
		// Do a compact, just in case we got fragmentation, though it's unlikely.
		HRESULT hres = g_pds->Compact();
		ASSERT ( SUCCEEDED ( hres ) );
#endif

		pdsb = NULL;

#ifdef DEBUG
		// See if we're out of memory or not.
		DSCAPS dscaps;
		ZeroMemory ( &dscaps, sizeof(dscaps) );
		dscaps.dwSize = sizeof(dscaps);
		g_pds->GetCaps ( &dscaps );
		if ( dscaps.dwMaxContigFreeHwMemBytes < 0x8000 )
		{
			SHARON ( "Running very low on sound memory - 0x%x bytes left.\n", dscaps.dwMaxContigFreeHwMemBytes );
			// And don't bother to load it.
			goto yuk_didnt_like_that_sound;
		}

		if ( dsbd.dwBufferBytes > 0x8000 )
		{
			SHARON ( "Tried to load <%s> more than 32k in length - bad person!\n", szName );
			goto yuk_didnt_like_that_sound;
		}
#endif

        if (SUCCEEDED(g_pds->CreateSoundBuffer(&dsbd, &pdsb, NULL)))
        {
            if (!FillSoundBuffer(pdsb, pbWaveData, dsbd.dwBufferBytes))
            {
                pdsb->Release();
                pdsb = NULL;
            }

			DCLL_bytes_of_sound_memory_used = dsbd.dwBufferBytes;


			#ifdef TARGET_DC

			DSBCAPS dsbcaps;
			ZeroMemory ( (void *)&dsbcaps, sizeof ( dsbcaps ) );
			dsbcaps.dwSize = sizeof ( dsbcaps );
			pdsb->GetCaps ( &dsbcaps );

			if ( dsbcaps.dwFlags & DSBCAPS_LOCHARDWARE )
			{
				//SHARON ( "Hardware buffer 0x%x bytes, %i CPU overhead\n", dsbcaps.dwBufferBytes, dsbcaps.dwPlayCpuOverhead );
				// Make sure this is the right length.
				if ( ( dsbd.dwBufferBytes & 31 ) != 0 )
				{
					TRACE ( "Hardware buffer wasn't a multiple of 32 bytes in length - don't loop it! %i \n", (dsbd.dwBufferBytes & 31) );
				}
				if ( dsbd.dwBufferBytes > 16383 )
				{
					TRACE ( "Hardware buffer more than 32k samples - don't loop it! %i \n", (dsbd.dwBufferBytes) );
				}
			}
			else
			{
				SHARON ( "Software buffer <%s>, 0x%x bytes, %iHz\n", szName, dsbcaps.dwBufferBytes, dsbd.lpwfxFormat->nSamplesPerSec );
				SHARON ( "Binning it for now.\n" );
				ASSERT ( FALSE );
				pdsb->Release();
				pdsb = NULL;
			}

#if 0
#ifdef DEBUG

			CBYTE *words_for_11khz[] =
			{
				"Suspension",
				"Scrape",
				"Balrog",
				"Footstep",
				"PAIN",
				"TAUNT",
				"darci\\",
				"roper\\"
				"bthug1\\",
				"wthug1\\",
				"wthug2\\",
				"cop\\COP",
				"misc\\",
				"Barrel",
				"Ambience",
				"CarX",
				"!"
			};

			// Shall we chop these down even more?
			bool bChopIt = FALSE;
			for (int j = 0; words_for_11khz[j][0] != '!'; j++)
			{
				if (strstr(szName, words_for_11khz[j]))
				{
					bChopIt = TRUE;
					break;
				}
			}


			if ( bChopIt )
			{
				// Chop it down to 8kHz.
				dwTotalSampleSize += ( dsbcaps.dwBufferBytes * 8000 / dsbd.lpwfxFormat->nSamplesPerSec );
			}
			else if ( dsbd.lpwfxFormat->nSamplesPerSec > 11100 )
			{
				// Pretend you halved the sample rate.
				dwTotalSampleSize += dsbcaps.dwBufferBytes / 2;
			}
			else
			{
				dwTotalSampleSize += dsbcaps.dwBufferBytes;
			}
			SHARON ( "Total desired sample memory 0x%x\n", dwTotalSampleSize );
#endif
#endif

			#endif

        }
        else
		{
            pdsb = NULL;
		}
yuk_didnt_like_that_sound:;
    }

#endif

    return pdsb;
}


IDirectSoundBuffer *CreateStreamingSoundBuffer(int nSamplesPerSec, WORD wBitsPerSample, DWORD dwBufferSize)
{
    IDirectSoundBuffer *pdsb        = NULL;
    DSBUFFERDESC       dsbd         = {0};
    WAVEFORMATEX       waveformatex = {0};

#ifdef STREAMING_BUFFER_IS_ADPCM
    // Set up the Wave format description for Yamaha ADPCM
    waveformatex.wFormatTag      = 0x0020;
    waveformatex.nChannels       = 1;
    waveformatex.nSamplesPerSec  = 22050;
    waveformatex.wBitsPerSample  = 4;
    waveformatex.nBlockAlign     = (waveformatex.nChannels * waveformatex.wBitsPerSample) / 8;
    waveformatex.nAvgBytesPerSec = waveformatex.nSamplesPerSec * waveformatex.nBlockAlign;
    waveformatex.cbSize          = 0;
    dsbd.dwSize                  = sizeof(dsbd);
    dsbd.dwBufferBytes           = dwBufferSize;
    dsbd.dwFlags                 = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY;// | DSBCAPS_LOCSOFTWARE;
    dsbd.lpwfxFormat             = &waveformatex;
#else
    // Set up the Wave format description
    waveformatex.wFormatTag      = WAVE_FORMAT_PCM;
    waveformatex.nChannels       = 1;
    waveformatex.nSamplesPerSec  = nSamplesPerSec;
    waveformatex.wBitsPerSample  = wBitsPerSample;
    waveformatex.nBlockAlign     = (waveformatex.nChannels * waveformatex.wBitsPerSample) / 8;
    waveformatex.nAvgBytesPerSec = waveformatex.nSamplesPerSec * waveformatex.nBlockAlign;
    waveformatex.cbSize          = 0;
    dsbd.dwSize                  = sizeof(dsbd);
    dsbd.dwBufferBytes           = dwBufferSize;
    dsbd.dwFlags                 = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY;// | DSBCAPS_LOCSOFTWARE;
    dsbd.lpwfxFormat             = &waveformatex;
#endif

    g_errLast = g_pds->CreateSoundBuffer(&dsbd, &pdsb, NULL);
    if (CheckError(TEXT("Create DirectSound Buffer")))
        return NULL;

    return pdsb;
}




















// ========================================================
//
// FROM HERE ONWARDS IT'S MY CODE!
//
// ========================================================

#include "dclowlevel.h"


typedef struct dcll_sound
{
	SLONG used;
	IDirectSoundBuffer   *dsb;
	IDirectSound3DBuffer *dsb_3d;

} DCLL_Sound;

#define DCLL_MAX_SOUNDS 1024

DCLL_Sound DCLL_sound[DCLL_MAX_SOUNDS];






//
// Nasty streaming stuff...
//

// How many bytes to do at a time.
#define DCLL_GRANULARITY 256

#ifdef STREAMING_BUFFER_IS_ADPCM

// We need a much smaller buffer for 1 second's worth.
#define DCLL_STREAM_BUFFER_SIZE ( DCLL_GRANULARITY * 64 )

#else
//#define DCLL_STREAM_BUFFER_SIZE (22050 * sizeof(UWORD) * 1)		// Number of bytes for 1 seconds of 22khz 16-bit mono sound
#define DCLL_STREAM_BUFFER_SIZE ( DCLL_GRANULARITY * 128 )
#endif


#define DCLL_STREAM_COMMAND_NOTHING         0
#define DCLL_STREAM_COMMAND_START_NEW_SOUND 1

SLONG DCLL_stream_loop;
SLONG DCLL_stream_command;
CBYTE DCLL_stream_new_fname[MAX_PATH];
SLONG DCLL_stream_data_offset;
SLONG DCLL_stream_silence_count;

IDirectSoundBuffer *DCLL_stream_dsb;
MFFileHandle        DCLL_stream_file_handle;	// The file handle of what we are streaming...
HANDLE              DCLL_stream_event;
HANDLE              DCLL_stream_thread;			// Streaming sound thread
float				DCLL_stream_volume_range = 1.0F;


// Used for buffering streaming reads.
char pcDCLL_stream_buffer[DCLL_GRANULARITY];


void DCLL_stream_set_volume_range(float max_vol)
{
	SATURATE(max_vol, 0.0F, 1.0F);

	//DCLL_stream_volume_range = max_vol;

	DCLL_stream_volume(max_vol);

}


#ifdef DEBUG

#define NUM_TRACIES 16
#define MAX_LENGTH_TRACIE 100


char pcTracieString[NUM_TRACIES][MAX_LENGTH_TRACIE];
int m_iDumpedTracieNum = 0;
int m_iTracieNum = 0;


void Tracie ( char *fmt, ... )
{
	va_list va;
	va_start ( va, fmt );
	vsprintf ( pcTracieString[m_iTracieNum++], fmt, va );
	if ( m_iTracieNum >= NUM_TRACIES )
	{
		m_iTracieNum = 0;
	}
	va_end ( va );
}


void DumpTracies ( void )
{
	while ( m_iDumpedTracieNum != m_iTracieNum )
	{
		TRACE ( pcTracieString[m_iDumpedTracieNum] );
		m_iDumpedTracieNum++;
		if ( m_iDumpedTracieNum >= NUM_TRACIES )
		{
			m_iDumpedTracieNum = 0;
		}
	}
}


// A sort of pseudo-trace thingie for threads.
// Use it like TRACIE(("thing %i\n", iThing ));
#define TRACIE(arg) Tracie arg;


#else

#define TRACIE sizeof

#endif




// Set this to 0 for new-fangled goodness on the DC.
#define OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL 0



//
// The thread that handles streaming.
//

DWORD DCLL_stream_process(int nUnused)
{
	while(1)
	{
		WaitForSingleObject(DCLL_stream_event, INFINITE);

		//
		// What's hapenned to our event?
		//

#if 0
		// Debugging - rip out this.
		DCLL_stream_command = DCLL_STREAM_COMMAND_NOTHING;
		if (DCLL_stream_file_handle != NULL)
		{
			// Close it.
			FileClose ( DCLL_stream_file_handle );
			DCLL_stream_file_handle = NULL;
		}
#else


		switch(DCLL_stream_command)
		{
			case DCLL_STREAM_COMMAND_NOTHING:

				{
					DWORD write_pos;
					SLONG start;

					//
					// Find out which bit of the buffer we should overwrite.
					//

					DWORD dummy;
#ifdef TARGET_DC
					// We need to find the write position.
#if OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL
					DCLL_stream_dsb->GetCurrentPosition(&dummy, &write_pos);
#else
					DCLL_stream_dsb->GetCurrentPosition(&write_pos, &dummy);
#endif
#else
					// Different on the PC.
					DCLL_stream_dsb->GetCurrentPosition(&write_pos, &dummy);
#endif



					// Very occasionally, the read position is just before the end of the buffer, which causes this to fall over.
					// So add a little something to tweak it.
#if OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL
					if ( write_pos >= ( DCLL_STREAM_BUFFER_SIZE / 2 ) )
#else

#define SOUND_POS_TWEAK_FACTOR 64
					if ( ( write_pos >= ( ( DCLL_STREAM_BUFFER_SIZE / 2 ) - SOUND_POS_TWEAK_FACTOR ) ) &&
						 ( write_pos <  ( ( DCLL_STREAM_BUFFER_SIZE ) - SOUND_POS_TWEAK_FACTOR ) ) )
#endif
					{
						//
						// Overwrite the first half.
						//

						start = 0;
						TRACIE (( "Write pos 0x%x - overwriting first half\n", write_pos ));
					}
					else
					{
						//
						// Overwrite the second half.
						//

						start = DCLL_STREAM_BUFFER_SIZE / 2;
						TRACIE (( "Write pos 0x%x - overwriting second half\n", write_pos ));
					}

					if (DCLL_stream_file_handle == NULL)
					{
						//
						// No more sound data, so fill the sound buffer
						// with silence.
						//

						DCLL_stream_silence_count += 1;

						if (DCLL_stream_silence_count >= 2)
						{
							//
							// We can safely stop the sound from playing.
							//

							DCLL_stream_silence_count = 0;

							DCLL_stream_dsb->Stop();

							break;
						}
					}

					UBYTE *block1_mem = NULL;
					UBYTE *block2_mem = NULL;
					DWORD block1_length;
					DWORD block2_length;

#ifdef TARGET_DC
#if OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL
					DCLL_stream_dsb->Lock(start, DCLL_STREAM_BUFFER_SIZE / 2, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, DSBLOCK_FROMWRITECURSOR );
#else
					DCLL_stream_dsb->Lock(start, DCLL_STREAM_BUFFER_SIZE / 2, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, 0 );
#endif
#else
					DCLL_stream_dsb->Lock(start, DCLL_STREAM_BUFFER_SIZE / 2, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, 0 );
#endif



					if ( block1_mem == NULL )
					{
						// NADS! Has happened though. Which is pretty damn scary.
						ASSERT ( FALSE );
						TRACE (( "Eek - the Lock just failed for the sound\n." ));
					}
					else
					{

						if (DCLL_stream_file_handle == NULL)
						{
							//
							// Silence...
							//

							// Can't use memset - must use DWORD writes.
							//memset(block1_mem, 0, block1_length);
							DWORD *dst1 = (DWORD *)block1_mem;
							DWORD count = block1_length;

							#ifdef TARGET_DC

							ASSERT ( ( (DWORD)dst1 & 3 ) == 0 );
							ASSERT ( ( count & 3 ) == 0 );

							#endif

							count >>= 2;
							while ( (count--) > 0 )
							{
								*dst1++ = 0;
							}
						}
						else
						{
							DWORD bytes_read;

							//
							// Read sound data from the file.
							//

	#ifdef TARGET_DC
							// The version that streams in 256 byte chunks.
							ASSERT ( ( block1_length & (DCLL_GRANULARITY-1) ) == 0 );
							DWORD dwBytesToWrite = block1_length;
							UBYTE *pbBlockMem = block1_mem;
							while ( dwBytesToWrite > 0 )
							{
								if ( DCLL_stream_file_handle != NULL )
								{
									bytes_read = FileRead ( DCLL_stream_file_handle, pcDCLL_stream_buffer, DCLL_GRANULARITY );
									if ( bytes_read < DCLL_GRANULARITY )
									{
										if ( DCLL_stream_loop )
										{
											// Start from the beginning again.
											FileSeek(DCLL_stream_file_handle, SEEK_MODE_BEGINNING, DCLL_stream_data_offset);
											DWORD new_bytes_read = FileRead ( DCLL_stream_file_handle, pcDCLL_stream_buffer + bytes_read, DCLL_GRANULARITY - bytes_read );
											ASSERT ( new_bytes_read == ( DCLL_GRANULARITY - bytes_read ) );
										}
										else
										{
											// Need to pad with silence.
											memset ( pcDCLL_stream_buffer + bytes_read, 0, DCLL_GRANULARITY - bytes_read );
											FileClose ( DCLL_stream_file_handle );
											DCLL_stream_file_handle = NULL;
										}
									}
								}
								else
								{
									bytes_read = 0;
									memset ( pcDCLL_stream_buffer, 0, DCLL_GRANULARITY );
								}

								// And copy the buffer in. MUST BE IN DWORDS.
								DWORD *pdwSrc = (DWORD*)pcDCLL_stream_buffer;
								DWORD *pdwDst = (DWORD*)pbBlockMem;
								for ( int i = 0; i < ( DCLL_GRANULARITY / 4 ); i++ )
								{
									*pdwDst++ = *pdwSrc++;
								}
								dwBytesToWrite -= DCLL_GRANULARITY;
								pbBlockMem += DCLL_GRANULARITY;
							}
	#else



	#if 0
							ReadFile(
								DCLL_stream_file_handle,
								block1_mem,
								block1_length,
							   &bytes_read,
								NULL);
	#else
							bytes_read = FileRead ( DCLL_stream_file_handle, block1_mem, block1_length );
	#endif

							if (bytes_read < block1_length)
							{
								if (DCLL_stream_loop)
								{
									//
									// Go back to the beginning of the data section of the file and
									// start looping again.
									//

									FileSeek(DCLL_stream_file_handle, SEEK_MODE_BEGINNING, DCLL_stream_data_offset);
									// SetFilePointer(DCLL_stream_file_handle, DCLL_stream_data_offset, NULL, FILE_BEGIN);

									FileRead(DCLL_stream_file_handle, block1_mem + bytes_read, block1_length - bytes_read);
								}
								else
								{
									//
									// Fill the remainder of the buffer with silence and close the file.
									//

									// Can't use memset - must be DWORD writes.
									//memset(block1_mem + bytes_read, 0, block1_length - bytes_read);
									DWORD *dst1 = (DWORD *)( block1_mem + bytes_read );
									DWORD count = block1_length - bytes_read;

									#ifdef TARGET_DC

									ASSERT ( ( (DWORD)dst1 & 3 ) == 0 );
									ASSERT ( ( count & 3 ) == 0 );

									#endif

									count >>= 2;
									while ( (count--) > 0 )
									{
										*dst1++ = 0;
									}

	#if 0
									CloseHandle(DCLL_stream_file_handle);
	#else
									FileClose ( DCLL_stream_file_handle );
	#endif

									DCLL_stream_file_handle = NULL;
								}
							}
#endif
						}

						/*

						memset(block1_mem, 0, block1_length);

						if (start)
						{
							SLONG i;

							for (i = 0; i < block1_length; i++)
							{
								block1_mem[i] = rand();
							}
						}

						*/

	#ifdef TARGET_DC
						// This has started falling over on the PC. But I don't care.
						ASSERT(block2_mem == NULL);
	#endif

						DCLL_stream_dsb->Unlock(block1_mem, block1_length, block2_mem, block2_length);
					}
				}

				break;

			case DCLL_STREAM_COMMAND_START_NEW_SOUND:

				{
					DWORD status = 0;

					DCLL_stream_dsb->GetStatus(&status);

					if (status & (DSBSTATUS_LOOPING|DSBSTATUS_PLAYING))
					{
						//
						// If the buffer is playing, then we  close
						// the current file handle and stop the sound playing.
						//

						if (DCLL_stream_file_handle)
						{
							FileClose(DCLL_stream_file_handle);

							DCLL_stream_file_handle   = NULL;
							DCLL_stream_silence_count = 0;
						}

						DCLL_stream_dsb->Stop();
					}

					{
						BYTE          temp[256];
						ULONG         bytes_read;
						DWORD         size;
						WAVEFORMATEX *pwfx;
						BYTE         *data_start;

						//
						// Open the sound file.
						//

#ifdef CRAPPY_STREAMING_ADPCM_HACK
						// Always load this
						if ( strstr ( DCLL_stream_new_fname, "FrontLoop" ) )
						{
							strcpy ( DCLL_stream_new_fname, "Data\\sfx\\1622DC\\GeneralMusic\\FrontLoopMONOAD.wav" );
						}
						else
						{
							strcpy ( DCLL_stream_new_fname, "Data\\sfx\\1622DC\\whoryou.wav" );
						}
#endif


#if 0
						DCLL_stream_file_handle = CreateFile(DCLL_stream_new_fname, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
						if (DCLL_stream_file_handle == NULL)
#else
						DCLL_stream_file_handle = FileOpen ( DCLL_stream_new_fname );
						if (DCLL_stream_file_handle == FILE_OPEN_ERROR )
#endif
						{
							DCLL_stream_file_handle = NULL;
							bytes_read = 0;
						}
						else
						{
							//
							// Read the first 256 bytes to get file header
							//

#if 0
							ReadFile(DCLL_stream_file_handle, temp, 256, &bytes_read, NULL);
#else
							bytes_read = FileRead ( DCLL_stream_file_handle, temp, 256 );
#endif
						}

						if (bytes_read != 256)
						{
							DCLL_stream_file_handle = NULL;

							DCLL_stream_dsb->Stop();
						}
						else
						{
							//
							// Parse the header information to get to the sound data.
							//

							ParseWaveFile((void *) temp, &pwfx, &data_start, &size);

							//
							// Set file pointer to point to start of data
							//

							DCLL_stream_data_offset = (SLONG) (data_start - temp);
#if 0
							SetFilePointer(DCLL_stream_file_handle, DCLL_stream_data_offset, NULL, FILE_BEGIN);
#else
							FileSeek ( DCLL_stream_file_handle, SEEK_MODE_BEGINNING, (int) (data_start - temp) );
#endif

							//
							// Fill up the sound buffer with the data.
							//

							UBYTE *block1_mem = NULL;
							UBYTE *block2_mem = NULL;
							DWORD block1_length;
							DWORD block2_length;

							DCLL_stream_dsb->Lock(0, 0, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, DSBLOCK_ENTIREBUFFER);

							if (block1_mem)
							{
								DWORD bytes_read;

								//
								// Read sound data from the file.
								//

#ifndef TARGET_DC

#if 0
								ReadFile(
									DCLL_stream_file_handle,
									block1_mem,
									block1_length,
								   &bytes_read,
									NULL);
#else
								bytes_read = FileRead ( DCLL_stream_file_handle, block1_mem, block1_length );
#endif

								if (bytes_read < block1_length)
								{
									//
									// Fill the remainder of the buffer with silence and close the file.
									//

									// Can't use memset - must use DWORD writes.
									//memset(block1_mem + bytes_read, 0, block1_length - bytes_read);
									DWORD *dst1 = (DWORD *)( block1_mem + bytes_read );
									DWORD count = block1_length - bytes_read;
									ASSERT ( ( (DWORD)dst1 & 3 ) == 0 );
									ASSERT ( ( count & 3 ) == 0 );
									count >>= 2;
									while ( (count--) > 0 )
									{
										*dst1++ = 0;
									}

#if 0
									CloseHandle(DCLL_stream_file_handle);
#else
									FileClose ( DCLL_stream_file_handle );
#endif
									DCLL_stream_file_handle = NULL;
								}

#else
// The version that streams in 256 byte chunks.
								ASSERT ( ( block1_length & (DCLL_GRANULARITY-1) ) == 0 );
								DWORD dwBytesToWrite = block1_length;
								UBYTE *pbBlockMem = block1_mem;
								while ( dwBytesToWrite > 0 )
								{
									if ( DCLL_stream_file_handle != NULL )
									{
										bytes_read = FileRead ( DCLL_stream_file_handle, pcDCLL_stream_buffer, DCLL_GRANULARITY );
										if ( bytes_read < DCLL_GRANULARITY )
										{
											// Need to pad with silence.
											memset ( pcDCLL_stream_buffer + bytes_read, 0, DCLL_GRANULARITY - bytes_read );
											FileClose ( DCLL_stream_file_handle );
											DCLL_stream_file_handle = NULL;
										}
									}
									else
									{
										bytes_read = 0;
										memset ( pcDCLL_stream_buffer, 0, DCLL_GRANULARITY );
									}

									// And copy the buffer in. MUST BE IN DWORDS.
									DWORD *pdwSrc = (DWORD*)pcDCLL_stream_buffer;
									DWORD *pdwDst = (DWORD*)pbBlockMem;
									for ( int i = 0; i < ( DCLL_GRANULARITY / 4 ); i++ )
									{
										*pdwDst++ = *pdwSrc++;
									}
									dwBytesToWrite -= DCLL_GRANULARITY;
									pbBlockMem += DCLL_GRANULARITY;
								}
#endif

							}

							DCLL_stream_dsb->Unlock(block1_mem, block1_length, block2_mem, block2_length);

							ASSERT(block2_mem == NULL);

							//
							// Start the buffer playing.
							//

							DCLL_stream_dsb->SetCurrentPosition(0);
							DCLL_stream_dsb->Play(0, 0, DSBPLAY_LOOPING);
						}
					}
				}

				DCLL_stream_command = DCLL_STREAM_COMMAND_NOTHING;

				break;

			default:
				ASSERT(0);
				break;
		}



#endif

	}

	return 0;

}

void DCLL_stream_init(void)
{
	DWORD thread_id;

	//
	// Create the event that we use to communicate with
	// the streaming thread.
	//

	DCLL_stream_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	//
	// Create the streaming buffer...
	//

	DCLL_stream_dsb = CreateStreamingSoundBuffer(22050, 16, DCLL_STREAM_BUFFER_SIZE);

	//
	// Spawn off the new thread.
	//

	DCLL_stream_thread  = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) DCLL_stream_process, NULL, 0, &thread_id);
	ASSERT(DCLL_stream_thread);
#ifdef TARGET_DC
	SetThreadPriority ( DCLL_stream_thread, THREAD_PRIORITY_HIGHEST );
#endif

	//
	// Setup notifications...
	//

    IDirectSoundNotify *pdsn;
	DSBPOSITIONNOTIFY   rgdsbpn[2];

	/*

	IDirectSoundBuffer_QueryInterface(
		DCLL_stream_dsb,
		IID_IDirectSoundNotify,
		(void **)&pdsn);

	*/

	DCLL_stream_dsb->QueryInterface(IID_IDirectSoundNotify, (void **)&pdsn);

    rgdsbpn[0].hEventNotify = DCLL_stream_event;
    rgdsbpn[1].hEventNotify = DCLL_stream_event;
    rgdsbpn[0].dwOffset     = DCLL_STREAM_BUFFER_SIZE / 2;
    rgdsbpn[1].dwOffset     = DCLL_STREAM_BUFFER_SIZE - 2;

    pdsn->SetNotificationPositions(2, rgdsbpn);

	//
    // No longer need the DirectSoundNotify interface, so release it
	//

    pdsn->Release();
}

SLONG DCLL_stream_play(CBYTE *fname, SLONG loop)
{
	if (DCLL_stream_command != DCLL_STREAM_COMMAND_NOTHING)
	{
		//
		// Waiting to play another sample.
		//

		if (loop && !DCLL_stream_loop)
		{
			return FALSE;
		}
	}

	//
	// The command to the thread.
	//

	DCLL_stream_loop    = loop;
	DCLL_stream_command = DCLL_STREAM_COMMAND_START_NEW_SOUND;

#ifdef TARGET_DC
	// Some filenames begin with ".\" which confuses the poor DC
	if ( ( fname[0] == '.' ) && ( fname[1] == '\\' ) )
	{
		fname += 2;
	}
#endif

	// Some missions have empty briefing names.
	if ( fname[0] == '\0' )
	{
		return FALSE;
	}


#ifndef TARGET_DC
	strcpy(DCLL_stream_new_fname, fname);
#else
	// Some filename have more than one '.', which GDWorkshop converts to an underline.
	// They are all of the format blah.ucmfoobar.wav, which changes to
	// blah.ucmfoobar_wav
	char *pcSrc = fname;
	char *pcDst = DCLL_stream_new_fname;
	bool bFoundFullStop = FALSE;
	while ( *pcSrc != '\0' )
	{
		*pcDst = *pcSrc++;
		if ( *pcDst == '.' )
		{
			if ( bFoundFullStop )
			{
				*pcDst = '_';
			}
			else
			{
				bFoundFullStop = TRUE;
			}
		}
		pcDst++;
	}
	*pcDst = '\0';


#if 0
// No longer needed - everything is ADPCM.
#ifdef STREAMING_BUFFER_IS_ADPCM
	// If the file ends in MONO, add a further AD to it.
	int iTemp = strlen ( DCLL_stream_new_fname );
	if ( ( DCLL_stream_new_fname[iTemp-8] == 'M' ) &&
		 ( DCLL_stream_new_fname[iTemp-7] == 'O' ) &&
		 ( DCLL_stream_new_fname[iTemp-6] == 'N' ) &&
		 ( DCLL_stream_new_fname[iTemp-5] == 'O' ) )
	{
		strcpy ( &(DCLL_stream_new_fname[iTemp-4]), "AD.wav" );
	}
#endif
#endif


#endif

	//
	// Wake up the process.
	//

	SetEvent(DCLL_stream_event);

	return TRUE;
}


void DCLL_stream_stop()
{

#ifndef TARGET_DC
	// This rout is buggered on the PC.
	return;
#endif

	//
	// Make sure a sound doesn't sneakily start playing...
	//

	DCLL_stream_command = DCLL_STREAM_COMMAND_NOTHING;

	DCLL_stream_dsb->Stop();

	//
	// Close the streaming file and stop the buffer from playing.
	//

	if (DCLL_stream_file_handle)
	{
#if 0
		CloseHandle(DCLL_stream_file_handle);
#else
		FileClose ( DCLL_stream_file_handle );
#endif

		DCLL_stream_file_handle   = NULL;
		DCLL_stream_silence_count = 0;
	}

	while(1)
	{
		ULONG   status;
		HRESULT res;

		res = DCLL_stream_dsb->GetStatus(&status);

		ASSERT(res == DS_OK);

		if ((status & DSBSTATUS_PLAYING) ||
			(status & DSBSTATUS_LOOPING))
		{
			Sleep(10);

			DCLL_stream_dsb->Stop();
		}
		else
		{
			return;
		}
	}
}

SLONG DCLL_stream_is_playing()
{
	ULONG   status;
	HRESULT res;

	if (DCLL_stream_command == DCLL_STREAM_COMMAND_START_NEW_SOUND)
	{
		return TRUE;
	}

	res = DCLL_stream_dsb->GetStatus(&status);

	ASSERT(res == DS_OK);

	if ((status & DSBSTATUS_PLAYING) ||
		(status & DSBSTATUS_LOOPING))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


// Returns TRUE if the stream has actually started playing,
// i.e. the disk has seeked, etc.
bool DCLL_stream_has_started_streaming ( void )
{
	ULONG   status;
	HRESULT res;

	res = DCLL_stream_dsb->GetStatus(&status);

	ASSERT(res == DS_OK);

	if ((status & DSBSTATUS_PLAYING) ||
		(status & DSBSTATUS_LOOPING))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



void DCLL_stream_wait()
{
	while(1)
	{
		if (DCLL_stream_is_playing())
		{
			Sleep(10);
		}
		else
		{
			return;
		}
	}
}

void DCLL_stream_volume(float volume)
{
	if (DCLL_stream_dsb == NULL)
	{
		return;
	}

	SATURATE(volume, 0.0F, 1.0F);

	// Done elsewhere.
	//volume *= DCLL_stream_volume_range;

	volume = powf( volume, 0.1F);

	SLONG ivolume = SLONG(DSBVOLUME_MIN + (DSBVOLUME_MAX - DSBVOLUME_MIN) * volume);

	DCLL_stream_dsb->SetVolume(ivolume);
}







void DCLL_memstream_init();


void DCLL_init()
{
	InitDirectSound();
	InitDirectSound3D();

	memset(DCLL_sound, 0, sizeof(DCLL_sound));

	DCLL_stream_init();
	DCLL_memstream_init();

	#ifdef DCLL_OUTPUT_STEREO_WAVS

	DCLL_handle = fopen("t:\\stereowavs.txt", "wb");

	#endif
}





DCLL_Sound *DCLL_load_sound(CBYTE *fname)
{
	SLONG i;

	DCLL_Sound *ds;

	for (i = 0; i < DCLL_MAX_SOUNDS; i++)
	{
		ds = &DCLL_sound[i];

		if (!ds->used)
		{
			goto found_unused_sound;
		}
	}

	//
	// No more sounds left!
	//

	ASSERT(0);

	return NULL;

found_unused_sound:;

	ds->used = TRUE;

	ds->dsb  = LoadSoundBuffer(fname, TRUE);

	if (!ds->dsb)
	{
		ds->used = FALSE;

		return NULL;
	}

	ASSERT(ds->dsb);

	(ds->dsb)->QueryInterface(IID_IDirectSound3DBuffer, (void **) &ds->dsb_3d);

	return ds;
}


void DCLL_set_volume(DCLL_Sound *ds, float volume)
{
	if (ds == NULL)
	{
		return;
	}

	SATURATE(volume, 0.0F, 1.0F);

	volume = powf(volume, 0.1F);

	SLONG ivolume = SLONG(DSBVOLUME_MIN + (DSBVOLUME_MAX - DSBVOLUME_MIN) * volume);

	HRESULT hres = ds->dsb->SetVolume(ivolume);

	//TRACE ( "Vol %i\n", ivolume );

}

SLONG	DCLL_still_playing(DCLL_Sound *ds)
{
	ULONG status;
	if(ds==NULL)
		return(0);
	if(ds->dsb==NULL)
		return(0);

	ds->dsb->GetStatus(&status);
	if ((status & DSBSTATUS_PLAYING) || (status & DSBSTATUS_LOOPING))
	{
		return(1);
	}
	else
		return(0);


}


void DCLL_2d_play_sound(DCLL_Sound *ds, SLONG flag)
{
	if (ds == NULL)
	{
		return;
	}

	ULONG status;

	ASSERT ( ds->dsb != NULL );
	if ( ds->dsb_3d != NULL )
	{
		ds->dsb_3d->SetMode(DS3DMODE_DISABLE, DS3D_IMMEDIATE);
	}

	ds->dsb->GetStatus(&status);

	if ((status & DSBSTATUS_PLAYING) ||
		(status & DSBSTATUS_LOOPING))
	{
		if (flag & DCLL_FLAG_INTERRUPT)
		{
			ds->dsb->SetCurrentPosition(0);
		}
	}
	else
	{
		switch(ds->dsb->Play(0, 0, (flag & DCLL_FLAG_LOOP) ? DSBPLAY_LOOPING : 0))
		{
			case DS_OK:
				break;

			case DSERR_PRIOLEVELNEEDED:
				ASSERT(0);
				break;

			default:
				ASSERT(0);
				break;
		}
	}
}

float DCLL_3d_listener_x;
float DCLL_3d_listener_y;
float DCLL_3d_listener_z;



void DCLL_3d_play_sound(DCLL_Sound *ds, float x, float y, float z, SLONG flag)
{
	if (ds == NULL)
	{
		return;
	}

	ASSERT(ds->dsb);
	ASSERT(ds->dsb_3d);

	ds->dsb_3d->SetMode(DS3DMODE_NORMAL, DS3D_IMMEDIATE);

	//
	// Start playing- or restart if it's already playing.
	//

	ULONG status;

	ds->dsb->GetStatus(&status);

	if ((status & DSBSTATUS_PLAYING) ||
		(status & DSBSTATUS_LOOPING))
	{
		if (flag & DCLL_FLAG_INTERRUPT)
		{
			ds->dsb->SetCurrentPosition(0);
		}
	}
	else
	{
		ds->dsb->Play(0,0,(flag & DCLL_FLAG_LOOP) ? DSBPLAY_LOOPING : 0);
	}

	//
	// Set position.
	//

	ds->dsb_3d->SetPosition(x,y,z, DS3D_IMMEDIATE);
}


void DCLL_3d_set_listener(
		float x,
		float y,
		float z,
		float matrix[9])
{
	DCLL_3d_listener_x = x;
	DCLL_3d_listener_y = y;
	DCLL_3d_listener_z = z;

	if (!g_pds3dl)
	{
		return;
	}

	g_pds3dl->SetPosition(x, y, z, DS3D_DEFERRED);

	g_pds3dl->SetOrientation(
		matrix[6],
		matrix[7],
		matrix[8],
		matrix[3],
		matrix[4],
		matrix[5],
		DS3D_DEFERRED);

	g_pds3dl->CommitDeferredSettings();
}


void DCLL_stop_sound(DCLL_Sound *ds)
{
	if (ds == NULL)
	{
		return;
	}

	ds->dsb->Stop();
}


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(arg) if ( (arg) != NULL ) { (arg)->Release(); (arg) = NULL; }
#endif


void DCLL_free_sound(DCLL_Sound *ds)
{
	if (ds == NULL || !ds->used)
	{
		return;
	}

	SAFE_RELEASE ( ds->dsb_3d );
	SAFE_RELEASE ( ds->dsb );
	ds->used   = FALSE;
}


void DCLL_fini(void)
{
	SLONG i;

	DCLL_Sound *ds;

	for (i = 0; i < DCLL_MAX_SOUNDS; i++)
	{
		ds = &DCLL_sound[i];

		if (ds->used)
		{
			DCLL_free_sound(ds);
		}
	}

	//
	// Release objects...
	//

	g_pds3dl->Release();
	g_pdsbPrimary->Release();
	g_pds->Release();

}







void init_my_dialog(HWND h)
{
}

void my_dialogs_over(HWND h)
{
}

void MilesTerm(void)
{
}












#ifndef TARGET_DC

#include "sound_id.h"

//
// Does the DC conversion of looping samples.
//

void DCLL_looping_sample_conversion(void)
{
	SLONG i;

	SLONG looping_sample[] =
	{
		S_SLIDE_START,
		S_SEARCH_END,
		S_ZIPWIRE,
		S_TROPICAL,
		S_RAIN_START,
		S_AMBIENCE_END,
		S_AMB_POLICE1,
		S_AMB_POSHEETA,
		S_AMB_OFFICE1,
		S_TUNE_CLUB_START,
		S_RECKONING_LOOP,
		S_HELI,
		S_TUNE_CLUB,
		S_ACIEEED,
		S_FRONT_END_LOOP_EDIT,
		S_FIRE_HYDRANT,
		S_FIRE,
		S_CAR_SIREN1,
		S_CARX_CRUISE,
		S_CARX_IDLE,
		S_CAR_REVERSE_LOOP,
	   -12345
	};

	CreateDirectory("t:\\SillyDCwavs", NULL);

	//
	// Moves all these files to to the directory on t:\
	//

	FILE *handle = fopen("t:\\SillyDCwavs\\dcconv.bat", "wb");

	if (!handle)
	{
		return;
	}

	fprintf(handle, "mkdir converted\r\n\r\n");

	CBYTE src[256];
	CBYTE dst[256];
	CBYTE jst[256];

	CBYTE *ch;

	for (i = 0; looping_sample[i] >= 0; i++)
	{
		sprintf(src, "data\\sfx\\1622\\%s", sound_list[looping_sample[i]]);

		for (ch = src; *ch; ch++);
		while(*ch != '\\') ch--;
		ch++;

		strcpy(jst, ch);

		sprintf(dst, "t:\\sillyDCwavs\\%s", jst);

		CopyFile(src, dst, FALSE);

		//
		// Mention this filename in the batch file.
		//

		fprintf(handle, "wavcon %s converted\\%s\r\n", jst, jst);
		fprintf(handle, "copy %s n:\\urbanchaos\\thegame\\data\\sfx\\1622DC\\%s /Y\r\n", jst, jst);
		fprintf(handle, "copy converted\\%s n:\\urbanchaos\\thegame\\data\\sfx\\1622DC\\%s /Y\r\n", jst, jst);
		fprintf(handle, "\r\n");
	}

	fclose(handle);
}

#endif





// ========================================================
//
// STREAMING FILES FROM MEMORY
//
// ========================================================

//
// The sound to stream.
//

UBYTE *DCLL_memstream_sound;
SLONG  DCLL_memstream_sound_length;	// length in bytes
UBYTE *DCLL_memstream_sound_upto;		// Where we have played upto.


#define DCLL_MEMSTREAM_BUFFER_SIZE (32768)

IDirectSoundBuffer *DCLL_memstream_dsb;
HANDLE              DCLL_memstream_event;
HANDLE              DCLL_memstream_thread;			// Streaming sound thread


//
// The thread that handles memstreaming.
//

DWORD DCLL_memstream_process(void)
{
	while(1)
	{
		WaitForSingleObject(DCLL_memstream_event, INFINITE);


// Debugging - disable it.
#if 1



		{
			DWORD write_pos;
			SLONG start;

			//
			// Find out which bit of the buffer we should overwrite.
			//

			DWORD dummy;

			#ifdef TARGET_DC
			// We need to find the write position.
#if OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL
			DCLL_memstream_dsb->GetCurrentPosition(&dummy, &write_pos);
#else
			DCLL_memstream_dsb->GetCurrentPosition(&write_pos, &dummy);
#endif
			#else
			// Different on the PC.
			DCLL_memstream_dsb->GetCurrentPosition(&write_pos, &dummy);
			#endif



#if OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL
			if ( write_pos >= ( DCLL_STREAM_BUFFER_SIZE / 2 ) )
#else
			if ( ( write_pos >= ( ( DCLL_STREAM_BUFFER_SIZE / 2 ) - SOUND_POS_TWEAK_FACTOR ) ) &&
				 ( write_pos <  ( ( DCLL_STREAM_BUFFER_SIZE ) - SOUND_POS_TWEAK_FACTOR ) ) )
#endif
			{
				//
				// Overwrite the first half.
				//

				start = 0;
				TRACIE (( "Memstream: writing first half\n" ));
			}
			else
			{
				//
				// Overwrite the second half.
				//

				start = DCLL_MEMSTREAM_BUFFER_SIZE / 2;
				TRACIE (( "Memstream: writing second half\n" ));
			}

			if (DCLL_memstream_sound == NULL)
			{
				//
				// No more sound data so stop the buffer! Something
				// bad must be happening.
				//

				DCLL_memstream_dsb->Stop();
			}

			UBYTE *block1_mem    = NULL;
			UBYTE *block2_mem    = NULL;
			DWORD  block1_length = 0;
			DWORD  block2_length = 0;

#if OLD_STYLE_THING_THAT_DIDNT_WORK_VERY_WELL
			DCLL_memstream_dsb->Lock(start, DCLL_MEMSTREAM_BUFFER_SIZE / 2, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, DSBLOCK_FROMWRITECURSOR);
#else
			DCLL_memstream_dsb->Lock(start, DCLL_MEMSTREAM_BUFFER_SIZE / 2, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, 0);
#endif


			//
			// Fill the buffer with data.
			//

			SLONG  i;
			SLONG  count = block1_length >> 2;
			SLONG *dst   = (SLONG *)  block1_mem;
			SLONG *src   = (SLONG *)  DCLL_memstream_sound_upto;
			SLONG *end   = (SLONG *) (DCLL_memstream_sound + DCLL_memstream_sound_length);

			#ifdef TARGET_DC

			ASSERT((SLONG(dst) & 3) == 0);
			ASSERT((count      & 3) == 0);

			#endif

			for (i = count; src != NULL && i > 0; i--)
			{
				*dst++ = *src++;

				if (src >= end)
				{
					src = (SLONG *) DCLL_memstream_sound;
				}
			}

			DCLL_memstream_sound_upto = (UBYTE *) src;

			DCLL_memstream_dsb->Unlock(block1_mem, block1_length, block2_mem, block2_length);
		}


#endif


	}

	// Doesn't ever exit, but the DC compiler
	// complains that it doesn't return a value.
	return 0;
}



void DCLL_memstream_init()
{
	DWORD thread_id;

	//
	// Create the event that we use to communicate with the
	// memstream thread.
	//

	DCLL_memstream_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	//
	// Create the memstream buffer.
	//

	DCLL_memstream_dsb = CreateStreamingSoundBuffer(22050, 16, DCLL_MEMSTREAM_BUFFER_SIZE);

	//
	// Spawn off the new thread.
	//

	DCLL_memstream_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) DCLL_memstream_process, NULL, 0, &thread_id);

	ASSERT(DCLL_memstream_thread);

	#ifdef TARGET_DC
	SetThreadPriority(DCLL_memstream_thread, THREAD_PRIORITY_HIGHEST);
	#endif

	//
	// Setup notifications...
	//

    IDirectSoundNotify *pdsn;
	DSBPOSITIONNOTIFY   rgdsbpn[2];

	DCLL_memstream_dsb->QueryInterface(IID_IDirectSoundNotify, (void **)&pdsn);

    rgdsbpn[0].hEventNotify = DCLL_memstream_event;
    rgdsbpn[1].hEventNotify = DCLL_memstream_event;
    rgdsbpn[0].dwOffset     = DCLL_MEMSTREAM_BUFFER_SIZE / 2;
    rgdsbpn[1].dwOffset     = DCLL_MEMSTREAM_BUFFER_SIZE - 2;

    if (pdsn->SetNotificationPositions(2, rgdsbpn) != DS_OK)
	{
		//
		//
		//

		ASSERT(0);
	}

	//
    // No longer need the DirectSoundNotify interface, so release it
	//

    pdsn->Release();
}

void DCLL_memstream_load(CBYTE *fname)
{
	BYTE          temp[256];
	ULONG         bytes_read;
	DWORD         size;
	WAVEFORMATEX *pwfx;
	BYTE         *data_start;
	MFFileHandle  handle;

	//
	// Stop the buffer.
	//

	DCLL_memstream_dsb->Stop();

	//
	// Initialise the sound.
	//

	DCLL_memstream_unload();

	//
	// Open the sound file.
	//

	handle = FileOpen(fname);

	if (handle == FILE_OPEN_ERROR )
	{
		ASSERT ( FALSE );
		return;
	}

	//
	// Read the first 256 bytes to get file header
	//

	bytes_read = FileRead(handle, temp, 256);

	if (bytes_read != 256)
	{
		return;
	}

	//
	// Parse the header information to get to the sound data.
	//

	ParseWaveFile((void *) temp, &pwfx, &data_start, &size);

	//
	// Set file pointer to point to start of data
	//

	FileSeek(handle, SEEK_MODE_BEGINNING, (int) (data_start - temp));

	//
	// Allocate the sound.
	//

	DCLL_memstream_sound        = (UBYTE *) MemAlloc(size);
	DCLL_memstream_sound_length = size;
	DCLL_memstream_sound_upto   = DCLL_memstream_sound;

	ASSERT(DCLL_memstream_sound);

	//
	// Load in the sound.
	//

	bytes_read = FileRead(handle, DCLL_memstream_sound, DCLL_memstream_sound_length);

	ASSERT(bytes_read == (unsigned)DCLL_memstream_sound_length);

	//
	// Lock the sound buffer.
	//

	UBYTE *block1_mem    = NULL;
	UBYTE *block2_mem    = NULL;
	DWORD  block1_length = 0;
	DWORD  block2_length = 0;

	DCLL_memstream_dsb->Lock(0, 0, (void **) &block1_mem, &block1_length, (void **) &block2_mem, &block2_length, DSBLOCK_ENTIREBUFFER);

	//
	// Make sure what happens is what we expect.
	//

	ASSERT(block2_mem    == NULL);
	ASSERT(block1_length == DCLL_MEMSTREAM_BUFFER_SIZE);
	ASSERT(block2_length == 0);

	//
	// Fill the buffer with data.
	//

	SLONG  i;
	SLONG  count = DCLL_MEMSTREAM_BUFFER_SIZE >> 2;
	SLONG *dst   = (SLONG *) block1_mem;
	SLONG *src   = (SLONG *) DCLL_memstream_sound;
	SLONG *end   = (SLONG *) (DCLL_memstream_sound + DCLL_memstream_sound_length);

	ASSERT((SLONG(dst) & 3) == 0);
	ASSERT((count      & 3) == 0);

	for (i = count; i > 0; i--)
	{
		*dst++ = *src++;

		if (src >= end)
		{
			src = (SLONG *) DCLL_memstream_sound;
		}
	}

	//
	// Unlock the buffer.
	//

	DCLL_memstream_dsb->Unlock(block1_mem, block1_length, block2_mem, block2_length);


	FileClose(handle);

}

void DCLL_memstream_volume(float volume)
{
	if (DCLL_memstream_dsb == NULL)
	{
		return;
	}

	SATURATE(volume, 0.0F, 1.0F);

	volume = powf( volume, 0.1F);

	SLONG ivolume = SLONG(DSBVOLUME_MIN + (DSBVOLUME_MAX - DSBVOLUME_MIN) * volume);

	DCLL_memstream_dsb->SetVolume(ivolume);
}


void DCLL_memstream_play()
{
	//
	// Start the buffer playing.
	//

	ASSERT(DCLL_memstream_sound);

#if 0
	// Don't reset it! It's so short, and if you reset it,
	// it sounds a bit crappy. Just start playing it again from the
	// current position.
	DCLL_memstream_dsb->SetCurrentPosition(0);
#endif
	DCLL_memstream_dsb->Play(0, 0, DSBPLAY_LOOPING);

	DCLL_memstream_volume(DCLL_stream_volume_range * 0.5F);
}

void DCLL_memstream_stop()
{
	//
	// Stop the sound.
	//

	DCLL_memstream_dsb->Stop();
}

void DCLL_memstream_unload()
{
	Sleep(100);	// Give the streaming thread enough time to finish what it's doing!

	if (DCLL_memstream_sound)
	{
		MemFree(DCLL_memstream_sound);
	}

	DCLL_memstream_sound        = NULL;
	DCLL_memstream_sound_length = 0;
	DCLL_memstream_sound_upto   = NULL;
}

