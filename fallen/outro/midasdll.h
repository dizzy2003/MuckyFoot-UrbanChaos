/*      midasdll.h
 *
 * MIDAS DLL programming interface
 *
 * $Id: midasdll.h,v 1.32 1997/08/15 16:48:25 pekangas Exp $
 *
 * Copyright 1996,1997 Housemarque Inc.
 *
 * This file is part of MIDAS Digital Audio System, and may only be
 * used, modified and distributed under the terms of the MIDAS
 * Digital Audio System license, "license.txt". By continuing to use,
 * modify or distribute this file you indicate that you have
 * read the license and understand and accept it fully.
*/


#ifndef __midasdll_h
#define __midasdll_h


/* This is a kluge, but necessary as Watcom C sucks: */
#ifdef EXPORT_IN_MIDASDLL_H

#ifdef __WC32__
    #define _FUNC(x) x __export __stdcall
    #define MIDAS_CALL __cdecl
#else
    #define _FUNC(x) __declspec(dllexport) x __stdcall
    #define MIDAS_CALL __cdecl
#endif

#else
    #if defined(__linux__) || defined(__UNIX__) || defined(__DJGPP__)
        #define _FUNC(x) x
        #define MIDAS_CALL
    #else
        #ifdef __DOS__
            #define _FUNC(x) x cdecl
            #define MIDAS_CALL __cdecl
        #else
            #define _FUNC(x) x __stdcall
            #define MIDAS_CALL __cdecl
        #endif
    #endif
#endif


#if defined(__WATCOMC__) && defined(__cplusplus)
/* Disable to annoying Watcom C++ warnings - I have no idea how to get around
   these without breaking Visual C compatibility: */
#pragma warning 604 9
#pragma warning 594 9
#endif


/* We'll need to define DWORD, BOOL, TRUE and FALSE if someone hasn't
   done that before. For now, we'll just assume that if no-one has defined
   TRUE we need to define everything. There definitions are compatible with
   windows.h. If something else in your system defines these differently,
   things should still work OK as long as FALSE is 0, TRUE is nonzero and
   DWORD is 32-bit. Take care that you don't compare BOOLs like "bool == TRUE"
   in that case though, just use "bool".

   THIS IS UGLY AND MAY NEED FIXING!
   ---------------------------------
*/

#ifndef TRUE
#define TRUE 1
#define FALSE 0
typedef int BOOL;
typedef unsigned long DWORD;
#endif /* ifndef TRUE */



enum MIDASoptions
{
    MIDAS_OPTION_NONE = 0,
    MIDAS_OPTION_MIXRATE,
    MIDAS_OPTION_OUTPUTMODE,
    MIDAS_OPTION_MIXBUFLEN,
    MIDAS_OPTION_MIXBUFBLOCKS,
    MIDAS_OPTION_DSOUND_MODE,
    MIDAS_OPTION_DSOUND_HWND,
    MIDAS_OPTION_DSOUND_OBJECT,
    MIDAS_OPTION_DSOUND_BUFLEN,
    MIDAS_OPTION_16BIT_ULAW_AUTOCONVERT,
    MIDAS_OPTION_FILTER_MODE,
    MIDAS_OPTION_MIXING_MODE,
    MIDAS_OPTION_DEFAULT_STEREO_SEPARATION,
    MIDAS_OPTION_FORCE_NO_SOUND
};


enum MIDASmodes
{
    MIDAS_MODE_NONE = 0,
    MIDAS_MODE_MONO = 1,
    MIDAS_MODE_STEREO = 2,
    MIDAS_MODE_8BIT = 4,
    MIDAS_MODE_16BIT = 8,
    MIDAS_MODE_8BIT_MONO = MIDAS_MODE_8BIT | MIDAS_MODE_MONO,
    MIDAS_MODE_8BIT_STEREO = MIDAS_MODE_8BIT | MIDAS_MODE_STEREO,
    MIDAS_MODE_16BIT_MONO = MIDAS_MODE_16BIT | MIDAS_MODE_MONO,
    MIDAS_MODE_16BIT_STEREO = MIDAS_MODE_16BIT | MIDAS_MODE_STEREO
};



