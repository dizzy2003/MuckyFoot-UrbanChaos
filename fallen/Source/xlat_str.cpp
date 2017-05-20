//
// xlat_str.cpp
// matthew rosenfeld 1 july 99
//
// translates strings to furrigan languages and remap buttons for the user's settings
//


//#define JAPANESE


#include "xlat_str.h"
#ifdef PSX
#define _MAX_PATH 256
#define	FILE_OPEN_ERROR	(-1)
#define FILE_READ_ERROR (-1)
#include "psxeng.h"
extern void	ZeroMemory(void *mem_ptr,ULONG size);

#endif

#ifdef TARGET_DC
#include "target.h"
#endif


//----------------------------------------------------------------------------
// DEFINES
//

#define REMAP_OFFSET (230)

//----------------------------------------------------------------------------
// GLOBALS
//

// translated text gets stuffed here
CBYTE  xlat_buffer[_MAX_PATH + 100];

// current translation set is loaded into here
CBYTE  xlat_set[MAX_STRING_SPACE];

// pointer table
CBYTE* xlat_ptr[MAX_STRINGS];

CBYTE* xlat_upto=0;

//----------------------------------------------------------------------------
// MBCS TWEAKS
//

UBYTE	previous_byte;

#ifdef JAPANESE
#define LEADBYTE(c)		( (previous_byte=c)>>7 )
#define TAILBYTE        ( LEADBYTE(previous_byte) )
#else
#define LEADBYTE(c)		( 0 )
#define TAILBYTE		( 0 )
#endif

// so it knows it's a new string...
inline void		mbcs_reset() {
	previous_byte = 0;
}

// inc char (_not_ byte) in a MBCS string
inline CBYTE*	mbcs_inc_char(CBYTE* &c) {
#ifdef JAPANESE
	c+=((*c)&128)?2:1;
#else
	c++;
#endif
	return c;
}




#ifdef TARGET_DC
// Life is too short for reading single bytes over the SCSI bus...

// Overload FileRead()
#define MYFILEREAD_CACHE_SIZE 256
char pcCharCache[MYFILEREAD_CACHE_SIZE];
int iCharCacheLen = 0;
int iCharCacheSize = 0;
MFFileHandle cached_file_handle = (void *)0xdeadbeef;
SLONG CachedFileRead(MFFileHandle file_handle,char *buffer)
{
	ASSERT ( file_handle != (void *)0xdeadbeef );

	if ( cached_file_handle != file_handle )
	{
		// New file - bin the cache.
		cached_file_handle = file_handle;
		iCharCacheSize = 0;
	}
	if ( iCharCacheSize == FILE_READ_ERROR )
	{
		// No good - no more characters.
		return ( FILE_READ_ERROR );
	}
	else if ( iCharCacheSize <= 0 )
	{
		// Read a chunk.
		iCharCacheSize = FileRead ( file_handle, pcCharCache, MYFILEREAD_CACHE_SIZE );
		iCharCacheLen = iCharCacheSize;
	}
	if ( iCharCacheSize == 0 )
	{
		// No good - no more characters.
		iCharCacheSize = FILE_READ_ERROR;
		return ( FILE_READ_ERROR );
	}
	*buffer = pcCharCache[iCharCacheLen - iCharCacheSize];
	iCharCacheSize--;
	return ( 1 );
}

// Call it when you open a new file!
void CacheFileReadFlush ( void )
{
	cached_file_handle = (void *)0xdeadbeef;
}
#endif




// safely finds previous character (_not_ byte) in a MBCS strings

/* this method works but is obviously slow

inline CBYTE*	mbcs_dec_char(CBYTE* &c, CBYTE* base) {
#ifdef JAPANESE
	CBYTE* last=base;
	
	while (base<c)
	{
		last=base;
		mbcs_inc_char(base);
	}
	c=last;
#else
	c--;
	if (c<base) c=base;
#endif
	return c;
}

*/

// retrieves previous char without changing original ptr
// this method should be faster in most cases. worst-case, it's only as slow as the previous
// one.

inline CBYTE* mbcs_prev_char(CBYTE* c, CBYTE* base)
{
#ifndef JAPANESE
	return c-1;
#else
	char *temp;
	// c must point either to a single-byte character
	// or to the beginning of a double-byte character.
	// It cannot point to the middle of a double-byte character.
	// Bad! Bad!

	// If we are at the beginning, or the previous byte
	// is at the beginning, simply return psz.
	if (c <= base - 1)
		return base;

	temp = c - 1; // point to previous byte

	// *(c-1) _must_ be either a trail or single byte.
	// if it's lo-ascii, it could be either trail or
	// single byte, so we keep going. but if it's hi-
	// ascii, it _must_ be a trail byte (because it's
	// immediately before a lead/single byte) so we know
	// the byte before is the lead byte we're looking for
	// and we can happily return.

	if (LEADBYTE(*temp))
		return (temp - 1);

	// Otherwise, step back until a non-lead-byte is found.
	while ((base <= --temp) && LEADBYTE(*temp)) ;

	// Now temp+1 must point to the beginning of a character,
	// so figure out whether we went back an even or an odd
	// number of bytes and go back 1 or 2 bytes, respectively.
	return ((c - 1) - ((c - temp) & 1));
#endif
}


