
// THIS IS PANTS!

#if 0

/************************************************************
 *
 *   font3d.h
 *   3D text writer
 *
 */

#ifndef _FONT_3D_H_
#define _FONT_3D_H_

// #include <MFStdLib.h>

// why does the above line toast it?
// oh well... set up manually then...

#define	TRUE				1
#define	FALSE				0
#define PI				(3.14159265F)
typedef	unsigned char		UBYTE;
typedef signed char			SBYTE;
typedef char				CBYTE;
typedef unsigned short		UWORD;
typedef signed short		SWORD;
typedef unsigned long		ULONG;
typedef signed long			SLONG;


struct FontVec {
	float x,y,z;       // vertex pos
	float tx,ty,tz;    // translated vertex pos
	float nx,ny,nz;    // normal pos
	float tnx,tny,tnz; // translated normal pos
};

struct FontFace {
	FontVec *a, *b, *c, norm;
};

struct FontData {
	ULONG     numpts, numfaces;
    FontVec  *pts;
	FontFace *faces;
	UWORD     width;
};

class Font3D {
  private:
    FontData data[100];
	ULONG    nextchar;
	ULONG	 LetterWidth(CBYTE chr);
	float	 fontscale;
  public:
    void ClearLetters();
	void AddLetter(CBYTE *fn);
	void DrawLetter(CBYTE chr, ULONG x, ULONG y, ULONG rgb=0xffffff, float yaw=0, float roll=0, float pitch=0, float scale=3.5);
	void DrawString(CBYTE *str, ULONG x, ULONG y, ULONG rgb=0xffffff, float scale=3.5, CBYTE wibble=0, UWORD zoom=0);
	Font3D(char *path, float scale=1.0);
	~Font3D();
};

#endif

#endif