// A3DManager.cpp
// 2nd October 1998
//
// (experimental) Aureal 3D Manager library
//
// requires either Aureal 3D sound card + drivers, OR normal sound card with A2D installed
//
// NOTE! RTTI (run-time type information) MUST BE ENABLED IN PROJECT SETTINGS!
//

#ifndef TARGET_DC


#include "A3DManager.h"
#include "snd_type.h"
//#include "A3DPlay.h"

extern volatile HWND	hDDLibWindow;


/**************************************************************************************
 *
 *		Globals
 *
 */

// Why is this global? Because sound sources need access to it too.
IA3d4		*a3droot;
IA3dGeom     *a3dgeom = NULL;
A3DManager	the_a3d_manager(A3D_1ST_REFLECTIONS | A3D_OCCLUSIONS | A3D_DIRECT_PATH_A3D);


void Decode(SLONG hr) {
	TRACE("A3D Error: ");
	switch(hr) {
	case A3DERROR_FAILED_FILE_OPEN:
		TRACE("Failed File Open"); break;

	case A3DERROR_FAILED_LOCK_BUFFER:
		TRACE("Failed Lock"); break;

	case A3DERROR_FAILED_UNLOCK_BUFFER:
		TRACE("Failed Unlock"); break;

	case A3DERROR_UNRECOGNIZED_FORMAT:
		TRACE("Unrecongized Format"); break;

	case A3DERROR_FAILED_ALLOCATE_WAVEDATA:
		TRACE("Failed Allocate Wavedata"); break;

	case E_INVALIDARG:
		TRACE("Invalid Argument"); break;

	case A3DERROR_MEMORY_ALLOCATION:
		TRACE("Memory Allocation Error"); break;

	case A3DERROR_FAILED_CREATE_PRIMARY_BUFFER:
		TRACE("Primary Buffer Create Failed"); break;

	case A3DERROR_NO_WAVE_DATA:
		TRACE("No Wave Data"); break;

	case A3DERROR_UNKNOWN_PLAYMODE:
		TRACE("Unknown Play Mode"); break;

	case A3DERROR_FAILED_PLAY:
		TRACE("Failed Play"); break;
	}

	TRACE("\n");
}

void ErrChk(SLONG hr) {
	if (hr==S_OK) return;
	Decode(hr);
}

BOOL Failed(SLONG hr) {
	if (hr==S_OK) FALSE;
	Decode(hr);
	return TRUE;
}

void A3D_Check_Init(void) {
  the_a3d_manager.Init(A3D_1ST_REFLECTIONS | A3D_OCCLUSIONS | A3D_DIRECT_PATH_A3D);
}


/* =============================================================
// RegDBSetKeyValue()	Set the registry database key value
//
// =============================================================*/

static void RegDBSetKeyValue(
	char *szKey,		/* in, key string */
	char *szName,		/* in, name string NULL == Default */
	char *szValue)		/* in, value string */
{
	DWORD dwOptions   = REG_OPTION_NON_VOLATILE;
	REGSAM samDesired =  KEY_ALL_ACCESS;
	HKEY hKey;
	DWORD dwDisposition;

	/* Create the key, even if it already existed. */

	RegCreateKeyExA(
		HKEY_CLASSES_ROOT,	/* handle of an open key  */
        szKey,				/* lpSubKey address of subkey name  */
		0,					/* reserved word */
		"REG_SZ",			/* address of class string */ 
		dwOptions,			/* special options flag */
		samDesired,			/* desired security access */
		NULL,				/* address of key security structure */
		&hKey,				/* address of buffer for opened handle  */
		&dwDisposition);	/* address of disposition value buffer */

	/* Just write to it. */
    RegSetValueExA( 
		  hKey,								/* handle of key to set value for  */
          szName,							/* address of value to set */
		  0,								/* reserved */
		  REG_SZ,							/* flag for value type */
		  (CONST BYTE *)szValue,			/* address of value data */
		  strlen(szValue)					/* length of value buffer */
		  ); 

	/* close the key. */
	RegCloseKey(hKey);
}


/* =============================================================
// A3dRegister()		Registers the COM objects for A3D.
//
// Returns:
//
//   S_OK		
//		
// =============================================================*/

