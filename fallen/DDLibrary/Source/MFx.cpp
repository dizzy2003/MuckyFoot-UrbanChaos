#if 0
#include "snd_type.h"
#if defined(Q_SOUND) || defined(NO_SOUND)

// since if NO_SOUND is defined, we still need MFx.cpp but with the short-circuited functions
// but if Q_SOUND is defined, MFx_QMDX will handle it, so this one needs taking out

//
// MFx.cpp
//
// Muckyfoot sound fx api for A3D / PSX
//
// A3D Version
//

#include "MFx.h"
#include "A3DManager.h"
#include "fallen/headers/fc.h"

#ifdef	TARGET_DC
#include "target.h"
#endif

#define MFX_MAX_CHANS		64
#define MFX_MAX_MASK		0x3f;
#define	MFX_MAX_QUEUED		32
#define	MFX_MAX_QUEUED_CHAN	5


extern IA3dGeom     *a3dgeom;

static BOOL MFX_initialised=0;

//----- structs --------------------------------------------------------------------

struct MFX_Queue   {
	UWORD		wave;
	SWORD		next;
	ULONG		flags;
	SLONG		x,y,z;
	float		gain;
};

struct MFX_Channel {
	UWORD		id;				// thing-id of sound on this channel
	UWORD		wave;			// sound sample playing on this channel
	ULONG		flags;
	SLONG		x,y,z;			// coordinates to play it, if it's positional
	A3DSource*	source;			// a3d source that handles this channel
	Thing*		thing;			// thing pointer
	SWORD     	queue;			// index of queue structure
	UWORD		queuectr;		// number of queued items
//	float		gain;			// channel gain
};


//----- globals stuff --------------------------------------------------------------

MFX_Channel MFX_channels[MFX_MAX_CHANS];
MFX_Queue	MFX_queue[MFX_MAX_QUEUED];


//----- internal stuff -------------------------------------------------------------

void	MFX_trigger_paired_channel(UWORD channel_id);
void	MFX_load_wave_file        (CBYTE *wave_file);


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
	SLONG i=chan-MFX_channels, id = chan->id, first = MFX_hash(chan->id);
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
//		if (i==first) ASSERT(0);
		if (i==first) return 0;
	}
}


void	MFX_kill(MFX_Channel* chan) {
	if (!chan) return;
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
		if (flags&(MFX_QUEUED|MFX_NEVER_OVERLAP))
			chan=MFX_get_first_channel(channel_id);
		else
			chan=MFX_get_channel(channel_id, wave);
		if (!chan)
			chan=MFX_get_free_channel(channel_id);
		else
			if ((flags&(MFX_NEVER_OVERLAP|MFX_QUEUED))==MFX_NEVER_OVERLAP) return 0; // existing sound is fine...
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
	MFX_Queue* q;

	if ((flags&&MFX_SHORT_QUEUE)&&chan->queuectr) {
		// short queue -- queued wave will always be the next wave played
		q=&MFX_queue[chan->queue];
		q->flags=flags;
		q->wave=wave;
		q->x=x; q->y=y; q->z=z;
		return;
	}

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
	MFX_queue[ndx].gain=1;
	MFX_queue[ndx].next=-1;
}

//----- external stuff -------------------------------------------------------------

void	MFX_play_xyz(UWORD channel_id, ULONG wave, ULONG flags, SLONG x, SLONG y, SLONG z) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	ASSERT(chan);

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
	if (!(flags&MFX_PAIRED_TRK2)) chan->source->Play(flags&MFX_LOOPED);
	if (flags&MFX_PAIRED_TRK1) MFX_trigger_paired_channel(channel_id+1);

}

void	MFX_play_pos(UWORD channel_id, ULONG wave, ULONG flags, GameCoord* position) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

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
	if (!(flags&MFX_PAIRED_TRK2)) chan->source->Play(flags&MFX_LOOPED);
	if (flags&MFX_PAIRED_TRK1) MFX_trigger_paired_channel(channel_id+1);
}

void	MFX_play_thing(UWORD channel_id, ULONG wave, ULONG flags, Thing* p) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

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
	if (!(flags&MFX_PAIRED_TRK2)) chan->source->Play(flags&MFX_LOOPED);
	chan->thing=p;
	p->Flags|=FLAGS_HAS_ATTACHED_SOUND;
	if (flags&MFX_PAIRED_TRK1) MFX_trigger_paired_channel(channel_id+1);

}

void	MFX_play_ambient(UWORD channel_id, ULONG wave, ULONG flags) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

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
	if (!(flags&MFX_PAIRED_TRK2)) chan->source->Play(flags&MFX_LOOPED);
	if (flags&MFX_PAIRED_TRK1) MFX_trigger_paired_channel(channel_id+1);
}

