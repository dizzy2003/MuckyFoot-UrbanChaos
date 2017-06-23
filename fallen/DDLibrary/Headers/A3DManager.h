// A3DManager.h
// 2nd October 1998
//
// (experimental) Aureal 3D Manager library
//
// requires either Aureal 3D sound card + drivers, OR normal sound card with A2D installed


#ifndef _A3D_MANAGER_H_
#define _A3D_MANAGER_H_





#ifndef	MF_STD_LIB_H

typedef	unsigned char		UBYTE;
typedef signed char			SBYTE;
typedef char				CBYTE;
typedef unsigned short		UWORD;
typedef signed short		SWORD;
typedef unsigned long		ULONG;
typedef signed long			SLONG;

void            TraceText(CBYTE *error, ...);
#define TRACE				TraceText

#endif

#ifndef _A3DPLAY_H_
//#define INITGUID
#endif

#include <objbase.h>
#include <stdlib.h>
#include <cguid.h>

#include "fallen/headers/A3D/Ia3dapi.h"


#define MAX_QUEUE_LENGTH	5

// prototypes

class A3DManager;
class A3DSource;
class A3DData;
class A3DBase;
class A3DList;

// globals

extern IA3d4		*a3droot;
extern A3DManager	the_a3d_manager;

// functions

void A3D_Check_Init(void);

// classes

//
//	A3DList -- a linked list thingy
//

class A3DList {
private:
	A3DBase *list, *tail;
	SLONG	 count;
public:
	A3DList()									{ count=0; list=tail=NULL; }
	~A3DList();
	void Add(A3DBase *item);
	void Del(A3DBase *item);
	void Clear();
	A3DBase *Index(SLONG index);
	A3DBase *Find(CBYTE *want);
	inline SLONG Count()						{ return count; }
	inline A3DBase *Head()						{ return list; }
	inline A3DBase *Tail()						{ return tail; }
	inline operator+=(A3DBase *item)			{ Add(item); return ( NULL ); };
	inline operator-=(A3DBase *item)			{ Del(item); return ( NULL ); };
	inline A3DBase* operator[](SLONG index)		{ return Index(index);	};
};


//
//	A3DManager -- At The Heart Of it All
//

#define A3D_MAT_COUNT		3
#define A3D_MAT_CARPET		0
#define A3D_MAT_STEEL		1
#define A3D_MAT_SNDPROOF	2

class A3DManager {
private:
	IA3dListener *a3dlis;
	IA3dMaterial *mat_lib[A3D_MAT_COUNT];

public:
	A3DList		  srclist,datalist;

	// construct, destruct
	A3DManager(SLONG features=0);
	~A3DManager();
	void Init(SLONG features);
	void Fini();

	// listener
	inline void ListenPos(float x, float y, float z)			{	if (a3dlis) a3dlis->SetPosition3f(x,y,z);				};
	inline void ListenRot(float head, float roll, float pitch)	{	if (a3dlis) a3dlis->SetOrientationAngles3f(head,pitch,roll);	};

	// channel play
	A3DSource* Play(A3DData *Original, A3DSource* Channel=NULL, UBYTE Looped=0);

	// channel check
	BOOL Valid(A3DBase* item);
	A3DSource *ValidChannel(A3DBase* item);
	A3DBase	  *ValidWave(A3DBase* item);

	// geom related
	void BindMaterial(SLONG material);

	// misc
	inline void Render()										{ 	a3droot->Flush(); a3droot->Clear();						};
//	inline void Render()										{ 	a3droot->Flush();										};
//	inline void Render()										{ 	a3droot->Clear();										};
//	inline void BeginFrame()									{	a3droot->Clear();										};

};


//
//	A3DBase -- the root of all evil
//

class A3DBase {
protected:
	IA3dSource	*a3dsrc;
	CBYTE		 title[_MAX_PATH];
	float		 length_seconds;
	ULONG		 length_samples;
	ULONG		 last_time;
public:
	// blah blah
	CBYTE		owner, type;
	// linked list pointers
	A3DBase  	*last, *next;