inline CBYTE*	mbcs_dec_char(CBYTE* &c, CBYTE* base) {
#ifndef JAPANESE
	return --c;
#else
	return (c=mbcs_prev_char(c,base)); 
#endif
}


// some standard library funcs in mbcs-eze.
// windows has some of these already, but the PSX doesn't, so...

CBYTE*			mbcs_strchr(CBYTE *str, CBYTE c) {
	CBYTE *scan=str, *end=str+strlen(str);
	while (scan<end) {
		if (*scan==c) return scan;
		mbcs_inc_char(scan);
	}
	return 0;
}

void			mbcs_strncpy(CBYTE *dst, CBYTE *src, CBYTE n) {
	CBYTE *scan=src, *end=src+strlen(src);
	while (n&&(scan<end)) 
	{
		if (LEADBYTE(*scan))
		{
			*dst=*scan; dst++; scan++;
		}
		*dst=*scan; dst++; scan++;
		n--;
	}
}

CBYTE* mbcs_strspnp(CBYTE *str, CBYTE *badlist) {
	while (*str&&mbcs_strchr(badlist,*str)) mbcs_inc_char(str);
	return str;
}

//----------------------------------------------------------------------------
// FUNCS
//

CBYTE* XLAT_remap(UBYTE remap_id) {
	return xlat_ptr[REMAP_OFFSET+remap_id];
}

CBYTE* XLAT_str_ptr(SLONG string_id) {
	CBYTE *xlated=xlat_ptr[string_id];
	if ((xlat_upto==xlat_set)||(!xlat_upto)||!xlated) {
		#ifdef TARGET_DC
		ASSERT ( FALSE );
		#endif
	  return "missing language file. get t:\\lang-english.txt and stick it in your fallen\\text directory";
	}
	return xlated;
}

CBYTE* XLAT_str(SLONG string_id, CBYTE *xlat_dest) {
	CBYTE *xlated=xlat_ptr[string_id];
	CBYTE *ptr, *ptr2, *buff;
	UWORD ofs;

	if (!xlat_dest) xlat_dest=xlat_buffer;

	buff=xlat_dest;

	if ((xlat_upto==xlat_set)||(!xlat_upto)||!xlated) {
		ASSERT ( FALSE );
	  return "missing language file. get t:\\lang-english.txt and stick it in your fallen\\text directory";
	}

	ZeroMemory(xlat_buffer,_MAX_PATH+100);

	while (ptr=mbcs_strchr(xlated,1)) {
		ofs=ptr-xlated;
		if (ofs) {
			strncpy(xlat_dest,xlated,ofs); // not MBCSed because ofs is in bytes not chars (mbcs_strchr)
			xlat_dest+=ofs;
		}
		ptr++;
		ptr2=XLAT_remap(*ptr);
		strcpy(xlat_dest,ptr2); // doesn't need MBCSicizing - a zero is always a zero, as it were.
		xlat_dest+=strlen(ptr2);
		xlated=ptr+1;
	}

	strcpy(xlat_dest,xlated);

	return buff;
}



CBYTE* XLAT_load_string(MFFileHandle &file, CBYTE *txt) {
	CBYTE *ptr=txt, *temp;
	UWORD emergency_bail_out_for_martins_machine=2000;
	UBYTE leadbyte=1;

	mbcs_reset();

	*ptr=0;
	while (emergency_bail_out_for_martins_machine--) {
#ifdef TARGET_DC
		if (CachedFileRead(file,ptr)==FILE_READ_ERROR)
#else
		if (FileRead(file,ptr,1)==FILE_READ_ERROR)
#endif
		{
			*ptr=0; return txt;
		};
		if ((*ptr==10)&&!TAILBYTE) break;
		ptr++;
	}

//	if ((ptr!=txt)&&(*(ptr-1)==13)) ptr--;

	temp=mbcs_prev_char(ptr,txt);
	if (*temp==13) 
		*temp=0;
	else
		*ptr=0;
	//if (!emergency_bail_out_for_martins_machine) MessageBox(0,"it's fucking fucked","piss.",MB_OK);
	return txt;
}