HRESULT A3dRegister(void)
{
    // A3d COM Keys
    RegDBSetKeyValue("A3d",	"",	"A3d Object");
    RegDBSetKeyValue("A3d\\CLSID", "", "{d8f1eee0-f634-11cf-8700-00a0245d918b}");
    RegDBSetKeyValue("CLSID\\{d8f1eee0-f634-11cf-8700-00a0245d918b}", "",  "A3d Object");
	RegDBSetKeyValue("CLSID\\{d8f1eee0-f634-11cf-8700-00a0245d918b}\\InprocServer32", "", "a3d.dll");
	RegDBSetKeyValue("CLSID\\{d8f1eee0-f634-11cf-8700-00a0245d918b}\\InprocServer32", "ThreadingModel",	"Apartment");

    RegDBSetKeyValue("A3dApi", "", "A3dApi Object");
    RegDBSetKeyValue("A3dApi\\CLSID", "", "{92FA2C24-253C-11d2-90FB-006008A1F441}");
    RegDBSetKeyValue("CLSID\\{92FA2C24-253C-11d2-90FB-006008A1F441}", "", "A3dApi Object");
    RegDBSetKeyValue("CLSID\\{92FA2C24-253C-11d2-90FB-006008A1F441}","AppID","{92FA2C24-253C-11D2-90FB-006008A1F441}");
	RegDBSetKeyValue("CLSID\\{92FA2C24-253C-11d2-90FB-006008A1F441}\\InprocServer32", "", "a3dapi.dll");
	RegDBSetKeyValue("CLSID\\{92FA2C24-253C-11d2-90FB-006008A1F441}\\InprocServer32", "ThreadingModel", "Apartment");


    RegDBSetKeyValue("A3dDAL", "", "A3dDAL Object");
    RegDBSetKeyValue("A3dDAL\\CLSID", "", "{442D12A1-2641-11d2-90FB-006008A1F441}");
    RegDBSetKeyValue("CLSID\\{442D12A1-2641-11d2-90FB-006008A1F441}", "", "A3dDAL Object");
    RegDBSetKeyValue("CLSID\\{442D12A1-2641-11d2-90FB-006008A1F441}","AppID","{442D12A1-2641-11D2-90FB-006008A1F441}");
	RegDBSetKeyValue("CLSID\\{442D12A1-2641-11d2-90FB-006008A1F441}\\InprocServer32", "", "a3d.dll");
	RegDBSetKeyValue("CLSID\\{442D12A1-2641-11d2-90FB-006008A1F441}\\InprocServer32", "ThreadingModel", "Apartment");


	return S_OK;
}


/**************************************************************************************
 *
 *		Construct, Destruct
 *
 */

void A3DManager::Init(SLONG features) {
	SLONG hr;

#ifndef A3D_SOUND
	return;
#endif

	if (a3droot) return;

	if (!hDDLibWindow) return;

	// Get the api root

	A3dRegister();

	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_A3dApi, NULL, CLSCTX_INPROC_SERVER,
	                      IID_IA3d4, (void **)&a3droot);
    if (FAILED(hr)) return;

	//---
	hr = a3droot->QueryInterface(IID_IA3dGeom, (void **)&a3dgeom);
	if (FAILED(hr)) return;
	//---

	// debug override
//	features=A3D_OCCLUSIONS|A3D_DIRECT_PATH|A3D_DISTANCE_MODEL|A3D_AUTO_DOPPLER;
//	features=A3D_OCCLUSIONS|A3D_DIRECT_PATH|A3D_DISTANCE_MODEL|A3D_AUTO_DOPPLER|A3D_1ST_REFLECTIONS|A3D_LATE_REFLECTIONS;
//	features=A3D_1ST_REFLECTIONS | A3D_OCCLUSIONS | A3D_DIRECT_PATH_A3D;
	features=A3D_1ST_REFLECTIONS | A3D_OCCLUSIONS; // dirpath seems to be not req'd any more

	a3droot->Init(NULL, features,A3DRENDERPREFS_DEFAULT);

	// set the coop level
	
	a3droot->SetCooperativeLevel(hDDLibWindow,A3D_CL_NORMAL);