enum MIDASsampleTypes
{
    MIDAS_SAMPLE_NONE = 0,
    MIDAS_SAMPLE_8BIT_MONO = 1,
    MIDAS_SAMPLE_16BIT_MONO = 2,
    MIDAS_SAMPLE_8BIT_STEREO = 3,
    MIDAS_SAMPLE_16BIT_STEREO = 4,
    MIDAS_SAMPLE_ADPCM_MONO = 5,
    MIDAS_SAMPLE_ADPCM_STEREO = 6,
    MIDAS_SAMPLE_ULAW_MONO = 7,
    MIDAS_SAMPLE_ULAW_STEREO = 8
};



enum MIDASloop
{
    MIDAS_LOOP_NO = 0,
    MIDAS_LOOP_YES
};


enum MIDASpanning
{
    MIDAS_PAN_LEFT = -64,
    MIDAS_PAN_MIDDLE = 0,
    MIDAS_PAN_RIGHT = 64,
    MIDAS_PAN_SURROUND = 0x80
};


enum MIDASchannels
{
    MIDAS_CHANNEL_AUTO = 0xFFFF,
    MIDAS_ILLEGAL_CHANNEL = 0xFFFF
};


enum MIDASdsoundModes
{
    MIDAS_DSOUND_DISABLED = 0,
    MIDAS_DSOUND_STREAM,
    MIDAS_DSOUND_PRIMARY,
    MIDAS_DSOUND_FORCE_STREAM
};


enum MIDASpositions
{
    MIDAS_POSITION_DEFAULT = 0xFFFFFFFF
};



enum MIDASfilterModes
{
    MIDAS_FILTER_NONE = 0,
    MIDAS_FILTER_LESS = 1,
    MIDAS_FILTER_MORE = 2,
    MIDAS_FILTER_AUTO = 3
};


enum MIDASechoFilterTypes
{
    MIDAS_ECHO_FILTER_NONE = 0,
    MIDAS_ECHO_FILTER_LOWPASS = 1,
    MIDAS_ECHO_FILTER_HIGHPASS = 2
};


enum MIDASmixingModes
{
    MIDAS_MIX_NORMAL_QUALITY = 0,
    MIDAS_MIX_HIGH_QUALITY = 1
};


enum MIDASsamplePlayStatus
{
    MIDAS_SAMPLE_STOPPED = 0,
    MIDAS_SAMPLE_PLAYING = 1
};


enum MIDASpostProcPosition
{
    MIDAS_POST_PROC_FIRST = 0,
    MIDAS_POST_PROC_LAST
};



typedef struct
{
    char        songName[32];
    unsigned    songLength;
    unsigned    numPatterns;
    unsigned    numInstruments;
    unsigned    numChannels;
} MIDASmoduleInfo;



typedef struct
{
    char        instName[32];
} MIDASinstrumentInfo;



typedef struct
{
    unsigned    position;
    unsigned    pattern;
    unsigned    row;
    int         syncInfo;
    unsigned    songLoopCount;
} MIDASplayStatus;


typedef struct
{
    unsigned    delay;                  /* milliseconds, 16.16 fixed point */
    int         gain;                   /* gain, 16.16 fixed point */
    int         reverseChannels;        /* reverse channels? */
    unsigned    filterType;             /* filter type, MIDASechoFilterTypes */
} MIDASecho;


typedef struct
{
    int         feedback;               /* feedback, 16.16 fixed point */
    int         gain;                   /* total gain, 16.16 fixed point */
    unsigned    numEchoes;              /* number of echoes */
    MIDASecho   *echoes;                /* the echoes */
} MIDASechoSet;


typedef void (MIDAS_CALL *MIDASpostProcFunction)(void *data,
    unsigned numSamples, void *user);

typedef struct _MIDASpostProcessor
{
    struct _MIDASpostProcessor *next, *prev; /* reserved */
    void *userData;                          /* reserved */
    MIDASpostProcFunction floatMono;
    MIDASpostProcFunction floatStereo;
    MIDASpostProcFunction intMono;
    MIDASpostProcFunction intStereo;
} MIDASpostProcessor;



