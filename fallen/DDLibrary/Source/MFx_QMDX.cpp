#include "snd_type.h"
#if defined(Q_SOUND)
//
// MFx.cpp
//
// Muckyfoot sound fx api for A3D / PSX
//
// QMDX Version
//

#include "MFx.h"
#include "QSManager.h"
#include "fallen/headers/fc.h"


//#define DISABLE_MFX


#define MFX_MAX_CHANS		64
#define MFX_MAX_MASK		0x3f;
#define	MFX_MAX_QUEUED		32
#define	MFX_MAX_QUEUED_CHAN	5


static BOOL MFX_initialised=0;

//----- structs --------------------------------------------------------------------

struct MFX_Queue   {
	UWORD		wave;
	SWORD		next;
	ULONG		flags;
	SLONG		x,y,z;
};

/*
struct MFX_Channel {
	UWORD		id;				// thing-id of sound on this channel
	UWORD		wave;			// sound sample playing on this channel
	ULONG		flags;
	SLONG		x,y,z;			// coordinates to play it, if it's positional
	A3DSource*	source;			// a3d source that handles this channel
	Thing*		thing;			// thing pointer
	SWORD     	queue;			// index of queue structure
	UWORD		queuectr;		// number of queued items
};
*/

//----- globals stuff --------------------------------------------------------------
/*
MFX_Channel MFX_channels[MFX_MAX_CHANS];
MFX_Queue	MFX_queue[MFX_MAX_QUEUED];
*/

//----- internal stuff -------------------------------------------------------------
/*
inline SLONG MFX_hash(SLONG chan) { return (chan*3)&MFX_MAX_MASK; } // really crap hash

MFX_Channel* MFX_get_channel(UWORD channel_id, UWORD wave_id) {
	SLONG i, first = MFX_hash(channel_id);
	MFX_Channel *chan = &MFX_channels[first];

	i=first;
	while (1) {
		if ((chan->id==channel_id)&&(chan->wave==wave_id)) return chan;
		i++; chan++;
		if (i>=MFX_MAX_CHANS) {
			i=0; chan=MFX_channels;
		}
		if (i==first) return NULL;
	}
}

MFX_Channel* MFX_get_first_channel(UWORD channel_id) {
	SLONG i, first = MFX_hash(channel_id);
	MFX_Channel *chan = &MFX_channels[first];

	i=first;
	while (1) {
		if (chan->id==channel_id) return chan;
		i++; chan++;
		if (i>=MFX_MAX_CHANS) {
			i=0; chan=MFX_channels;
		}
		if (i==first) return NULL;
	}
}

MFX_Channel* MFX_get_next_channel(MFX_Channel* chan) {
	SLONG i, id = chan->id, first = MFX_hash(chan->id);
	while (1) {
		i++; chan++;
		if (i>=MFX_MAX_CHANS) {
			i=0; chan=MFX_channels;
		}
		if (i==first) return NULL;
		if (chan->id==id) return chan;
	}
}


MFX_Channel* MFX_get_free_channel(SLONG channel_id) {
	SLONG i, first=MFX_hash(channel_id);
	MFX_Channel* chan = &MFX_channels[first];

	i=first;
	while (1) {
		if (!chan->source) return chan;
		i++; chan++;
		if (i>=MFX_MAX_CHANS) {
			i=0; chan=MFX_channels;
		}
		if (i==first) ASSERT(0);
	}
}


void	MFX_kill(MFX_Channel* chan) {
	chan->thing=0;
	if (!chan->source) return;
	delete chan->source;
	chan->source=0;
	chan->flags=chan->id=chan->wave=0;
}

inline MFX_Channel* MFX_locate_for_play(UWORD channel_id, ULONG wave, ULONG flags) {
	if (flags&MFX_OVERLAP) {
		return MFX_get_free_channel(channel_id);
	} else {
		MFX_Channel *chan;
		if (flags&MFX_QUEUED)
			chan=MFX_get_first_channel(channel_id);
		else
			chan=MFX_get_channel(channel_id, wave);
		if (!chan) chan=MFX_get_free_channel(channel_id);
		return chan;
	}
}

inline void	MFX_std_chan_setup(MFX_Channel* chan, UWORD id, ULONG wave, ULONG flags) {
	chan->id=id;
	chan->wave=wave;
	chan->flags=flags;
	chan->source=new A3DSource(the_a3d_manager.datalist[wave]);
	chan->source->User=(SLONG)chan;
	chan->source->autofree=1;
	chan->queue=-1;
	chan->queuectr=0;
	chan->thing=0;
	if (flags&MFX_LO_PRIORITY) chan->source->SetPriority(0.25);
	if (flags&MFX_HI_PRIORITY) chan->source->SetPriority(0.75);
}

inline UWORD MFX_queue_free(SLONG base) {
	SLONG i=base;
	while (1) {
		if (MFX_queue[i].wave==0) return i;
		i++; if (i==MFX_MAX_QUEUED) i=0;
		if (i==base) return -1;
	}
}

inline void MFX_queue_wave(MFX_Channel* chan, UWORD wave, ULONG flags, SLONG x, SLONG y, SLONG z) {
	SLONG ndx;

	if (chan->queuectr>MFX_MAX_QUEUED_CHAN) return;

	if (chan->queue>=0) {
	  ndx=chan->queue;
	  while (MFX_queue[ndx].next>=0) ndx=MFX_queue[ndx].next;
	  MFX_queue[ndx].next=MFX_queue_free(ndx);
	  ndx=MFX_queue[ndx].next;
	} else {
	  chan->queue=ndx=MFX_queue_free(0);
	}
	if (ndx==-1) return;
	chan->queuectr++;
	MFX_queue[ndx].flags=flags;
	MFX_queue[ndx].wave=wave;
	MFX_queue[ndx].x=x;
	MFX_queue[ndx].y=y;
	MFX_queue[ndx].z=z;
	MFX_queue[ndx].next=-1;
}
*/