//	a3droot->SetCooperativeLevel(GetDesktopWindow(),A3D_CL_NORMAL);


	// Use it to acquire a listener

	hr = a3droot->QueryInterface(IID_IA3dListener, (void **)&a3dlis);
    if (FAILED(hr)) return;

	// Enable the resource manager

//	a3droot->SetResourceManagerMode(A3D_RESOURCE_MODE_DYNAMIC);
	// disabled -- seems to be obsoleted in latest version

	// Initialise our ill-gotten gains

	a3dgeom->Enable(A3D_OCCLUSIONS);
//	a3dgeom->Disable(A3D_OCCLUSIONS);

	//a3dgeom->Enable(A3D_DIRECT_PATH_A3D);// not req'd any more?

//	a3dgeom->Enable(A3D_DISTANCE_MODEL);
//	a3dgeom->Disable(A3D_DISTANCE_MODEL);
//	a3dgeom->Enable(A3D_AUTO_DOPPLER);
//	a3dgeom->Disable(A3D_AUTO_DOPPLER);

	a3dgeom->Enable(A3D_1ST_REFLECTIONS);
//	a3dgeom->Enable(A3D_LATE_REFLECTIONS);


//	a3droot->SetCoordinateSystem(A3D_LEFT_HANDED_CS);

	ListenPos(0,0,0);
	ListenRot(0,0,0);

	a3dgeom->LoadIdentity();

	//--- build material library

	a3dgeom->NewMaterial(&mat_lib[A3D_MAT_CARPET]);
	a3dgeom->NewMaterial(&mat_lib[A3D_MAT_STEEL]);
	a3dgeom->NewMaterial(&mat_lib[A3D_MAT_SNDPROOF]);

	mat_lib[A3D_MAT_CARPET]		->SetReflectance(0.8,0.3);
	mat_lib[A3D_MAT_CARPET]		->SetTransmittance(0.8, 0.3);

	mat_lib[A3D_MAT_STEEL]		->SetReflectance(0.9, 1.0);
	mat_lib[A3D_MAT_STEEL]		->SetTransmittance(0.5, 0.5);

	mat_lib[A3D_MAT_SNDPROOF]	->SetReflectance(0.0, 0.0);
	mat_lib[A3D_MAT_SNDPROOF]	->SetTransmittance(1.0, 0.0);

	//---

	a3droot->Clear(); // off we go...
}

A3DManager::A3DManager(SLONG features) {
	a3droot=NULL;
	a3dlis=NULL;
	a3dgeom=NULL;
	memset(mat_lib,0,A3D_MAT_COUNT*sizeof(IA3dMaterial*));
	//Init(features);
}

void A3DCleanUp(void) {
#ifdef A3D_SOUND
	the_a3d_manager.Fini();
#endif
}

void A3DManager::Fini(void) {

	SLONG i;

	//--- free material library

	for (i=0;i<A3D_MAT_COUNT;i++) {
		if (mat_lib[i]) {
			mat_lib[i]->Release();
			mat_lib[i]=0;
		}
	}


	//---

	if (a3droot) a3droot->Flush(); // to restore balance and harmony. or something.
	srclist.Clear();
	datalist.Clear();
	if (a3dlis)
		a3dlis->Release();
	if (a3dgeom) a3dgeom->Release();	
	if (a3droot)
		a3droot->Release();
	a3dgeom=0;
	a3droot=0;
	a3dlis=0;
    CoUninitialize();
}

A3DManager::~A3DManager() {
	Fini();
}

//
// Channel Play
//

A3DSource* A3DManager::Play(A3DData *Original, A3DSource* Channel, UBYTE flags) {
	if (!Channel) {
//		TRACE("Creating\n");
		Channel = new A3DSource(Original);
	} else {
		Channel->Pause();
		if (flags & 2) Channel->Rewind();
		if (Original!=Channel->cloned_from) {
			Channel->Change(Original);
			Channel->Rewind();
		}
	}
//	TRACE("playing %p (a3dsrc=%p)\n",Channel,Channel->GetSource());
	Channel->Flags=flags;
	Channel->Play(flags & 1);
	return Channel;
}

//
// Channel Check
//