UBYTE	MFX_play_stereo(UWORD channel_id, ULONG wave, ULONG flags) {
#ifdef NO_SOUND
	return(0);
#endif
	MFX_Channel* chan=MFX_locate_for_play(channel_id, wave, flags);

	if (!chan) return 0; // erk, no free channels

	// we now have a channel which is either blank or can be stomped
	if (chan->source) {
		if (flags&MFX_QUEUED) {
			MFX_queue_wave(chan,wave,flags,0,0,0);
			return 2;
		}
		if ((chan->wave==wave)&!(flags&MFX_REPLACE)) return 0;
		MFX_kill(chan);
	}

	MFX_std_chan_setup(chan,channel_id,wave,flags);
	if (!(flags&MFX_PAIRED_TRK2)) {
		//TRACE("play stereo: live\n");
		chan->source->Play(flags&MFX_LOOPED);
	}
	if (flags&MFX_PAIRED_TRK1) MFX_trigger_paired_channel(channel_id+1);
	return 1;
}

void	MFX_stop(SLONG channel_id, ULONG wave) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan;

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
	}
}

void	MFX_stop_attached(Thing *p) {
#ifdef NO_SOUND
	return;
#endif
	SLONG i;
	MFX_Channel *chan=MFX_channels;

	for (i=0;i<MFX_MAX_CHANS;i++,chan++) {
		if (chan->thing==p) MFX_kill(chan);
	}
}

//----- audio processing functions -----


void	MFX_set_pitch(UWORD channel_id, ULONG wave, SLONG pitchbend) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	float pitch;

	if(!chan)
		return;

	pitch=(float)(pitchbend+256)/256.0f;
	if (chan->source) chan->source->SetPitchf(pitch);
}

void	MFX_set_gain(UWORD channel_id, ULONG wave, UBYTE gain) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	float fgain;

	if(!chan)
		return;

	fgain=(float)(gain)/256.0f;
	//TRACE("gain set: %f\n",fgain);
	if (chan->source) chan->source->SetGainf(fgain);
}

void	MFX_set_queue_gain(UWORD channel_id, ULONG wave, UBYTE gain) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	MFX_Queue* q;
	float fgain;

	if(!chan) return;

	fgain=(float)(gain)/256.0f;
	if (chan->queue>=0)
		q=&MFX_queue[chan->queue];
	else { // assume we really want to set the current one?
//		TRACE("queuef (short circuit) gain set: %f\n",fgain);
		if (chan->source) chan->source->SetGainf(fgain);
		return;
	}
//	TRACE("queuef gain set: %f\n",fgain);
	while (q) {
		q->gain=fgain;
		q=(q->next)?&MFX_queue[q->next]:0;
	}

}

/*
void	MFX_set_channel_gain(UWORD channel_id, UBYTE gain) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id,0);

	if (!chan) return;
	chan->gain=(float)(gain)/256.0;
	if (chan->source) chan->source->SetGainf(chan->gain);
}
*/

void	MFX_set_wave(UWORD channel_id, ULONG wave, ULONG new_wave) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
}


void	MFX_set_xyz(UWORD channel_id, UWORD wave, SLONG x, SLONG y, SLONG z) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	if(!chan)
		return;
	if (chan->source) chan->source->SetPositionl(x>>8,y>>8,z>>8);
}


void	MFX_set_pos(UWORD channel_id, UWORD wave, GameCoord* position) {
#ifdef NO_SOUND
	return;
#endif
	MFX_Channel* chan = MFX_get_channel(channel_id, wave);
	if(!chan)
		return;
	if (chan->source) chan->source->SetPositionl(position->X>>8,position->Y>>8,position->Z>>8);
}

//----- listener & environment -----

void	MFX_set_listener(SLONG x, SLONG y, SLONG z, SLONG heading, SLONG roll, SLONG pitch) {
#ifdef NO_SOUND
	return;
#endif
	float h,p,r;

	heading*=360; h=(float)heading; h/=2048;
	pitch*=360; p=(float)pitch; p/=2048;
	a3dgeom->LoadIdentity();
	a3dgeom->PushMatrix();
	  a3dgeom->Translate3f((float)x/256.0f,(float)y/256.0f,(float)z/256.0f);
	  a3dgeom->Rotate3f(-h,0,1,0);
	  a3dgeom->Rotate3f(p,1,0,0);
	  a3dgeom->Scale3f(1,1,-1);
	  a3dgeom->BindListener();
	a3dgeom->PopMatrix();

}

void	MFX_set_environment(SLONG env_type) {
	// do nothing for the moment.
	// later, probably set up some funkatronic a3d stuff
}

//----- sound library functions -----

extern CBYTE *sound_list[];

void	MFX_load_wave_list(CBYTE *names[]) {
#ifdef NO_SOUND
	return;
#endif

  SLONG i;
  CBYTE buff[_MAX_PATH];
  void *oldnames=NULL;

  if (names==0) names=sound_list;

  if (MFX_initialised)
		MFX_stop(MFX_CHANNEL_ALL,MFX_WAVE_ALL);

  if (oldnames==names) return;

  oldnames=names;

  MFX_free_wave_list();

  memset(MFX_channels,0,sizeof(MFX_Channel)*MFX_MAX_CHANS);

  i=0;
  while (strcmp(names[i],"!")) {

	  if (stricmp("NULL.wav",names[i])) {
		strcpy(buff,"Data\\sfx\\1622\\");
		strcat(buff,names[i]);
	    MFX_load_wave_file(buff);
	  }
	  i++;
  }
}

