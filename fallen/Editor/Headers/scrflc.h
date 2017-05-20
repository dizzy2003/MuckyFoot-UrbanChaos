#ifndef	_SCRFLC_H_
#define	_SCRFLC_H_

//**************************************|************************************

struct	FLCFileHeader
{
	ULONG								Size;
	UWORD								Magic;
	UWORD								NumberOfFrames;
	UWORD								Width;
	UWORD								Height;
	UWORD								Depth;
	UWORD								Flags;
	ULONG								Speed;
	UWORD								Reserved_0;
	ULONG								Created;
	ULONG								Creator;
	ULONG								Updated;
	ULONG								Updater;
	UWORD								AspectX;
	UWORD								AspectY;
	UBYTE								Reserved_1[38];
	ULONG								OFrame1;
	ULONG								OFrame2;
	UBYTE								Reserved_2[40];
};

//**************************************|************************************

struct	FLCPrefixChunk
{
	ULONG								Size;
	UWORD								Type;
};

//**************************************|************************************

struct	FLCFrameChunk
{
	ULONG								Size;
	UWORD								Type;
	UWORD								Chunks;
	UBYTE								Reserved_0[8];
};

//**************************************|************************************

struct	FLCFrameDataChunk
{
	ULONG								Size;
	UWORD								Type;
};

//**************************************|************************************

struct	FLCPostageStamp
{
	ULONG								Size;
	UWORD								Type;
	UWORD								Height;
	UWORD								Width;
	UWORD								XLate;
};

//**************************************|************************************

union	MultiPointer
{
	UBYTE								*UByte;
	UWORD								*UWord;
	ULONG								*ULong;
	SBYTE								*SByte;
	SWORD								*SWord;
	SLONG								*SLong;
	struct	FLCFileHeader				*FLCFileHeader;
	struct	FLCPrefixChunk				*FLCPrefixChunk;
	struct	FLCFrameChunk				*FLCFrameChunk;
	struct	FLCFrameDataChunk			*FLCFrameDataChunk;
	struct	FLCPostageStamp				*FLCPostageStamp;
};

//**************************************|************************************

struct	Animation
{
	SLONG								PlaybackMode;
	UBYTE								*LastFrame;
	UBYTE								*NextFrameBuffer;
	union	MultiPointer				NextFrameBufferPointer;
	MFFileHandle						PlayFileHandle;
	MFFileHandle						RecordFileHandle;
	SWORD								Xpos;
	SWORD								Ypos;
	UBYTE								Palette[256 * 3];
	SLONG								FrameNumber;
	SLONG								FrameSizeMaximum;
	SLONG								Active;
	struct	FLCFileHeader				FLCFileHeader;
	struct	FLCPrefixChunk				FLCPrefixChunk;
	struct	FLCFrameChunk				FLCFrameChunk;
	struct	FLCFrameDataChunk			FLCFrameDataChunk;
	struct	FLCPostageStamp				FLCPostageStamp;
};

//**************************************|************************************

#define		FLI_COLOUR256				4
#define		FLI_SS2						7
#define		FLI_COLOUR					11
#define		FLI_LC						12
#define		FLI_BLACK					13
#define		FLI_BRUN					15
#define		FLI_COPY					16
#define		FLI_PSTAMP					18

//**************************************|************************************

#define		PLAYBACK_MODE_STOPPED		0
#define		PLAYBACK_MODE_RECORD		(1 << 0)
#define		PLAYBACK_MODE_PLAY			(1 << 1)

//**************************************|************************************
extern	SLONG	anim_stop(void);
extern	SLONG	anim_record();

extern	SLONG	anim_open(SBYTE *file_name, SWORD xpos, SWORD ypos, SWORD width, SWORD height, SBYTE *postage_stamp, SLONG	playback );
extern	SLONG	anim_close(SLONG playback);
extern	SLONG	anim_make_next_frame(UBYTE *WScreen, UBYTE *palette );
extern	SLONG	anim_make_FLI_PSTAMP();
extern	SLONG	anim_make_FLI_COLOUR256(UBYTE *palette);
extern	SLONG	anim_make_FLI_COLOUR(UBYTE *palette);
extern	SLONG	anim_make_FLI_SS2(UBYTE *wscreen, UBYTE *last_screen);
extern	SLONG	anim_make_FLI_LC(UBYTE *wscreen, UBYTE *last_screen);
extern	SLONG	anim_make_FLI_BLACK(UBYTE *wscreen );
extern	SLONG	anim_make_FLI_BRUN(UBYTE *wscreen );
extern	SLONG	anim_make_FLI_COPY(UBYTE *wscreen );
extern	SLONG	anim_write_data(UBYTE *data, SLONG size);
extern	SLONG	anim_store_data(UBYTE *data, SLONG size);
extern	SLONG	anim_show_next_frame();
extern	SLONG	anim_show_FLI_PSTAMP();
extern	SLONG	anim_show_FLI_COLOUR256();
extern	SLONG	anim_show_FLI_COLOUR();
extern	SLONG	anim_show_FLI_SS2();
extern	SLONG	anim_show_FLI_LC();
extern	SLONG	anim_show_FLI_BLACK();
extern	SLONG	anim_show_FLI_BRUN();
extern	SLONG	anim_show_FLI_COPY();
extern	SLONG	anim_read_data(UBYTE *data, SLONG size);

//**************************************|************************************
#endif