BOOL A3DManager::Valid(A3DBase* item) {
	A3DBase *temp;

	try {
	  temp = dynamic_cast<A3DBase*>(item);
	  return TRUE;	// always returns TRUE coz a failed dynamic_cast<> returns NULL, not an exception
	}
	catch (...) {
		return FALSE;
	}
}

A3DSource* A3DManager::ValidChannel(A3DBase* item) {
	A3DSource *temp;

	try {
	  temp = dynamic_cast<A3DSource*>(item);
	  return temp;
	}
	catch (...) {
		return NULL;	// redundant
	}
}

A3DBase* A3DManager::ValidWave(A3DBase* item) {
	A3DBase *temp;

	try {
	  temp = dynamic_cast<A3DBase*>(item);
	  return temp;
	}
	catch (...) {
		return NULL;	// redundant
	}
}


void A3DManager::BindMaterial(SLONG material) {
	a3dgeom->BindMaterial(mat_lib[material]);
}

/**************************************************************************************
 *
 *		Source Construct/Destruct
 *
 */

A3DSource::A3DSource(CBYTE *fn) {
	A3DBase *data=NULL;

	if (fn) {
		data=the_a3d_manager.datalist.Find(fn);
		if (!data) {
			data = new A3DData(fn);
		}
	}
	DupeConstruct(data);

}

A3DSource::A3DSource(A3DBase *original) {
	DupeConstruct(original);
}

void A3DSource::SetupParams() {
	a3dsrc->SetGain(1.0);
	a3dsrc->SetMinMaxDistance(1,6144,A3D_MUTE);
//	a3dsrc->SetMinMaxDistance(1,6144,A3D_AUDIBLE);
//	a3dsrc->SetDistanceModelScale(0.001);
	a3dsrc->SetDistanceModelScale(0.002);
}

void A3DSource::DupeConstruct(A3DBase *original) {
	SLONG hr;
	the_a3d_manager.srclist += this;
	rendermode=0;
	autofree=0;
	queuepos=0;
	User=0;
	cloned_from=original;
	a3dsrc=0;
	if (!original) return;
//	if (original) {
		strcpy(title,original->GetTitle());
		type=original->type;
		owner=original->owner;
		hr = a3droot->DuplicateSource(original->GetSource(),&a3dsrc);
//	}
    if (FAILED(hr)) return;
	if (type&A3DSOURCE_INITIAL_RENDERMODE_NATIVE) {
		a3dsrc->SetRenderMode(A3DSOURCE_RENDERMODE_NATIVE);
	} else {
		SetPositionl(0,0,0);
		a3dsrc->SetRenderMode(A3DSOURCE_RENDERMODE_DEFAULT);
	}
	SetupParams();
}


A3DSource::~A3DSource() {
	Stop();
	FreeWave();
	the_a3d_manager.srclist -= this;
}


/**************************************************************************************
 *
 *		Source Configuration
 *
 */

void A3DSource::SetMute(UBYTE mute) {
/*	if (mute)
		rendermode|=A3D_MUTE;
	else
		rendermode&=!A3D_MUTE;
	if (a3dsrc) a3dsrc->SetRenderMode(rendermode);
	*/
}

void A3DSource::Set3D(UBYTE is3d) {
	// despite what the API docs say, this doesn't seem to actually exist yet. hm.
	/*
	if (is3d)
		rendermode&=!A3D_DISABLE_3D;
	else
		rendermode|=A3D_DISABLE_3D;
	if (a3dsrc) a3dsrc->SetRenderMode(rendermode);
	*/
}

void A3DSource::DoChange(A3DBase *original) {
	SLONG hr;

	FreeWave();
	cloned_from=original;
	if (original) {
		strcpy(title,original->GetTitle());
		hr = a3droot->DuplicateSource(original->GetSource(),&a3dsrc);
		if (FAILED(hr)) {
			TRACE("change failed\n");
			return;
		}
		SetupParams();
	}
}

void A3DSource::Change(A3DBase *original) {
	QueueFlush();
	DoChange(original);
}

void A3DSource::Queue(A3DBase *original, SLONG x, SLONG y, SLONG z, SLONG flags) {
	if (queuepos==MAX_QUEUE_LENGTH) return;
	queue[queuepos].original=original;
	queue[queuepos].x=x;
	queue[queuepos].y=y;
	queue[queuepos].z=z;
	queue[queuepos].flags=flags;
	queuepos++;
}