	A3DBase()													{	last=next=NULL; a3dsrc=NULL; title[0]=0; length_samples=0; length_seconds=0; last_time=0; };
	virtual ~A3DBase()											{	/* why can't it be pure virtual? */			};

	// data
	void FreeWave();

	// queries
	inline IA3dSource	*GetSource()							{	return a3dsrc;											};
	inline CBYTE		*GetTitle()								{	return title;											};
	ULONG				 GetLengthSamples();
	float				 GetLengthSeconds();
	BOOL				 HasEnded(UBYTE early_out);
//	BOOL				 HasEnded() 							{   float f; a3dsrc->GetWaveTime(&f); return (f>GetLengthSeconds()); };


};

//
//	A3DData -- stores data, nothing else
//

class A3DData : public A3DBase {
public:
	// construct, destruct
	A3DData(CBYTE *fn=0, UBYTE ntype=A3DSOURCE_TYPEDEFAULT);	// constructor: load filename, or no data
	virtual ~A3DData();
};

//
//	A3DSource -- actually does something useful
//

struct QueueItem {
	A3DBase	*original;
	SLONG	 x,y,z;
	SLONG	 flags;
};

class A3DSource : public A3DBase {
private:
	ULONG		 rendermode;
	UBYTE		 queuepos;
	QueueItem	 queue[MAX_QUEUE_LENGTH];
	void DupeConstruct(A3DBase *original);		// duplicate existing source
	void DoChange(A3DBase *original);
	void SetupParams();
public:
	BOOL		autofree;
	ULONG		Flags;
	SLONG		User;							// random user-defined var
	A3DBase		*cloned_from;

	// construct, destruct
	A3DSource(CBYTE *fn=0);						// constructor: load filename, or no data
	A3DSource(A3DBase *original);				// constructor: duplicate existing source
	virtual ~A3DSource();

	// configuration
	inline void SetPriority(float priority)						{	if (a3dsrc) a3dsrc->SetPriority(priority);							};
	void SetMute(UBYTE mute);
	void Set3D(UBYTE is3d);
	inline void SetGain(UBYTE vol) 								{	if (a3dsrc) a3dsrc->SetGain(((float)vol)/255.0f);					};
	void Change(A3DBase *original);		// reset to a new source file (& flush queue)
	void Queue(A3DBase *original, SLONG x, SLONG y, SLONG z, SLONG flags);// queue for a new source at end of current
	inline void QueueFlush()									{   queuepos=0;															};

	// callback
	UBYTE CBEnded();


	// 3D
	inline void SetPositionf(float x, float y, float z)			{	if (a3dsrc) a3dsrc->SetPosition3f(x,y,z);							};
	inline void SetPositionl(SLONG x, SLONG y, SLONG z)			{	if (a3dsrc) a3dsrc->SetPosition3f((float)x,(float)y,(float)z);		};
	inline void SetRotationf(float h, float r, float p)			{	if (a3dsrc) a3dsrc->SetOrientationAngles3f(h,r,p);						};
	inline void SetRotationl(SLONG h, SLONG r, SLONG p)			{	if (a3dsrc) a3dsrc->SetOrientationAngles3f((float)h,(float)r,(float)p);	};
	inline void SetVelocityf(float x, float y, float z)			{	if (a3dsrc) a3dsrc->SetVelocity3f(x,y,z);							};
	inline void SetVelocityl(SLONG x, SLONG y, SLONG z)			{	if (a3dsrc) a3dsrc->SetVelocity3f((float)x,(float)y,(float)z);		};

	// audio properties
	inline void SetPitchf(float pitchbend)						{	if (a3dsrc) a3dsrc->SetPitch(pitchbend); };
	inline void SetGainf(float gain)							{   if (a3dsrc) a3dsrc->SetGain(gain);		 };

	// transport controls
	void Play(UBYTE looped);
	void Stop();
	void Rewind();
	inline void Pause()												{ 	if (a3dsrc) a3dsrc->Stop();								};

};



#endif