void XLAT_compress_tokens(CBYTE *txt) {
	CBYTE *test, *scan, *ptr=txt;
	CBYTE buff[10];
	UWORD skip;
	while (scan=mbcs_strchr(txt,'|')) {
		test=scan;
		test=mbcs_strspnp(mbcs_inc_char(test),"0123456789");
		if (test) {
			skip=test-scan;
			strncpy(buff,scan+1,skip-1); *(buff+skip-1)=0;
			*scan=1; scan++;
			*scan=atoi(buff);
			strcpy(scan+1,test);
			txt=scan+1;
		} else {
			*scan=1; scan++;
			*scan=atoi(scan+1);
			*(scan+1)=0;
		}
		txt++;
	}
}

void XLAT_load(CBYTE *fn) {
	MFFileHandle handle;
	CBYTE *txt;
	UWORD id=0;
	UWORD emergency_bail_out_for_martins_machine=2000;

#ifdef JAPANESE
	fn="text\\lang_japanese.txt";
#endif

	xlat_upto=xlat_set;
	ZeroMemory(xlat_ptr,sizeof(xlat_ptr));
	ZeroMemory(xlat_set,sizeof(xlat_set));

#ifndef PSX
	if (!FileExists(fn))
	{
		ASSERT ( FALSE );
		return;
	}
#endif

	handle=FileOpen(fn);
#ifdef TARGET_DC
	CacheFileReadFlush();
#endif

	do {
		txt=XLAT_load_string(handle,xlat_upto);
		if (*txt) {
			XLAT_compress_tokens(xlat_upto);
			xlat_ptr[id++]=xlat_upto;
			xlat_upto+=strlen(xlat_upto)+1;
		}
	} while (*txt&&emergency_bail_out_for_martins_machine--);

	//if (!emergency_bail_out_for_martins_machine) MessageBox(0,"It timed out for some reason","erg.",MB_OK);
	
	FileClose(handle);
	TRACE("\n");

	TRACE("Language data: %s\n",XLAT_str_ptr(X_THIS_LANGUAGE_IS));
}

void XLAT_poke(SLONG offset, CBYTE *str) {
  strcpy(xlat_upto,str);
  xlat_ptr[offset]=xlat_upto;
  xlat_upto+=strlen(str)+1;
}

void XLAT_init() {
	// sticks some useful default stuff in
#ifndef PSX
	ASSERT(xlat_upto);
	XLAT_poke(REMAP_OFFSET+XCTL_JUMP,"space");
	XLAT_poke(REMAP_OFFSET+XCTL_PUNCH,"Z");
	XLAT_poke(REMAP_OFFSET+XCTL_KICK,"X");
	XLAT_poke(REMAP_OFFSET+XCTL_ACTION,"C");
	XLAT_poke(REMAP_OFFSET+XCTL_LEFT,"left");
	XLAT_poke(REMAP_OFFSET+XCTL_RIGHT,"right");
	XLAT_poke(REMAP_OFFSET+XCTL_START,"start");
	XLAT_poke(REMAP_OFFSET+XCTL_SELECT,"select");
	XLAT_poke(REMAP_OFFSET+XCTL_SPACE,"space");
	XLAT_poke(REMAP_OFFSET+XCTL_ENTER,"enter");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_FIRST,"A");
	XLAT_poke(REMAP_OFFSET+XCTL_RUN,"V");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_HIGH,"shoulder");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_LOW,"shoulder");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_ESC,"esc");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_CONTINUE,"space");
#else
	ASSERT(xlat_upto);
	XLAT_poke(REMAP_OFFSET+XCTL_JUMP,STR_CROSS);
	XLAT_poke(REMAP_OFFSET+XCTL_PUNCH,STR_SQUARE);
	XLAT_poke(REMAP_OFFSET+XCTL_KICK,STR_TRI);
	XLAT_poke(REMAP_OFFSET+XCTL_ACTION,STR_CIRCLE);
	XLAT_poke(REMAP_OFFSET+XCTL_LEFT,"left");
	XLAT_poke(REMAP_OFFSET+XCTL_RIGHT,"right");
	XLAT_poke(REMAP_OFFSET+XCTL_START,"start");
	XLAT_poke(REMAP_OFFSET+XCTL_SELECT,"select");
	XLAT_poke(REMAP_OFFSET+XCTL_SPACE,STR_CROSS);
	XLAT_poke(REMAP_OFFSET+XCTL_ENTER,"start");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_FIRST,"L1");
	XLAT_poke(REMAP_OFFSET+XCTL_RUN,"R1");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_HIGH,"R2");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_LOW,"L2");
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_ESC,STR_TRI);
	XLAT_poke(REMAP_OFFSET+XCTL_CAM_CONTINUE,STR_CROSS);
#endif

}
