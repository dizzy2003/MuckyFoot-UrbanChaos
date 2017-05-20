// BreakTimer.h
//
// break timer

// Interface:
//
// Simply call BreakTime(name) at each breakpoint and BreakFrame() after the screen flip call
// Call BreakStart() and BreakEnd() at start/end of game replay code
//
// The library reports on the times between frames and breakpoints
// by storing peak times (min and max) and average time between points

//#define	BREAKTIMER

#ifdef BREAKTIMER

extern void BreakStart();
extern void BreakEnd(const char* filename);
extern void BreakTime(const char* name);
extern void BreakFacets(ULONG facets);
extern void BreakFrame();

#else

#define BreakStart() 0
#define BreakEnd(X) 0
#define BreakTime(X) 0
#define BreakFacets(N) 0
#define BreakFrame() 0

#endif

extern void StartStopwatch();
extern float StopStopwatch();