inline SLONG PlayType(ULONG flags) {
	SLONG type=0;
	if (flags&MFX_LOOPED) type=WAVE_LOOP;
	if (flags&MFX_REPLACE) type|=WAVE_PLAY_INTERUPT;
	if (flags&MFX_OVERLAP) type|=WAVE_PLAY_OVERLAP;
	if (flags&MFX_QUEUED) type|=WAVE_PLAY_QUEUE;
	if (!(flags&(MFX_REPLACE|MFX_OVERLAP|MFX_QUEUED))) type|=WAVE_PLAY_NO_INTERUPT;
	return type;
}

//----- external stuff -------------------------------------------------------------

void	MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z) {
#ifdef DISABLE_MFX
	return;
#endif

	WaveParams params;

	params.Priority				=	1;
	params.Flags					=	WAVE_CARTESIAN;
	params.Mode.Cartesian.Scale	=	(128<<8);
	params.Mode.Cartesian.X		=	x>>8;
	params.Mode.Cartesian.Y		=	y>>8;
	params.Mode.Cartesian.Z		=	z>>8;

	PlayWave(channel_id,wave,PlayType(flags),&params);
/*	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	if (!chan) return; // erk, no free channels

	x>>=8; y>>=8; z>>=8;
	chan->x=x; chan->y=y; chan->z=z;

	// we now have a channel which is either blank or can be stomped
	if (chan->source) {
		if (flags&MFX_QUEUED) {
			MFX_queue_wave(chan,wave,flags,x,y,z);
			return;
		}
		if ((chan->wave==wave)&!(flags&MFX_REPLACE)) {
			chan->source->SetPositionl(chan->x,chan->y,chan->z);
			return;
		}
		MFX_kill(chan);
	}

	MFX_std_chan_setup(chan,channel_id,wave,flags);
	chan->source->SetPositionl(x,y,z);
	chan->source->Play(flags&MFX_LOOPED);
*/
}