void	MFX_load_wave_list(CBYTE *path,CBYTE *script_file) {
#ifdef NO_SOUND
	return;
#endif

	FILE			*script_handle;
	CBYTE			wave_name[MAX_PATH],
					wave_file[MAX_PATH];
	ULONG			streamed; // currently ignored...
//	static CBYTE previous_list[_MAX_PATH]={0};

//	if (!stricmp(previous_list,script_file)) return;

	MFX_free_wave_list();

//	strcpy(previous_list,script_file);

	memset(MFX_channels,0,sizeof(MFX_Channel)*MFX_MAX_CHANS);

	script_handle	=	MF_Fopen(script_file,"r");
	if(script_handle)
	{
		while(fscanf(script_handle,"%s %d",wave_name,&streamed)>0)
		{
			strcpy(wave_file,path);
			strcat(wave_file,wave_name);
			// cunning correction thingy
			if (stricmp("NULL.wav",wave_name))
				MFX_load_wave_file(wave_file);
		}

		// Finished with the script.
		MF_Fclose(script_handle);
	}

}


void	MFX_load_wave_file(CBYTE *wave_file) {
#ifdef NO_SOUND
	return;
#endif

	A3DData			*wave;

	if (strstr(wave_file,"music\\")) {
	  wave = new A3DData(wave_file,A3DSOURCE_INITIAL_RENDERMODE_NATIVE);
	} else {
	  wave = new A3DData(wave_file);
	}
	wave->owner=1;
}

void	MFX_free_wave_list(void) {
#ifdef NO_SOUND
	return;
#endif
	A3DData *item, *next;

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
	}
}

//----- querying information back -----

UWORD	MFX_get_wave(UWORD channel_id, UBYTE index) {
	MFX_Channel *chan=MFX_get_first_channel(channel_id);
	while (index) {
		chan=MFX_get_next_channel(chan);
		index--;
	}
	return chan?chan->wave:0;

}

//----- general system stuff -----

void	MFX_render ( void ) {
#ifdef NO_SOUND
	return;
#endif
	A3DSource *item, *next;
	MFX_Channel* targ;
	MFX_Queue* q;

	the_a3d_manager.Render();
	item=static_cast<A3DSource*>(the_a3d_manager.srclist.Head());
	while (item) {
		next=static_cast<A3DSource*>(item->next);
		targ=(MFX_Channel*)item->User;
		if (targ->flags&MFX_PAIRED_TRK2) { item=next; continue; }
		if (item->autofree&&item->HasEnded((targ->flags&MFX_EARLY_OUT)?1:0)) {
			if (targ) {
				if (!targ->queuectr) {
//					TRACE("[MFX] deleted\n");
					targ->source=0;
					delete item;
					if (targ->thing) targ->thing->Flags&=~FLAGS_HAS_ATTACHED_SOUND;
				} else {
					float gain;
//					if (targ->thing&&(targ->thing->Class==CLASS_VEHICLE)) TRACE("[MFX] queue-play\n");
					q=&MFX_queue[targ->queue];
					gain=q->gain;

					if (q->flags&MFX_PAIRED_TRK1) MFX_trigger_paired_channel(targ->id+1);

					// update targ
					targ->flags=q->flags&~MFX_PAIRED_TRK2; // no longer relevant
					if ((targ->flags&MFX_MOVING)&&(targ->thing)) {
					  targ->x=targ->thing->WorldPos.X>>8;
					  targ->y=targ->thing->WorldPos.Y>>8;
					  targ->z=targ->thing->WorldPos.Z>>8;
					} else {
						targ->x=q->x; targ->y=q->y; targ->z=q->z;
					}
					//if (q->wave>100) TRACE("MFX play from queue -- w%d gain%f\n",q->wave,gain);
					targ->wave=q->wave;
					targ->queuectr--;
					targ->queue=q->next;
					// free q
					q->wave=0;
					// update source
					item->Change(the_a3d_manager.datalist[targ->wave]);
					item->SetPositionl(targ->x,targ->y,targ->z);
					item->SetGainf(gain);
					item->Play(q->flags&MFX_LOOPED);
					the_a3d_manager.Render();

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
	}
}


void	MFX_trigger_paired_channel(UWORD channel_id) {
	MFX_Channel* chan;

	chan = MFX_get_first_channel(channel_id);

	if (!chan||!chan->source) return;

	chan->flags&=~MFX_PAIRED_TRK2;
	chan->source->Play(chan->flags&MFX_LOOPED);
}



#endif //#if defined(Q_SOUND) || defined(NO_SOUND)
#endif