UBYTE A3DSource::CBEnded() {
/*	UBYTE i;

	if (queuepos) {
		DoChange(queue[0].original);
		SetPositionl(queue[0].x,queue[0].y,queue[0].z);
		Flags=queue[0].flags;
		if (queue[0].flags&A3D_LOOP) 
			Play(1);
		else
			Play(0);
		for (i=1;i<MAX_QUEUE_LENGTH;i++)
			queue[i-1]=queue[i];
		queuepos--;
		return 0;
	}*/
	return 1;
}

/**************************************************************************************
 *
 *		Source Transport Controls
 *
 */

void A3DSource::Play(UBYTE looped) {
	if (a3dsrc) {
		if (looped)
			ErrChk(a3dsrc->Play(A3D_LOOPED));
		else
			ErrChk(a3dsrc->Play(A3D_SINGLE));
	}
}

void A3DSource::Stop() {
	if (a3dsrc) {
		a3dsrc->Stop();
		Rewind();
	}
}

void A3DSource::Rewind() {
	if (a3dsrc) {
		a3dsrc->SetWavePosition(0);
		last_time=0;
	}
}

 
/**************************************************************************************
 *
 *		A3DData -- a source that simply holds data. no transport, location, etc
 *
 */

A3DData::A3DData(CBYTE *fn, UBYTE ntype) {
	HRESULT hr;

	the_a3d_manager.datalist += this;
	next=NULL;
	a3dsrc=NULL;

	owner=0;
	
	type=ntype;
	hr = a3droot->NewSource(type,&a3dsrc);
    if (FAILED(hr)) {
		strcpy(title,"[failed]");
		Decode(hr);
		return;
	}
	if (fn) {
		strcpy(title,fn);
		hr = a3dsrc->LoadWaveFile(fn);
		if (FAILED(hr)) {
			Decode(hr);
			return;
		}
		if (type & A3DSOURCE_INITIAL_RENDERMODE_NATIVE)
			a3dsrc->SetRenderMode(A3DSOURCE_RENDERMODE_NATIVE);
	} else {
		strcpy(title,"[null]");
	}
}


A3DData::~A3DData() {
	FreeWave();
	the_a3d_manager.datalist -= this;
}

/**************************************************************************************
 *
 *		A3DList	-- just a linked list. yawn.
 *
 */


A3DList::~A3DList() {
	Clear();
}

void A3DList::Clear() {
	// this may require some explanation.
	// the action of deleting list calls list's destructor -- which actually adjusts list
	// itself to point to the next item, ready for the next turn. 
	while (list) delete list;
}

void A3DList::Add(A3DBase *item) {
	item->last=tail;
	if (tail) tail->next=item;
	tail=item;
	if (!list) list=item;
	count++;
};

void A3DList::Del(A3DBase *item) {
	if (tail==item) tail=item->last;
	if (item->last)
		item->last->next=item->next;
	else
		list=item->next;
	if (item->next) item->next->last=item->last;
	count--;
};

A3DBase *A3DList::Index(SLONG index) {
	A3DBase *walk=list;

	while ((--index)&&walk) walk=walk->next;
	return walk;
};

A3DBase *A3DList::Find(CBYTE *want) {
	A3DBase *walk=list;

	while (walk&&stricmp(want,walk->GetTitle())) walk=walk->next;
	return walk;
}


/**************************************************************************************
 *
 *		A3DBase	-- the root of all evil
 *
 */

void A3DBase::FreeWave() {
	if (a3dsrc) a3dsrc->Release();
	a3dsrc=NULL;
	length_samples=0;
	length_seconds=0;
}


ULONG A3DBase::GetLengthSamples() {
	WAVEFORMATEX format;
	ULONG size;

	if (a3dsrc) {
		if (!length_samples) {
			a3dsrc->GetWaveFormat(&format);
			size=a3dsrc->GetWaveSize(); // sigh, not implemented yet
			length_samples=size/(format.wBitsPerSample>>3);
		}
	}
	return length_samples;
}