typedef void* MIDASmodule;
typedef DWORD MIDASmodulePlayHandle;
typedef DWORD MIDASsample;
typedef DWORD MIDASsamplePlayHandle;
typedef void* MIDASstreamHandle;
typedef void* MIDASechoHandle;


#ifdef __cplusplus
extern "C" {
#endif

_FUNC(int)      MIDASgetLastError(void);
_FUNC(char*)    MIDASgetErrorMessage(int errorCode);

_FUNC(DWORD)    MIDASgetDisplayRefreshRate(void);

_FUNC(BOOL)     MIDASstartup(void);
_FUNC(BOOL)     MIDASdetectSD(void);
_FUNC(BOOL)     MIDASdetectSoundCard(void);
_FUNC(BOOL)     MIDASconfig(void);
_FUNC(BOOL)     MIDASloadConfig(char *fileName);
_FUNC(BOOL)     MIDASsaveConfig(char *fileName);
_FUNC(BOOL)     MIDASreadConfigRegistry(DWORD key, char *subKey);
_FUNC(BOOL)     MIDASwriteConfigRegistry(DWORD key, char *subKey);

_FUNC(BOOL)     MIDASinit(void);
_FUNC(BOOL)     MIDASsetOption(int option, DWORD value);
_FUNC(DWORD)    MIDASgetOption(int option);
_FUNC(BOOL)     MIDASclose(void);
_FUNC(BOOL)     MIDASsuspend(void);
_FUNC(BOOL)     MIDASresume(void);
_FUNC(BOOL)     MIDASopenChannels(int numChannels);
_FUNC(BOOL)     MIDAScloseChannels(void);
_FUNC(BOOL)     MIDASsetAmplification(DWORD amplification);
_FUNC(BOOL)     MIDASstartBackgroundPlay(DWORD pollRate);
_FUNC(BOOL)     MIDASstopBackgroundPlay(void);
_FUNC(BOOL)     MIDASpoll(void);
_FUNC(void)     MIDASlock(void);
_FUNC(void)     MIDASunlock(void);
_FUNC(char*)    MIDASgetVersionString(void);
_FUNC(BOOL)     MIDASsetTimerCallbacks(DWORD rate, BOOL displaySync,
				       void (MIDAS_CALL *preVR)(),
				       void (MIDAS_CALL *immVR)(),
				       void (MIDAS_CALL *inVR)());
_FUNC(BOOL)     MIDASremoveTimerCallbacks(void);

_FUNC(DWORD)    MIDASallocateChannel(void);
_FUNC(BOOL)     MIDASfreeChannel(DWORD channel);

_FUNC(MIDASmodule) MIDASloadModule(char *fileName);
_FUNC(MIDASmodulePlayHandle) MIDASplayModule(MIDASmodule module,
                                             BOOL loopSong);
_FUNC(MIDASmodulePlayHandle) MIDASplayModuleSection(MIDASmodule module,
                                                    unsigned startPos,
                                                    unsigned endPos,
                                                    unsigned restartPos,
                                                    BOOL loopSong);
_FUNC(BOOL)     MIDASstopModule(MIDASmodulePlayHandle playHandle);
_FUNC(BOOL)     MIDASfreeModule(MIDASmodule module);

_FUNC(BOOL)     MIDASgetPlayStatus(MIDASmodulePlayHandle playHandle,
                                   MIDASplayStatus *status);
_FUNC(BOOL)     MIDASsetPosition(MIDASmodulePlayHandle playHandle,
                                 int newPosition);
_FUNC(BOOL)     MIDASsetMusicVolume(MIDASmodulePlayHandle playHandle,
                                    unsigned volume);
_FUNC(BOOL)     MIDASgetModuleInfo(MIDASmodule module, MIDASmoduleInfo *info);
_FUNC(BOOL)     MIDASgetInstrumentInfo(MIDASmodule module, int instNum,
                    MIDASinstrumentInfo *info);
_FUNC(BOOL)     MIDASsetMusicSyncCallback(MIDASmodulePlayHandle playHandle,
                                          void (MIDAS_CALL *callback)
                                          (unsigned syncInfo,
                                           unsigned position, unsigned row));
_FUNC(BOOL)     MIDASfadeMusicChannel(MIDASmodulePlayHandle playHandle,
                                      unsigned channel, unsigned fade);

_FUNC(MIDASsample) MIDASloadRawSample(char *fileName, int sampleType,
                    int loopSample);
_FUNC(MIDASsample) MIDASloadWaveSample(char *fileName, int loopSample);
_FUNC(BOOL)         MIDASfreeSample(MIDASsample sample);
_FUNC(BOOL)         MIDASallocAutoEffectChannels(unsigned numChannels);
_FUNC(BOOL)         MIDASfreeAutoEffectChannels(void);
_FUNC(MIDASsamplePlayHandle) MIDASplaySample(MIDASsample sample,
                        unsigned channel, int priority, unsigned rate,
                        unsigned volume, int panning);
_FUNC(BOOL)     MIDASstopSample(MIDASsamplePlayHandle sample);
_FUNC(BOOL)     MIDASsetSampleRate(MIDASsamplePlayHandle sample,
                    unsigned rate);
_FUNC(BOOL)     MIDASsetSampleVolume(MIDASsamplePlayHandle sample,
                    unsigned volume);
_FUNC(BOOL)     MIDASsetSamplePanning(MIDASsamplePlayHandle sample,
                    int panning);
_FUNC(BOOL)     MIDASsetSamplePriority(MIDASsamplePlayHandle sample,
                    int priority);
_FUNC(DWORD)    MIDASgetSamplePlayStatus(MIDASsamplePlayHandle sample);

_FUNC(MIDASstreamHandle) MIDASplayStreamFile(char *fileName,
                                             unsigned sampleType,
                                             unsigned sampleRate,
                                             unsigned bufferLength,
                                             int loopStream);
_FUNC(BOOL)     MIDASstopStream(MIDASstreamHandle stream);

_FUNC(MIDASstreamHandle) MIDASplayStreamWaveFile(char *fileName,
                                                 unsigned bufferLength,
                                                 int loopStream);

_FUNC(MIDASstreamHandle) MIDASplayStreamPolling(unsigned sampleType,
                                                unsigned sampleRate,
                                                unsigned bufferLength);
_FUNC(unsigned) MIDASfeedStreamData(MIDASstreamHandle stream,
                    unsigned char *data, unsigned numBytes, BOOL feedAll);

_FUNC(BOOL)     MIDASsetStreamRate(MIDASstreamHandle stream, unsigned rate);
_FUNC(BOOL)     MIDASsetStreamVolume(MIDASstreamHandle stream,
                    unsigned volume);
_FUNC(BOOL)     MIDASsetStreamPanning(MIDASstreamHandle stream, int panning);

_FUNC(DWORD)    MIDASgetStreamBytesBuffered(MIDASstreamHandle stream);
_FUNC(BOOL)     MIDASpauseStream(MIDASstreamHandle stream);
_FUNC(BOOL)     MIDASresumeStream(MIDASstreamHandle stream);

_FUNC(MIDASechoHandle) MIDASaddEchoEffect(MIDASechoSet *echoSet);
_FUNC(BOOL)     MIDASremoveEchoEffect(MIDASechoHandle echoHandle);

_FUNC(BOOL)     MIDASaddPostProcessor(MIDASpostProcessor *postProc,
                                      unsigned procPos, void *userData);
_FUNC(BOOL)     MIDASremovePostProcessor(MIDASpostProcessor *postProc);



#ifdef __cplusplus
}
#endif