void	MFX_play_pos(UWORD channel_id, ULONG wave, ULONG flags, GameCoord* position) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	if (!chan) return; // erk, no free channels

	chan->x=position->X>>8; chan->y=position->Y>>8; chan->z=position->Z>>8;

	// we now have a channel which is either blank or can be stomped
	if (chan->source) {
		if (flags&MFX_QUEUED) {
			MFX_queue_wave(chan,wave,flags,chan->x,chan->y,chan->z);
			return;
		}
		if ((chan->wave==wave)&!(flags&MFX_REPLACE)) {
			chan->source->SetPositionl(chan->x,chan->y,chan->z);
			return;
		}
		MFX_kill(chan);
	}

	MFX_std_chan_setup(chan,channel_id,wave,flags);
	chan->source->SetPositionl(chan->x,chan->y,chan->z);
	chan->source->Play(flags&MFX_LOOPED);*/
	WaveParams params;

	params.Priority				=	1;
	params.Flags					=	WAVE_CARTESIAN;
	params.Mode.Cartesian.Scale	=	(128<<8);
	params.Mode.Cartesian.X		=	position->X>>8;
	params.Mode.Cartesian.Y		=	position->Y>>8;
	params.Mode.Cartesian.Z		=	position->Z>>8;

	PlayWave(channel_id,wave,PlayType(flags),&params);

}

void	MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	if (!chan) return; // erk, no free channels

	chan->x=p->WorldPos.X>>8; chan->y=p->WorldPos.Y>>8; chan->z=p->WorldPos.Z>>8;

	// we now have a channel which is either blank or can be stomped
	if (chan->source) {
		if (flags&MFX_QUEUED) {
			MFX_queue_wave(chan,wave,flags,chan->x,chan->y,chan->z);
			return;
		}
		if ((chan->wave==wave)&!(flags&MFX_REPLACE)) return;
		MFX_kill(chan);
	}

	MFX_std_chan_setup(chan,channel_id,wave,flags);
	chan->source->SetPositionl(chan->x,chan->y,chan->z);
	chan->source->Play(flags&MFX_LOOPED);
	chan->thing=p;
	p->Flags|=FLAGS_HAS_ATTACHED_SOUND;*/
	WaveParams params;

	params.Priority				=	1;
	params.Flags					=	WAVE_CARTESIAN;
	params.Mode.Cartesian.Scale	=	(128<<8);
	params.Mode.Cartesian.X		=	p->WorldPos.X>>8;
	params.Mode.Cartesian.Y		=	p->WorldPos.Y>>8;
	params.Mode.Cartesian.Z		=	p->WorldPos.Z>>8;

	PlayWave(channel_id,wave,PlayType(flags),&params);


}

void	MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	if (!chan) return; // erk, no free channels

	chan->x=FC_cam[0].x;
	chan->y=FC_cam[0].y; // add some fancier stuff later
	chan->z=FC_cam[0].z;

	// we now have a channel which is either blank or can be stomped
	if (chan->source) {
		if (flags&MFX_QUEUED) {
			MFX_queue_wave(chan,wave,flags,chan->x,chan->y,chan->z);
			return;
		}
		if ((chan->wave==wave)&!(flags&MFX_REPLACE)) {
			chan->source->SetPositionl(chan->x,chan->y,chan->z);
			return;
		}
		MFX_kill(chan);
	}

	MFX_std_chan_setup(chan,channel_id,wave,flags);
	chan->source->SetPositionl(chan->x,chan->y,chan->z);
	chan->source->Play(flags&MFX_LOOPED);*/
	WaveParams params;

	params.Priority				=	1;
	params.Flags					=	WAVE_CARTESIAN;
	params.Mode.Cartesian.Scale	=	(128<<8);
	params.Mode.Cartesian.X		=	FC_cam[0].x;
	params.Mode.Cartesian.Y		=	FC_cam[0].y;
	params.Mode.Cartesian.Z		=	FC_cam[0].z;

	PlayWave(channel_id,wave,PlayType(flags),&params);

}

void	MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	if (!chan) return; // erk, no free channels

	// we now have a channel which is either blank or can be stomped
	if (chan->source) {
		if ((chan->wave==wave)&!(flags&MFX_REPLACE)) return;
		MFX_kill(chan);
	}

	MFX_std_chan_setup(chan,channel_id,wave,flags);
	chan->source->Play(flags&MFX_LOOPED);*/
    MFX_play_ambient(channel_id,wave,flags); // heh
}

