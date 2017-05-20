/************************************************************
 *
 *   flamengine.h
 *   2D flame (and other fx) engine
 *
 */

#ifndef _FLAMENGINE_
#define _FLAMENGINE_

#include <MFStdLib.h>

struct FlameXY2 {
	UBYTE         x,y;
};

union FlameXY {
	FlameXY2      loc;
	UWORD         ofs;
};

#ifdef TARGET_DC
// Properly aligned!
struct FlameParticle {
	FlameXY       pos;
	UBYTE         jx,jy;    // Jitter range
	UBYTE		  ex,ey;	// Emitter position
	UBYTE		  life;     // could be ttl but is just on/off flag
	UBYTE         prate;    // speed of pulsing
	UBYTE         pmode;    // kind of pulse (ramp/cycle)
	UBYTE         wmode;    // walk mode
	SWORD         pulse;    // particles pulsing in and out
	ULONG         pstart;   // start of pulse range;
	ULONG         pend;     // end of pulse range;
};
#else
struct FlameParticle {
	FlameXY       pos;
	UBYTE         jx,jy;    // Jitter range
	UBYTE		  ex,ey;	// Emitter position
	UBYTE		  life;     // could be ttl but is just on/off flag
	SWORD         pulse;    // particles pulsing in and out
	UBYTE         prate;    // speed of pulsing
	UBYTE         pmode;    // kind of pulse (ramp/cycle)
	UBYTE         wmode;    // walk mode
	ULONG         pstart;   // start of pulse range;
	ULONG         pend;     // end of pulse range;
};
#endif

struct FlameParams {
	UBYTE         blur, dark, convec;
	UBYTE         palette[768];
	UWORD         free, posn;
	UBYTE		  randomize;// extra random stuff for flames
	FlameParticle particles[2000];
};

class Flamengine {
  private:
	UBYTE		 data[256*256*2];
	UBYTE		 work[256*256*2];
	void AddParticles();	//   Sparkly bits
	void AddParticles2();	//   Sparkly bits 
	void Feedback();        //   Funky feedback; must be used BEFORE any other screen draw
	void ConvectionBlur();  //   Straightforward blur box filter but offset one line
	void ConvectionBlur2(); //   H@X0R'd version
	void Darkening();		//   Moves and/or darkens the image
	void UpdateTexture();   //   Locks, pokes, unlocks and updates a texture with the fire data
	void ReadHeader(MFFileHandle handle);
	void ReadParts(MFFileHandle handle);
	void BlitOffset();		//   Blit the texture, skewed for feedback, onto the backbuffer
  public:
	FlameParams  params;
	Flamengine(char *name);
	~Flamengine();
	void Run();				//	 Update the animation one frame
	void Blit();			//   Blit the texture, suitably alpha'd, onto the backbuffer
	void BlitHalf(CBYTE side);	//   Same, but half only (for seperate left + right)
};

#endif