#endif


/*
 * $Log: midasdll.h,v $
 * Revision 1.32  1997/08/15 16:48:25  pekangas
 * Added user post-processing functions
 *
 * Revision 1.31  1997/07/31 16:36:34  pekangas
 * Fixed to work in Linux again
 *
 * Revision 1.30  1997/07/31 14:30:58  pekangas
 * Added option MIDAS_OPTION_FORCE_NO_SOUND
 *
 * Revision 1.29  1997/07/31 10:56:48  pekangas
 * Renamed from MIDAS Sound System to MIDAS Digital Audio System
 *
 * Revision 1.28  1997/07/29 16:51:09  pekangas
 * Added sample playing status query
 *
 * Revision 1.27  1997/07/25 13:49:47  pekangas
 * Actually added MIDAS_FILTER_AUTO (oops)
 *
 * Revision 1.26  1997/07/25 13:48:24  pekangas
 * Changed MIDAS_OPTION_OVERSAMPLING setting to MIDAS_OPTION_MIXING_MODE
 * Added automatic filtering
 * Added MIDASgetOption()
 *
 * Revision 1.25  1997/07/11 11:05:54  pekangas
 * Added new options to the echoes: Each echo now has its own filter setting
 * (low/high-pass or nothing) and the whole echo set has a common gain.
 *
 * Revision 1.24  1997/07/10 18:40:23  pekangas
 * Added echo effect support
 *
 * Revision 1.23  1997/07/09 08:58:00  pekangas
 * Added default stereo separation
 *
 * Revision 1.22  1997/07/08 19:16:44  pekangas
 * Added Win32 setup functions, save/restore setup from registry, and
 * fixed WinWave to ignore buffer blocks -setting to be compatible with the
 * new setup.
 *
 * Revision 1.21  1997/06/05 20:18:49  pekangas
 * Added preliminary support for interpolating mixing (mono only at the
 * moment)
 *
 * Revision 1.20  1997/06/02 00:54:15  jpaana
 * Changed most __LINUX__ defines to __UNIX__ for generic *nix porting
 *
 * Revision 1.19  1997/05/21 17:47:41  pekangas
 * Changed MIDASfilterTypes to MIDASfilterModes
 *
 * Revision 1.18  1997/05/20 20:37:59  pekangas
 * Added WAVE support to both streams and samples
 *
 * Revision 1.17  1997/05/07 17:14:53  pekangas
 * Added a lot of new thread synchronization code, mainly to minimize the
 * cases where MIDAS state may be modified when the player thread is active
 * and vice versa. Added MIDASlock() and MIDASunlock() to the API.
 *
 * Revision 1.16  1997/05/03 17:54:02  pekangas
 * Added optional simple output filtering
 *
 * Revision 1.15  1997/05/02 13:19:49  pekangas
 * Several changes: Added support for non-looping module playback, added
 * support for playing several modules simultaneously, added module section
 * playback, added automatic channel allocation and deallocation to stream
 * playback, simplified automatic effect channel handling and added functions
 * for allocating and deallocating individiual channels.
 *
 * Revision 1.14  1997/04/08 15:48:07  pekangas
 * Added gmpFadeChannel / MIDASfadeMusicChannel functions
 *
 * Revision 1.13  1997/04/07 21:07:51  pekangas
 * Added the ability to pause/resume streams.
 * Added functions to query the number of stream bytes buffered
 *
 * Revision 1.12  1997/03/09 19:13:01  pekangas
 * Added the possibility to turn off u-law autoconvert
 *
 * Revision 1.11  1997/03/05 16:49:49  pekangas
 * Added timer functions to DLL, some other minor modifications
 *
 * Revision 1.10  1997/02/27 16:03:36  pekangas
 * Added DJGPP support
 *
 * Revision 1.9  1997/02/20 19:49:40  pekangas
 * Added u-law sample type
 *
 * Revision 1.8  1997/02/19 20:45:09  pekangas
 * Added functions MIDASsuspend() and MIDASresume()
 *
 * Revision 1.7  1997/02/12 17:18:37  pekangas
 * Added MIDASsetAmplification()
 *
 * Revision 1.6  1997/02/12 16:28:12  pekangas
 * Added ADPCM sample type
 *
 * Revision 1.5  1997/02/06 20:58:20  pekangas
 * Added DirectSound support - new files, errors, and global flags
 *
 * Revision 1.4  1997/01/16 18:41:59  pekangas
 * Changed copyright messages to Housemarque
 *
 * Revision 1.3  1997/01/16 18:26:27  pekangas
 * Added numerous new functions
 *
 * Revision 1.2  1996/09/28 08:12:40  jpaana
 * Fixed for Linux
 *
 * Revision 1.1  1996/09/25 18:38:12  pekangas
 * Initial revision
 *
*/