float A3DBase::GetLengthSeconds() {
	WAVEFORMATEX format;
	SLONG size;

	if (a3dsrc) {
		if (length_seconds==0) {
			a3dsrc->GetWaveFormat(&format);
			//size=a3dsrc->GetWaveSize(); // ditto - DONT WORRY ABOUT THE WARNING THIS ISN'T USED
			length_seconds=((float)size)/format.nAvgBytesPerSec;
		}
	}
	return length_seconds;
}


BOOL A3DBase::HasEnded(UBYTE early_out) {
	ULONG t,c;

	if (a3dsrc) {
		
		if (early_out) {
			a3dsrc->GetWavePosition(&t);
			c=a3dsrc->GetWaveSize();
			if (t>=c-4000) { /*TRACE("caught early? %d/%d\n",t,c);*/ return 1; }
		}

/*		a3dsrc->GetWavePosition(&t);
//		TRACE("%d : ( %d )\n",t,last_time);
		if ((!t)&&(!last_time))
			t=1;
		else
			last_time=t;
		return (!t);
		*/
		a3dsrc->GetStatus(&t);
		return !(t&A3DSTATUS_PLAYING);
	}
	return 1;
/*	TRACE("%d : %d\n",t,GetLengthSamples());
	return (t>GetLengthSamples());*/
};



#else //#ifndef TARGET_DC

// Spoof it all...

#include "A3DManager.h"
#include "snd_type.h"
//#include "A3DPlay.h"


IA3d4		*a3droot;
IA3dGeom     *a3dgeom = NULL;
A3DManager	the_a3d_manager(A3D_1ST_REFLECTIONS | A3D_OCCLUSIONS | A3D_DIRECT_PATH_A3D);

void Decode(SLONG hr) {}
void ErrChk(SLONG hr) {}
BOOL Failed(SLONG hr) { return TRUE; }
void A3D_Check_Init(void) {}
static void RegDBSetKeyValue(
	char *szKey,		/* in, key string */
	char *szName,		/* in, name string NULL == Default */
	char *szValue)		/* in, value string */
{}

HRESULT A3dRegister(void) { return 0; }
void A3DManager::Init(SLONG features) {}
A3DManager::A3DManager(SLONG features) {}
void A3DCleanUp(void) {}
void A3DManager::Fini(void) {}
A3DManager::~A3DManager() {}
A3DSource* A3DManager::Play(A3DData *Original, A3DSource* Channel, UBYTE flags) { return NULL; }
BOOL A3DManager::Valid(A3DBase* item) { return TRUE; }
A3DSource* A3DManager::ValidChannel(A3DBase* item) { return NULL; }
A3DBase* A3DManager::ValidWave(A3DBase* item) { return NULL; }
void A3DManager::BindMaterial(SLONG material) {}
A3DSource::A3DSource(CBYTE *fn) {}
A3DSource::A3DSource(A3DBase *original) {}
void A3DSource::SetupParams() {}
void A3DSource::DupeConstruct(A3DBase *original) {}
A3DSource::~A3DSource() {}
void A3DSource::SetMute(UBYTE mute) {}
void A3DSource::Set3D(UBYTE is3d) {}
void A3DSource::DoChange(A3DBase *original) {}
void A3DSource::Change(A3DBase *original) {}
void A3DSource::Queue(A3DBase *original, SLONG x, SLONG y, SLONG z, SLONG flags) {}
UBYTE A3DSource::CBEnded() { return 0; }
void A3DSource::Play(UBYTE looped) {}
void A3DSource::Stop() {}
void A3DSource::Rewind() {}
A3DData::A3DData(CBYTE *fn, UBYTE ntype) {}
A3DData::~A3DData() {}
A3DList::~A3DList() {}
void A3DList::Clear() {}
void A3DList::Add(A3DBase *item) {}
void A3DList::Del(A3DBase *item) {}
A3DBase *A3DList::Index(SLONG index) { return NULL; }
A3DBase *A3DList::Find(CBYTE *want) { return NULL; }
void A3DBase::FreeWave() {}
ULONG A3DBase::GetLengthSamples() { return 0; }
float A3DBase::GetLengthSeconds() { return 0.0f; }
BOOL A3DBase::HasEnded(UBYTE early_out) { return TRUE; }


#endif //#else //#ifndef TARGET_DC