void	MFX_stop(SLONG channel_id, ULONG wave) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan;

	if (channel_id==MFX_CHANNEL_ALL) {

		UWORD i;
		for (i=0,chan=MFX_channels;i<MFX_MAX_CHANS;i++,chan++) {
			MFX_kill(chan);
		}

	} else {

		if (wave==MFX_WAVE_ALL) {

			MFX_Channel* next;
			chan = MFX_get_first_channel(channel_id);
			while (chan) {
				next=MFX_get_next_channel(chan);
				MFX_kill(chan);
				chan=next;
			}

		} else {

			MFX_kill(MFX_get_channel(channel_id,wave));

		}
	}*/
	StopWave(channel_id,wave);
}

void	MFX_stop_attached(Thing *p) {
#ifdef DISABLE_MFX
	return;
#endif
	StopWave(THING_NUMBER(p),0);
/*	SLONG i;
	MFX_Channel *chan=MFX_channels;

	for (i=0;i<MFX_MAX_CHANS;i++,chan++) {
		if (chan->thing==p) MFX_kill(chan);
	}*/
}

//----- audio processing functions -----

void	MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	float pitch;

	pitch=(float)(pitchbend+256)/256.0;
	if (chan->source) chan->source->SetPitchf(pitch);*/
}

void	MFX_set_wave(UWORD channel_id, ULONG wave, ULONG new_wave) {
#ifdef DISABLE_MFX
	return;
#endif
//	MFX_Channel* chan = MFX_get_channel(channel_id, wave);

}

void	MFX_set_xyz(UWORD channel_id, UWORD wave, SLONG x, SLONG y, SLONG z) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	if (chan->source) chan->source->SetPositionl(x>>8,y>>8,z>>8);*/
}


void	MFX_set_pos(UWORD channel_id, UWORD wave, GameCoord* position) {
#ifdef DISABLE_MFX
	return;
#endif
/*	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	if (chan->source) chan->source->SetPositionl(position->X>>8,position->Y>>8,position->Z>>8);*/
}

//----- listener & environment -----

extern void	SetListenerOrientation(SLONG angle,SLONG roll,SLONG tilt);


void	MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch) {
#ifdef DISABLE_MFX
	return;
#endif

  SetListenerPosition(x>>8,y>>8,z>>8,128<<8);
  SetListenerOrientation(heading,roll,pitch);

/*	float h,p,r;

	heading*=360; h=(float)heading; h/=2048;
	pitch*=360; p=(float)pitch; p/=2048;
	a3dgeom->LoadIdentity();
	a3dgeom->PushMatrix();
	  a3dgeom->Translate3f((float)x/256.0,(float)y/256.0,(float)z/256.0);
	  a3dgeom->Rotate3f(-h,0,1,0);
	  a3dgeom->Rotate3f(p,1,0,0);
	  a3dgeom->Scale3f(1,1,-1);
	  a3dgeom->BindListener();
	a3dgeom->PopMatrix();*/

}

void	MFX_set_environment(SLONG env_type) {
	// do nothing for the moment.
	// later, probably set up some funkatronic a3d stuff
}

//----- sound library functions -----

extern CBYTE *sound_list[];
extern void	LoadWave(CBYTE *wave_name);

void	MFX_load_wave_list(CBYTE *names[]) {
#ifdef DISABLE_MFX
	return;
#endif

  SLONG i;
  CBYTE buff[_MAX_PATH];
  void *oldnames=NULL;

  if (names==0) names=sound_list;

  if (oldnames==names) return;

  oldnames=names;

//  MFX_free_wave_list();
  FreeWaveList();

//  memset(MFX_channels,0,sizeof(MFX_Channel)*MFX_MAX_CHANS);

  i=0;
  while (strcmp(names[i],"!")) {

	  if (stricmp("NULL.wav",names[i])) {
		strcpy(buff,"Data\\sfx\\1622\\");
		strcat(buff,names[i]);
//	    MFX_load_wave_file(buff);
		LoadWave(buff);
	  }
	  i++;
  }
}

void	MFX_load_wave_list(CBYTE *path,CBYTE *script_file) {
#ifdef DISABLE_MFX
	return;
#endif

	FILE			*script_handle;
	CBYTE			wave_name[MAX_PATH],
					wave_file[MAX_PATH];
	ULONG			streamed; // currently ignored...

//	MFX_free_wave_list();
	FreeWaveList();

//	memset(MFX_channels,0,sizeof(MFX_Channel)*MFX_MAX_CHANS);

	script_handle	=	MF_Fopen(script_file,"r");
	if(script_handle)
	{
		while(fscanf(script_handle,"%s %d",wave_name,&streamed)>0)
		{
			strcpy(wave_file,path);
			strcat(wave_file,wave_name);
			// cunning correction thingy
			if (stricmp("NULL.wav",wave_name))
//				MFX_load_wave_file(wave_file);
				LoadWave(wave_file);
		}

		// Finished with the script.
		MF_Fclose(script_handle);
	}

}


void	MFX_load_wave_file(CBYTE *wave_file) {
#ifdef DISABLE_MFX
	return;
#endif
/*
	A3DData			*wave;

	if (strstr(wave_file,"music\\")) {
	  wave = new A3DData(wave_file,A3DSOURCE_INITIAL_RENDERMODE_NATIVE);
	} else {
	  wave = new A3DData(wave_file);
	}
	wave->owner=1;*/
}

void	MFX_free_wave_list(void) {
#ifdef DISABLE_MFX
	return;
#endif
	/*A3DData *item, *next;

	if (MFX_initialised)
		MFX_stop(MFX_CHANNEL_ALL,MFX_WAVE_ALL);
	else
		memset(MFX_channels,0,sizeof(MFX_Channel)*MFX_MAX_CHANS);

	MFX_initialised=1;

	item=static_cast<A3DData*>(the_a3d_manager.datalist.Head());
	while (item) {
		next=static_cast<A3DData*>(item->next);
		if (item->owner==1) delete item;
		item=next;
	}*/
}


//----- general system stuff -----

void	MFX_render() {
#ifdef DISABLE_MFX
	return;
#endif
/*	A3DSource *item, *next;
	MFX_Channel* targ;
	MFX_Queue* q;

	the_a3d_manager.Render();
	item=static_cast<A3DSource*>(the_a3d_manager.srclist.Head());
	while (item) {
		next=static_cast<A3DSource*>(item->next);
		targ=(MFX_Channel*)item->User;
		if (item->autofree&&item->HasEnded()) {
			if (targ) {
				if (!targ->queuectr) {
//					TRACE("[MFX] deleted\n");
					targ->source=0;
					delete item;
					if (targ->thing) targ->thing->Flags&=~FLAGS_HAS_ATTACHED_SOUND;
				} else {
//					TRACE("[MFX] queue-play\n");
					q=&MFX_queue[targ->queue];
					// update targ
					targ->x=q->x; targ->y=q->y; targ->z=q->z;
					targ->flags=q->flags;
					targ->wave=q->wave;
					targ->queuectr--;
					targ->queue=q->next;
					// free q
					q->wave=0;
					// update source
					item->Change(the_a3d_manager.datalist[targ->wave]);
					item->SetPositionl(targ->x,targ->y,targ->z);
					item->Play(q->flags&MFX_LOOPED);
				}
			} else {
//				TRACE("[MFX] deleted (no User)\n");
				delete item;
			}
		} else {
			// it continues playing... we may need to do some processing
			if (targ) {
				if (targ->flags&MFX_CAMERA) {
//					TRACE("[MFX] repositioned by camera\n");
					targ->x=FC_cam[0].x>>8;
					targ->z=FC_cam[0].z>>8;
					if (!(targ->flags&MFX_LOCKY)) targ->y=FC_cam[0].y>>8;
					item->SetPositionl(targ->x,targ->y,targ->z);
				}
				if ((targ->flags&MFX_MOVING)&&(targ->thing)) {
				  targ->x=targ->thing->WorldPos.X>>8;
				  targ->y=targ->thing->WorldPos.Y>>8;
				  targ->z=targ->thing->WorldPos.Z>>8;
				  item->SetPositionl(targ->x,targ->y,targ->z);
				}
			}
		}
		item=next;
	}*/
}

#endif
