// cutscene.cpp
// matthew rosenfeld 22 apr 1999


#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"
#include	"inline.h"
#include	"gi.h"
#include	"fmatrix.h"
#include	"game.h"
#include	"person.h"

#include	"GEdit.h"
#include	"MapView.h"
#include	"console.h"
#include	"propedit.h"

#include	"cutscene.h"

#include	"fallen/editor/headers/anim.h"
#include	"poly.h"
#include	"polypage.h"
#include	"animate.h"
#include	"fc.h"
#include	"drawxtra.h"

#include	"mfx.h"
#include	"sound.h"
#include	"sound_id.h"

#include	"font2d.h"


#define IM_SCENE_FOLDER		(0)
#define	IM_SCENE_SPEAKER	(1)
#define IM_SCENE_PERSON		(2)
#define	IM_SCENE_ANIM		(3)
#define	IM_SCENE_CAMERA		(4)
#define IM_SCENE_WAVE		(5)
#define IM_SCENE_ACTION		(6)
#define IM_SCENE_WARNING	(7)
#define IM_SCENE_BUBBLE		(8)

#define CT_CHAR				(1)
#define CT_CAM				(2)
#define CT_WAVE				(3)
#define CT_FX				(4)
#define CT_TEXT				(5)

#define PT_ANIM				(1)
#define PT_ACTION			(2)
#define PT_WAVE 			(3)
#define PT_CAM				(4)
#define PT_TEXT				(5)

#define CUTSCENE_DATA_VERSION (2)

#define PF_BACKWARDS		(1) // for anims
#define PF_SECURICAM		(1) // for cameras
#define PF_INTERPOLATE_MOVE	(2) // for both
#define PF_SMOOTH_MOVE_IN	(4)
#define PF_SMOOTH_MOVE_OUT	(8)
#define PF_SMOOTH_MOVE_BOTH (PF_SMOOTH_MOVE_IN|PF_SMOOTH_MOVE_OUT)
#define PF_INTERPOLATE_ROT	(16)
#define PF_SMOOTH_ROT_IN	(32)
#define PF_SMOOTH_ROT_OUT	(64)
#define PF_SMOOTH_ROT_BOTH	(PF_SMOOTH_ROT_IN|PF_SMOOTH_ROT_OUT)
#define PF_SLOMO			(128)


// channel memory is reserved in chunks, and expanded when needed
// by creating a new block and copying the old one across. this isn't
// terribly quick, but it's irrelevant in the editor, and in the game
// the number of channels is known at load-time. saves wasting in-game
// memory on stuff like prev/next linked list pointers.

struct CSEditChannel;

struct CSData {
	UBYTE			 version;
	UBYTE			 channelcount;
	UBYTE			 pad1, pad2;
	CSChannel		*channels;
	CSEditChannel	*editchannels;
};

struct CSChannel {
	UBYTE		 type;			// 0=unused, 1=character, 2=camera, 3=spot sound, 4=vfx
	UBYTE		 flags;			// come up with some later... :P
	UBYTE		 pad1,pad2;
	UWORD		 index;			// of the sound/character or the type of fx
	UWORD		 packetcount;	//
	CSPacket	*packets;		//
};

struct CSPacket {
	UBYTE		type;		// 0=unused, 1=animation, 2=action, 3=sound, 4=camerarec		0
	UBYTE		flags;		// come up with some later... :P
	UWORD		index;		// of animation, sound etc										4
	UWORD		start;		// time of packet start
	UWORD		length;		// natural packet length										8
	GameCoord	pos;		// location														20
	UWORD		angle,pitch;// no roll :P
};

// this is a parallel to CSData which is not saved, but created afresh as you 'load' a CSData
// and freed on exit. it stores info the editor needs to keep track of but the game doesn't give
// a toss about

struct CSEditChannel {
	CSChannel*		data;
	Thing*			thing;
};

//---------------------------------------------------------------

CBYTE interpolate_strings[]="snap|linear|smooth in|smooth out|smooth both";

//---------------------------------------------------------------

#define SLOMO_RATE	(10)

PropertyEditor	*pedit;
TreeBrowser		*browser;
DragServer		*drag;
TimeLine		*timeline;
TimeLineRuler	*ruler;
TimeLineScroll	*scroll;
CSData			*cutscene;
CSPacket		*current_packet=0;

HWND			CUTSCENE_edit_wnd=0;
BOOL			CUTSCENE_mouselook=0;
BOOL			CUTSCENE_playback=0;
BOOL			CUTSCENE_slomo=0;
BOOL			CUTSCENE_need_keyboard=0;
HTREEITEM		darcianim,roperanim,soundbase;
int				CUTSCENE_slomo_ctr=SLOMO_RATE;
UBYTE			CUTSCENE_fade_level=255;
CBYTE			subtitle_str[255];

//---------------------------------------------------------------

extern void	calc_camera_pos(void);

//---------------------------------------------------------------
// Data structure handlers -- alloc, free, etc

void CUTSCENE_chan_free(CSChannel &chan) {
	int i;
	CSPacket *pkt=chan.packets;
	for (i=0;i<chan.packetcount;i++,pkt++)
		if ((pkt->type==PT_TEXT)&&(pkt->pos.X))
			free((CBYTE*)pkt->pos.X);
	if (chan.packets) delete [] chan.packets;
}

void CUTSCENE_editchan_free(CSEditChannel &chan) {
	if (chan.thing) {
		//if (chan.data->type==CT_CHAR) free_person(chan.thing);
		THING_kill(chan.thing);
	}
	chan.thing=0;
}

void CUTSCENE_chan_delete(CSData* cutscene, int chan) {
	CSChannel *buff, *buff2;
	CSEditChannel *edit, *edit2;
	int ct=cutscene->channelcount-1;

	if ((chan<0)||(chan>ct)) return;

	buff=cutscene->channels+chan;

	// frees Thing structures and the like associated with the channel
	CUTSCENE_editchan_free(cutscene->editchannels[chan]);
	// this only frees the resources allocated by the channel, not the channel itself :(
	// yes, that sucks
	CUTSCENE_chan_free(*buff);

	// now free the channel itself (messy)
	buff2 = buff = new CSChannel[ct];
	if (chan>0) {
		memcpy(buff,cutscene->channels,sizeof(cutscene->channels[0])*chan);
		buff2+=chan;
	}
	if (chan<ct) {
		memcpy(buff2,cutscene->channels+chan+1,sizeof(cutscene->channels[0])*(ct-chan));
	}
	delete [] cutscene->channels;

	edit2 = edit = new CSEditChannel[ct];
	if (chan>0) {
		memcpy(edit,cutscene->editchannels,sizeof(cutscene->editchannels[0])*chan);
		edit2+=chan;
	}
	if (chan<ct) {
		memcpy(edit2,cutscene->editchannels+chan+1,sizeof(cutscene->editchannels[0])*(ct-chan));
	}
	delete [] cutscene->editchannels;

	cutscene->channelcount--;
	cutscene->channels=buff;
	cutscene->editchannels=edit;
}

CSData* CUTSCENE_data_alloc() {
	CSData* data = new CSData;
	data->channelcount=0;
	data->version=CUTSCENE_DATA_VERSION;
	data->channels=0;
	data->editchannels=0;
	return data;
}

void CUTSCENE_data_free(CSData* data) {
	if (data->channels) {
		int i;
		for (i=0;i<data->channelcount;i++) {
			CUTSCENE_chan_free(data->channels[i]);
			CUTSCENE_editchan_free(data->editchannels[i]);
		}
		delete [] data->channels;
		delete [] data->editchannels;
	}
	delete data;
}

CSChannel* CUTSCENE_add_channel(CSData* data) {
	CSChannel* buff = new CSChannel[data->channelcount+1];
	memcpy(buff,data->channels,sizeof(data->channels[0])*data->channelcount);
	delete [] data->channels;
//	data->channelcount++;
	data->channels=buff;
	buff=&data->channels[data->channelcount];
	buff->flags=0; buff->packetcount=0; buff->packets=0; buff->index=0; buff->type=0;

	CSEditChannel* buff2 = new CSEditChannel[data->channelcount+1];
	memcpy(buff2,data->editchannels,sizeof(data->editchannels[0])*data->channelcount);
	delete [] data->editchannels;
	data->editchannels=buff2;
	buff2=&data->editchannels[data->channelcount];
	data->channelcount++;
	buff2->data=buff;
	buff2->thing=0;

	return buff;
}

CSPacket* CUTSCENE_add_packet(CSChannel* chan) {
	CSPacket* buff = new CSPacket[chan->packetcount+1];
	memcpy(buff,chan->packets,sizeof(chan->packets[0])*chan->packetcount);
	delete [] chan->packets;
	chan->packetcount++;
	chan->packets=buff;
	buff=&chan->packets[chan->packetcount-1];
	buff->flags=0; buff->length=0; buff->type=0;
	return buff;
}

void CUTSCENE_del_packet(CSChannel* chan, int pack) {
	CSPacket* buff;
	CSPacket* buff2;
	int ct=chan->packetcount-1;

	if ((pack<0)||(pack>=chan->packetcount)) return;

	buff=chan->packets+pack;
	if ((buff->type==PT_TEXT)&&(buff->pos.X)) free((CBYTE*)buff->pos.X);

	buff2 = buff = new CSPacket[ct];
	if (pack>0) {
		memcpy(buff,chan->packets,sizeof(chan->packets[0])*pack);
		buff2+=pack;
	}
	if (pack<ct) {
		memcpy(buff2,chan->packets+pack+1,sizeof(chan->packets[0])*(ct-pack));
	}
	delete [] chan->packets;
	chan->packetcount--;
	chan->packets=buff;
}


CSPacket *CUTSCENE_get_packet(CSChannel* chan, int cell) {
	CSPacket* pkt=chan->packets;
	int ctr=1;
	if ((cell<0)||(cell>2000)) return 0;
	while (pkt&&(pkt->start!=cell)) {
		pkt++;
		ctr++;
		if (ctr>chan->packetcount) pkt=0;
	}
	return pkt;
}

//-----------------------------------------------------
// Load/save cutscene data
//-----------------------------------------------------

void CUTSCENE_write_packet(FILE *file_handle, CSPacket* pack, int version) {
	fwrite((void*)pack,sizeof(CSPacket),1,file_handle);
	if ((pack->type==PT_TEXT)&&(pack->pos.X)) {
		int l=strlen((CBYTE*)pack->pos.X);
		fwrite((void*)&l,sizeof(l),1,file_handle);
		fwrite((void*)pack->pos.X,l,1,file_handle);
	}
}

void CUTSCENE_write_channel(FILE *file_handle, CSChannel* chan, int version) {
	SLONG packnum;
	CSPacket* pack=chan->packets;

	fwrite((void*)chan,sizeof(CSChannel),1,file_handle);
	for (packnum=0;packnum<chan->packetcount;packnum++,pack++) CUTSCENE_write_packet(file_handle,pack,version);
}

void CUTSCENE_write(FILE *file_handle, CSData* cutscene) {
	UBYTE channum;
	CSChannel* chan=cutscene->channels;
	cutscene->version=CUTSCENE_DATA_VERSION;
	fwrite((void*)&cutscene->version,1,1,file_handle);
	fwrite((void*)&cutscene->channelcount,1,1,file_handle);
	for (channum=0;channum<cutscene->channelcount;channum++,chan++) CUTSCENE_write_channel(file_handle,chan,cutscene->version);
}

// --- read ---

void CUTSCENE_read_packet(FILE *file_handle, CSChannel* chan, int version) {
	CSPacket *pack = CUTSCENE_add_packet(chan);
	fread((void*)pack,sizeof(CSPacket),1,file_handle);
	switch(version) {
	case 1:
		if (pack->type==PT_CAM) pack->length=0xff7f;
		break;
	}
	switch(pack->type) {
	case PT_TEXT:
		if (pack->pos.X) {
			int l;
			fread((void*)&l,sizeof(l),1,file_handle);
			pack->pos.X=(int)malloc(l+1);
			ZeroMemory((void*)pack->pos.X,l+1);
			fread((void*)pack->pos.X,l,1,file_handle);
		}
		break;
	}
}

void CUTSCENE_read_channel(FILE *file_handle, CSData* cutscene) {
	SLONG packnum, packct;
	CSChannel *chan = CUTSCENE_add_channel(cutscene);
	fread((void*)chan,sizeof(CSChannel),1,file_handle);
	chan->packets=0; packct=chan->packetcount; chan->packetcount=0;
	for (packnum=0;packnum<packct;packnum++) CUTSCENE_read_packet(file_handle,chan,cutscene->version);
}

void CUTSCENE_read(FILE *file_handle, CSData** cutsceneref) {
	UBYTE channum, chanct;
	CSData* cutscene=CUTSCENE_data_alloc();
	*cutsceneref=cutscene;
	fread((void*)&cutscene->version,1,1,file_handle);
	fread((void*)&chanct,1,1,file_handle); // cutscene->channelcount will be updated by read_channel's call to add_channel...
	for (channum=0;channum<chanct;channum++) CUTSCENE_read_channel(file_handle,cutscene);
}


//-----------------------------------------------------
// Recreate in-editor data for cutscene
//-----------------------------------------------------


Thing* CUTSCENE_create_person(UBYTE person_type, SLONG x, SLONG y, SLONG z) {
	SLONG  person_index;

	person_index = create_person(
		person_type,
		0,
		x << 8,
		y << 8,
		z << 8);
	return TO_THING(person_index);
}



CBYTE* PeopleStrings[] = { "Darci", "Roper", "Cop", "Civvy", "Rasta Thug", "Grey Thug",
	"Red Thug", "Tart", "Slag", "Hostage", "Mechanic", "Tramp", "MIB 1", "MIB 2", "MIB 3" };

void CUTSCENE_recreate(CSData* cutscene) {
	int channum, pktnum, image, who;
	CSChannel *chan;
	CSEditChannel *edit;
	CSPacket *pkt;
	CBYTE msg[600];

	chan=cutscene->channels;
	edit=cutscene->editchannels;
	for (channum=0;channum<cutscene->channelcount;channum++) {
		pkt=chan->packets;
		switch(chan->type) {
		case CT_CHAR:
			msg[0]=IM_SCENE_PERSON;
			if ((chan->index>0)&&(chan->index<16))
				strcpy(msg+1,PeopleStrings[chan->index-1]);
			else
				strcpy(msg+1,"Some Person");
			if ((chan->index>0)&&(chan->index<=PERSON_NUM_TYPES))
				who=chan->index-1;
			else
				who=-1;
/*			switch(chan->index) {
			  case 1: who=PERSON_DARCI; break;
			  case 2: who=PERSON_ROPER; break;
			  default: who=-1;
			}*/
			if (who>-1)
				edit->thing=CUTSCENE_create_person(who,64<<8,0,64<<8);
			else
				edit->thing=0;
			timeline->Add(msg);
			for(pktnum=0;pktnum<chan->packetcount;pkt++,pktnum++) {
				timeline->MarkEntry(channum,pkt->start,pkt->length,1);
			}
			break;
		case CT_CAM:
			msg[0]=IM_SCENE_CAMERA;
			strcpy(msg+1,"Camera");
			timeline->Add(msg);
			for(pktnum=0;pktnum<chan->packetcount;pkt++,pktnum++) {
				timeline->MarkEntry(channum,pkt->start,1,4); // always 1, and besides, ct_cam's length is used for Other Things(tm)
			}
			break;
		case CT_WAVE:
			msg[0]=IM_SCENE_SPEAKER;
			strcpy(msg+1,"Loudspeaker");
			timeline->Add(msg);
			for(pktnum=0;pktnum<chan->packetcount;pkt++,pktnum++) {
				timeline->MarkEntry(channum,pkt->start,1,4);
			}
			break;
		case CT_TEXT:
			msg[0]=IM_SCENE_BUBBLE;
			strcpy(msg+1,"Subtitles");
			timeline->Add(msg);
			for(pktnum=0;pktnum<chan->packetcount;pkt++,pktnum++) {
				timeline->MarkEntry(channum,pkt->start,pkt->length,4);
			}
			break;
		case CT_FX:
			msg[0]=(CBYTE)0xff;
			strcpy(msg+1,"FX");
			timeline->Add(msg);
			break;
		default:
			msg[0]=(CBYTE)0xff;
			strcpy(msg+1,"hell if i know");
			timeline->Add(msg);
			break;
		}
		chan++;
		edit++;
	}

	timeline->SetReadHead(0);
	timeline->Repaint();

}

void CUTSCENE_remove(CSData* cutscene) {
	int channum;
	CSChannel *chan;
	CSEditChannel *edit;

	chan=cutscene->channels;
	edit=cutscene->editchannels;
	for (channum=0;channum<cutscene->channelcount;channum++,chan++,edit++) {
		if (edit->thing) {
			THING_kill(edit->thing);
			edit->thing=0;
		}
	}
}



//-----------------------------------------------------


/*
SLONG	CUTSCENE_how_long_is_anim(SLONG who, SLONG anim)
{
	GameKeyFrame	*frame;
	SLONG	total=0;

	frame=global_anim_array[who][anim];


	while(frame)
	{
		SLONG	step;

		step=frame->TweenStep;
		if(!step)
			step=1;


		total+=256/step;
		if(frame->Flags&ANIM_FLAG_LAST_FRAME)
			break;

		frame	=	frame->NextFrame;
	}
	return(total);

}

*/



//---------------------------------------------------------------
// Object Inspector Handler
/*
void InitProps(PropertyEditor *pedit, UWORD type) {
	pedit->Clear();
	switch (type) {
	case 0:
/+		pedit->Add("Name","Unknown",PROPTYPE_STRING);
		pedit->Add("Data","Flirble",PROPTYPE_STRING);
		pedit->Add("Schlerb","1",PROPTYPE_INT);
		pedit->Add("booltest","off",PROPTYPE_BOOL);
		pedit->Add("multitst","one|two|three|four",PROPTYPE_MULTI);+/
//		pedit->Add("insanetst","one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|thirteen|fourteen|fifteen|sixteen|seventeen|eighteen|nineteen|twenty|twentyone|blah|wibble|moo|i|just|need|to|pad|this|out|as|much|asis|possible|la|le|lo|li|lu|mi|mo|mu|ma|me|mi|marp|peep|footle",PROPTYPE_MULTI);
		break;
	}
}
*/
SLONG	LoadAllAnimNames(CBYTE *fname, TreeBrowser *browser, SLONG base_ctr);

int GetItemIndex(CBYTE *str, CBYTE *pathtail) {
	int i;
	CBYTE combined[_MAX_PATH];

	strcpy(combined,pathtail);
	strcat(combined,str);

	i=0;
	while (strcmp(sound_list[i],"!")) {
		if (!stricmp(combined,sound_list[i])) {
			return i;
		}
		i++;
	}
	return 0;
}

int ScanWavs(TreeBrowser *browser, CBYTE *path, BOOL subdirs, HTREEITEM parent, UBYTE indent, SLONG param, SLONG img, SLONG imgfld, CBYTE *pathtail) {
	HANDLE handle;
	BOOL res;
	WIN32_FIND_DATA data;
	CBYTE *pt;
	int count=0, itemndx;

    handle = FindFirstFile(path, &data);

	res=(handle!=INVALID_HANDLE_VALUE);
	while (res) {
		if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (itemndx=GetItemIndex(data.cFileName,pathtail))
				browser->Add(data.cFileName,parent,indent,param+itemndx,img);
			count++;
		}
		res=FindNextFile(handle,&data);
	}
	FindClose(handle);

	if (!subdirs) return count;

	pt=strrchr(path,'\\');
	strcpy(pt,"\\*.*");

    handle = FindFirstFile(path, &data);

	res=(handle!=INVALID_HANDLE_VALUE);
	while (res) {
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)&&(data.cFileName[strlen(data.cFileName)-1]!='.')) {
			CBYTE path2[_MAX_PATH], wild[_MAX_PATH], combo[_MAX_PATH];
			int added;

			strcpy(path2,path);
			pt=strrchr(path2,'\\');
			strcpy(wild,pt); pt++;
			strcpy(pt,data.cFileName);
			strcat(path2,wild);
			browser->Add(data.cFileName,parent,indent,-1,imgfld);
			strcpy(combo,pathtail);
			strcat(combo,data.cFileName);
			strcat(combo,"\\");
			added=ScanWavs(browser,path2,true,0,indent+1,param,img,imgfld,combo);
			// param+=added;
			count+=added;
		}
		res=FindNextFile(handle,&data);
	}
	FindClose(handle);

	return count;


}

void InitBrowser(HWND hWnd) {
	int i;

	browser = new TreeBrowser(hWnd);
	browser->SetImageList(GEDIT_hinstance,IDB_SCENE_ICONS1);
	browser->Add("People",0,0,-1,IM_SCENE_FOLDER);
/*	browser->Add("Darci",0,1,1,IM_SCENE_PERSON);
	browser->Add("Roper",0,1,2,IM_SCENE_PERSON);
	browser->Add("Civilian",0,1,-1,IM_SCENE_FOLDER);
	browser->Add("Bald Guy",0,2,3,IM_SCENE_PERSON);
	browser->Add("Sports Casual Guy",0,2,4,IM_SCENE_PERSON);
	browser->Add("Thug",0,1,5,IM_SCENE_PERSON);*/

	for (i=0;i<PERSON_NUM_TYPES;i++)
		browser->Add(PeopleStrings[i],0,1,i+1,IM_SCENE_PERSON);

	browser->Add("Animations",0,0,-1,IM_SCENE_FOLDER);
	darcianim=browser->Add("Darci's",0,1,-1,IM_SCENE_FOLDER);
	LoadAllAnimNames("data\\darci1.anm",browser,0);
	roperanim=browser->Add("Roper's",0,1,-1,IM_SCENE_FOLDER);
	LoadAllAnimNames("data\\roper.anm",browser,1024);
//	browser->Add("Tasks",0,0,-1,IM_SCENE_FOLDER);
//	browser->Add("Go Here",0,1,8,IM_SCENE_ACTION);
	browser->Add("Tools",0,0,-1,IM_SCENE_FOLDER);
	browser->Add("Camera",0,1,9,IM_SCENE_CAMERA);
	browser->Add("Loudspeaker",0,1,10,IM_SCENE_SPEAKER);
	browser->Add("Subtitles",0,1,11,IM_SCENE_BUBBLE);
	soundbase=browser->Add("Sound FX",0,0,-1,IM_SCENE_FOLDER);
//	browser->AddDir("data\\sfx\\1622\\*.wav",true,0,1,2048,IM_SCENE_WAVE,IM_SCENE_FOLDER);
	ScanWavs(browser,"data\\sfx\\1622\\*.wav",true,0,1,2048,IM_SCENE_WAVE,IM_SCENE_FOLDER,"");
}

/*
VOID CALLBACK tf(HWND hWnd, UINT message, UINT idEvent, DWORD dwTime) {
//	PostMessage(hWnd, WM_USER, 0, 0);
	// evil. eeee ville!
	PostMessage(hWnd, WM_ENTERIDLE, 0, 0);
}
*/

//---------------------------------------------------------------
// yucky oh-god-i-have-to-deal-with-the-animation-filing-system-stuff

// all we want is the damn names!

#define	ANIM_NAME_SIZE	64


void SkipBodyPartInfo(MFFileHandle file_handle)
{
	SLONG	c0,c1;
	SLONG	no_people,no_body_bits,string_len;
	CBYTE	*dummy;
	UBYTE	dummy2;

	FileRead(file_handle,&no_people,sizeof(SLONG));

	FileRead(file_handle,&no_body_bits,sizeof(SLONG));

	FileRead(file_handle,&string_len,sizeof(SLONG));

	for(c0=0;c0<no_people;c0++)
	{
		dummy=(CBYTE*)malloc(string_len+1);
		FileRead(file_handle,dummy,string_len);
		free(dummy);
		for(c1=0;c1<no_body_bits;c1++)
			FileRead(file_handle,&dummy2,sizeof(UBYTE));
	}
}

void	LoadAnim(MFFileHandle file_handle, TreeBrowser *browser, SLONG num)
{
	CBYTE			anim_name[ANIM_NAME_SIZE];
	CBYTE			full_name[ANIM_NAME_SIZE+20];
	SLONG			c0,
					frame_count;
	SWORD			wdummy;
	CBYTE			version=0;
	SLONG			ldummy;


	FileRead(file_handle,&version,1);

	if(version==0||version>20)
	{
		anim_name[0]=version;
		FileRead(file_handle,&anim_name[1],ANIM_NAME_SIZE-1);
		version=0;
	}
	else
		FileRead(file_handle,anim_name,ANIM_NAME_SIZE);

	if ((global_anim_array[0][num]==global_anim_array[0][0])||!global_anim_array[0][num]) {
		sprintf(full_name,"%d: (%s disabled)",num&1023,anim_name);
		browser->Add(full_name,0,2,0,IM_SCENE_WARNING);
	} else {
		sprintf(full_name,"%d: %s",num&1023,anim_name);
		browser->Add(full_name,0,2,num,IM_SCENE_ANIM);
	}

/*//	if (!((global_anim_array[0][num]==(void*)0xe4bf8e8c)||!global_anim_array[0][num])) {
	if (!((global_anim_array[0][num]==global_anim_array[0][0])||!global_anim_array[0][num])) {
		sprintf(full_name,"%d: %s",num&1023,anim_name);
		browser->Add(full_name,0,2,num,IM_SCENE_ANIM);
	}*/


	FileRead(file_handle,&ldummy,sizeof(ldummy));
	FileRead(file_handle,&frame_count,sizeof(frame_count));
	if(version>3) FileRead(file_handle,&wdummy,1);

	for(c0=0;c0<frame_count;c0++)
	{
		FileRead(file_handle,&wdummy,sizeof(wdummy));
		FileRead(file_handle,&ldummy,sizeof(ldummy));
		FileRead(file_handle,&ldummy,sizeof(ldummy));
		if(version>0) FileRead(file_handle,&wdummy,sizeof(wdummy));

		if(version>1)
		{
			struct	FightCol	fcol;
			SLONG	count,c0;

			FileRead(file_handle,&count,sizeof(count));

			for(c0=0;c0<count;c0++)
				FileRead(file_handle,&fcol,sizeof(struct FightCol));
		}
	}
}

SLONG	LoadAllAnimNames(CBYTE *fname, TreeBrowser *browser, SLONG base_ctr)
{
	SLONG			anim_count,version,
					c0;
	MFFileHandle	file_handle;


	file_handle	=	FileOpen(fname);
	if(file_handle!=FILE_OPEN_ERROR)
	{
		FileRead(file_handle,&anim_count,sizeof(anim_count));
		if(anim_count<0)
		{
			version=anim_count;
			FileRead(file_handle,&anim_count,sizeof(anim_count));

			SkipBodyPartInfo(file_handle);

		}

		for(c0=0;c0<anim_count;c0++)
			LoadAnim(file_handle,browser,c0+base_ctr);
		return anim_count;
	}
	else
	{
		// Unable to open file.
		return 0;
	}
}


//---------------------------------------------------------------
// Main Editor Proc

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
extern void ClearLatchedKeys();

void new_calc_camera_pos() {

//	cam_focus_dist=1;

	FMATRIX_calc(
					cam_matrix,
					cam_yaw,
					cam_pitch,
					0
				);

	cam_x	=	cam_focus_x;
	cam_y	=	cam_focus_y; // PAP_calc_height_at(LEDIT_cam_focus_x, LEDIT_cam_focus_z) + 0x100;
	cam_z	=	cam_focus_z;

/*	cam_x	-=	MUL64(cam_matrix[6], cam_focus_dist);
	cam_y	-=	MUL64(cam_matrix[7], cam_focus_dist);
	cam_z	-=	MUL64(cam_matrix[8], cam_focus_dist);*/

	FMATRIX_vector	(
						cam_forward,
						cam_yaw,
						0
					);

	FMATRIX_vector	(
						cam_left,
						(cam_yaw + 512) & 2047,
						0
					);
}

void MouselookToggle() {
	CUTSCENE_mouselook^=1;
	if (CUTSCENE_mouselook)
	{
		SetCursorPos(320,240);
		SetCapture(CUTSCENE_edit_wnd);
		ShowCursor(FALSE);
	} else {
		ReleaseCapture();
		ShowCursor(TRUE);
	}
}

int get_index_from_string(CBYTE *string, CBYTE *opts) {
	CBYTE *pt,*buff;
	int   i=0, r=-1;

	buff=(CBYTE*)malloc(strlen(opts)+1);
	strcpy(buff,opts);
	opts=buff; // now we can play with opts without destroying the original
	while (pt=strchr(opts,'|')) {
		*pt=0;
		pt++;
		if (!stricmp(string,opts)) r=i;
		opts=pt;
		i++;
	}
	if (!stricmp(string,opts)) r=i;
	free(buff);
	return r;

}

CBYTE* get_string_from_index(int index, CBYTE *opts, CBYTE *result) {
	CBYTE *pt=opts;

	while (index--) {
	  pt=strchr(pt,'|');
	  if (!pt) { *result=0; return result; }
	  pt++;
	}

	strcpy(result,pt);
	pt=strchr(result,'|');
	if (pt) *pt=0;
	return result;
}


int	CUTSCENE_get_chan_from_item(CSData *cutscene, Thing *dragitem) {
	int channum;
	CSEditChannel* chan=cutscene->editchannels;
	for (channum=0;channum<cutscene->channelcount;channum++,chan++)
		if (chan->thing==dragitem) return channum;
	return -1;
}

void CUTSCENE_match_selection_to_item(CSData *cutscene, Thing *dragitem) {
	int chan=CUTSCENE_get_chan_from_item(cutscene,dragitem);
	SendMessage(GetDlgItem(CUTSCENE_edit_wnd,IDC_LIST2),LB_SETCURSEL,chan,0);
	current_packet=CUTSCENE_get_packet(cutscene->channels+chan,timeline->GetReadHead());
}

Thing* CUTSCENE_item_from_point(CSData *cutscene, SLONG x, SLONG y, SLONG z) {
	SLONG chanctr, packctr;
	CSChannel *chan;
//	CSPacket *pack;
	CSEditChannel *edit;
	chan=cutscene->channels;
	edit=cutscene->editchannels;
	for (chanctr=0;chanctr<cutscene->channelcount;chanctr++) {
		if (chan->type==CT_CHAR) {
			if ((abs(x-(edit->thing->WorldPos.X>>8))<50)&&(abs(z-(edit->thing->WorldPos.Z>>8))<50)) return edit->thing;
		}
		chan++; edit++;
	}
	return 0;
}

BOOL CUTSCENE_find_surrounding_packets(CSChannel *chan, int cell, int* left, int* right) {
	int leftmax=-1, rightmin=2001, packctr=0;
	CSPacket *pack;
	*left=-1; *right=2001;
	pack=chan->packets;
	for (packctr=0;packctr<chan->packetcount;packctr++,pack++) {
		if ((pack->start<=cell)&&(pack->start>leftmax)) {
			leftmax=pack->start; *left=packctr;
		}
		if ((pack->start>=cell)&&(pack->start<rightmin)) {
			rightmin=pack->start; *right=packctr;
		}
	}
	return ((leftmax>-1)&&(rightmin<2000));
}

extern SLONG	person_normal_animate(Thing *p_person);

inline int  LERPValue(int a, int b, int m) {
	return a+(((b-a)*m)/256);
}

inline int  LERPAngle(int a, int b, int m) {
	if ((a-b)>1024) b+=2048;
	if ((b-a)>1024) a+=2048;
	return a+(((b-a)*m)/256);
}

void LERPAnim(CSChannel *chan, Thing *person, int cell) {
	int left,right,lastpos,pos,dist, a, mult, smoothmult, smoothin, smoothout;
	int x,y,z;
	CSPacket *pktA, *pktB;
	GameCoord posn;

	CUTSCENE_find_surrounding_packets(chan,cell,&left,&right);
	if (left<0) return;
	pktA=chan->packets+left;
	pos=cell-pktA->start;

	// interpolation
	if ((right<2001)&&(right!=left)) {
		pktB=chan->packets+right;
		dist=pktB->start-pktA->start;
//		mult=(pos*256)/dist;
		mult=(pos*256);
		if (CUTSCENE_slomo) {
			mult+=((SLOMO_RATE-CUTSCENE_slomo_ctr)*256)/SLOMO_RATE;
		}
		mult/=dist;


		if (pktA->flags&(PF_SMOOTH_MOVE_BOTH|PF_SMOOTH_ROT_BOTH)) {
			// this little fiddle should smooth in/out the edges
			smoothmult=(mult-128)*4;
			smoothmult=(SIN(smoothmult&2047)>>8)+256;
			smoothmult>>=1;
			smoothin=(mult-256)*2;
			smoothin=256+(SIN(smoothin&2047)>>8);
			smoothout=SIN(mult*2)>>8;
		}

		// motion:
		if (pktA->flags&PF_INTERPOLATE_MOVE) {
			if ((pktA->flags&PF_SMOOTH_MOVE_BOTH)==PF_SMOOTH_MOVE_BOTH) {
/*				x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*smoothmult)/256);
				y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*smoothmult)/256);
				z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*smoothmult)/256);*/
				x=LERPValue(pktA->pos.X,pktB->pos.X,smoothmult);
				y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothmult);
				z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothmult);
			} else
				if (pktA->flags&PF_SMOOTH_MOVE_IN) {
/*					x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*smoothin)/256);
					y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*smoothin)/256);
					z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*smoothin)/256);*/
					x=LERPValue(pktA->pos.X,pktB->pos.X,smoothin);
					y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothin);
					z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothin);
				} else
					if (pktA->flags&PF_SMOOTH_MOVE_OUT) {
/*						x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*smoothout)/256);
						y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*smoothout)/256);
						z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*smoothout)/256);*/
						x=LERPValue(pktA->pos.X,pktB->pos.X,smoothout);
						y=LERPValue(pktA->pos.Y,pktB->pos.Y,smoothout);
						z=LERPValue(pktA->pos.Z,pktB->pos.Z,smoothout);
					} else { // linear
/*						x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*mult)/256);
						y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*mult)/256);
						z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*mult)/256);*/
						x=LERPValue(pktA->pos.X,pktB->pos.X,mult);
						y=LERPValue(pktA->pos.Y,pktB->pos.Y,mult);
						z=LERPValue(pktA->pos.Z,pktB->pos.Z,mult);
					}
		} else {
			x=pktA->pos.X;
			y=pktA->pos.Y;
			z=pktA->pos.Z;
		}

		// er... later.

		// rotation:
		if (pktA->flags&PF_INTERPOLATE_ROT) {
			if ((pktA->flags&PF_SMOOTH_ROT_BOTH)==PF_SMOOTH_ROT_BOTH) {
				mult=smoothmult;
			} else
				if (pktA->flags&PF_SMOOTH_ROT_IN) {
					mult=smoothin;
				} else
					if (pktA->flags&PF_SMOOTH_ROT_OUT) {
						mult=smoothout;
					} // else mult is already correct for linear
			a=LERPAngle(pktA->angle,pktB->angle,mult);
		} else {
			a=pktA->angle;
		}
		person->Draw.Tweened->Angle=a;
		posn.X=x; posn.Y=y; posn.Z=z;
		move_thing_on_map(person,&posn);

	}


	set_anim(person,pktA->index);
	if (pktA->flags&1) // reverse
		pos=(pktA->length-pos)-1;
	if (pos<=0) return;
/*	TICK_RATIO=1<<TICK_SHIFT;
	while (pos--) person_normal_animate(person);*/
	TICK_RATIO=pos<<TICK_SHIFT;
	if (CUTSCENE_slomo) {
		TICK_RATIO+=((SLOMO_RATE-CUTSCENE_slomo_ctr)<<TICK_SHIFT)/SLOMO_RATE;
	}
	person_normal_animate(person);


}

void LERPCamera(CSChannel *chan, int cell) {
	int x,y,z,p,a, left, right, dist, pos, mult, smoothin, smoothout, smoothmult,lens;
	CSPacket *pktA,*pktB;

	if (!CUTSCENE_find_surrounding_packets(chan, cell,&left,&right)) return;

	pktA=chan->packets+left;
	pktB=chan->packets+right;
	dist=pktB->start-pktA->start;
	pos=cell-pktA->start;

	mult=(pos*256);
	if (CUTSCENE_slomo) {
		mult+=((SLOMO_RATE-CUTSCENE_slomo_ctr)*256)/SLOMO_RATE;
//		mult+=(SLOMO_RATE-CUTSCENE_slomo_ctr)/SLOMO_RATE;
	}
	mult/=dist;

	if (pktA->flags&(PF_SMOOTH_MOVE_BOTH|PF_SMOOTH_ROT_BOTH)) {
		// this little fiddle should smooth in/out the edges
		smoothmult=(mult-128)*4;
		smoothmult=(SIN(smoothmult&2047)>>8)+256;
		smoothmult>>=1;
		smoothin=(mult-256)*2;
		smoothin=256+(SIN(smoothin&2047)>>8);
		smoothout=SIN(mult*2)>>8;
	}

	if (pktA->flags&PF_INTERPOLATE_MOVE) {
		if ((pktA->flags&PF_SMOOTH_MOVE_BOTH)==PF_SMOOTH_MOVE_BOTH) {
			x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*smoothmult)/256);
			y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*smoothmult)/256);
			z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*smoothmult)/256);
			lens=(pktA->length&0xff)+((((pktB->length&0xff)-(pktA->length&0xff))*smoothmult)/256);
		} else
			if (pktA->flags&PF_SMOOTH_MOVE_IN) {
				x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*smoothin)/256);
				y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*smoothin)/256);
				z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*smoothin)/256);
				lens=(pktA->length&0xff)+((((pktB->length&0xff)-(pktA->length&0xff))*smoothin)/256);
			} else
				if (pktA->flags&PF_SMOOTH_MOVE_OUT) {
					x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*smoothout)/256);
					y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*smoothout)/256);
					z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*smoothout)/256);
					lens=(pktA->length&0xff)+((((pktB->length&0xff)-(pktA->length&0xff))*smoothout)/256);
				} else { // linear
					x=pktA->pos.X+(((pktB->pos.X-pktA->pos.X)*mult)/256);
					y=pktA->pos.Y+(((pktB->pos.Y-pktA->pos.Y)*mult)/256);
					z=pktA->pos.Z+(((pktB->pos.Z-pktA->pos.Z)*mult)/256);
					lens=(pktA->length&0xff)+((((pktB->length&0xff)-(pktA->length&0xff))*mult)/256);
				}
	} else {
		x=pktA->pos.X;
		y=pktA->pos.Y;
		z=pktA->pos.Z;
		lens=pktA->length&0xff;
	}
	lens=((lens-0x7f)*1.5)+0xff;
	FC_cam[0].lens=(lens*0x24000)>>8;

	if (pktA->flags&PF_INTERPOLATE_ROT) {
		if ((pktA->flags&PF_SMOOTH_ROT_BOTH)==PF_SMOOTH_ROT_BOTH) {
			mult=smoothmult;
		} else
			if (pktA->flags&PF_SMOOTH_ROT_IN) {
				mult=smoothin;
			} else
				if (pktA->flags&PF_SMOOTH_ROT_OUT) {
					mult=smoothout;
				} // else mult is already correct for linear
		p=LERPAngle(pktA->pitch,pktB->pitch,mult);
		a=LERPAngle(pktA->angle,pktB->angle,mult);
	} else {
		p=pktA->pitch;
		a=pktA->angle;
	}

	cam_focus_x=x;
	cam_focus_y=y;
	cam_focus_z=z;
	cam_pitch=p;
	cam_yaw=a;
	CUTSCENE_fade_level=LERPValue(pktA->length>>8,pktB->length>>8,mult);
}

void SnapAnimation() {
	int left,right,channum,cell;
	CSChannel *chan;
	CSEditChannel *edit;
	CSPacket *pktA;
	GameCoord posA, posB;

	channum=timeline->GetSelectedRow();
	cell=timeline->GetReadHead();
	chan=cutscene->channels+channum;
	edit=cutscene->editchannels+channum;

	if (!edit->thing) return;

	CUTSCENE_find_surrounding_packets(chan,cell-1,&left,&right);

	if (left<0) { MessageBeep(0); return; }
	pktA=chan->packets+left;

	LERPAnim(chan, edit->thing, pktA->start+pktA->length-1);
	calc_sub_objects_position(
		edit->thing, edit->thing->Draw.Tweened->AnimTween, SUB_OBJECT_LEFT_FOOT,
		&posA.X, &posA.Y, &posA.Z);
	posA.X<<=8; posA.Y<<=8; posA.Z<<=8;
	posA.X+=pktA->pos.X; posA.Y+=pktA->pos.Y; posA.Z+=pktA->pos.Z;

	// posA now abs-world-coord pos of foot

	LERPAnim(chan, edit->thing, cell);

	calc_sub_objects_position(
		edit->thing, edit->thing->Draw.Tweened->AnimTween, SUB_OBJECT_LEFT_FOOT,
		&posB.X, &posB.Y, &posB.Z);

	posB.X<<=8; posB.Y<<=8; posB.Z<<=8;

	current_packet->pos.X=posA.X-posB.X;
	current_packet->pos.Y=posA.Y-posB.Y;
	current_packet->pos.Z=posA.Z-posB.Z;
	move_thing_on_map(edit->thing,&current_packet->pos);


	/*
		track_x<<=8;
		track_y<<=8;
		track_z<<=8;
		track_x+=p_person->WorldPos.X;
		track_z+=p_person->WorldPos.Z;
		track_y=(PAP_calc_height_at_thing(p_person,track_x>>8,track_z>>8)<<8)+0x180;
	*/

}

void DoEraseChan(int channum) {
	CBYTE msg[255],buf[255];
	timeline->GetText(channum,buf);

	sprintf(msg,"Are you sure you want to permanently delete this %s track?",buf);
	if (MessageBox(CUTSCENE_edit_wnd,msg,"Confirm: Delete Track",MB_ICONEXCLAMATION|MB_OKCANCEL)==IDOK) {
		CUTSCENE_chan_delete(cutscene,channum);
		timeline->Del(channum);
		current_packet=0;
		timeline->SetReadHead(timeline->GetReadHead());
	}
}

void DoErase(int do_row=-1, int do_cell=-1) {
	CSChannel* chan=0;
	CSPacket* pkt=0;
	int pack,length;

	if ((do_row<0)||(do_cell<0)) {
		do_row=timeline->GetSelectedRow();
		if (!current_packet) {
			if (do_row>-1) DoEraseChan(do_row);
			return;
		}
		do_cell=current_packet->start;
		chan=cutscene->channels+do_row;
		pkt=current_packet;
	} else {
		chan=cutscene->channels+do_row;
		pkt=CUTSCENE_get_packet(chan,do_cell);
	}
	if (!pkt) return;

	pack=pkt-chan->packets;
	length=(pkt->type!=PT_CAM) ? pkt->length : 1;
	timeline->MarkEntry(do_row,do_cell,length,0xff);
	timeline->Repaint();
	CUTSCENE_del_packet(chan, pack);
	timeline->SetReadHead(timeline->GetReadHead());

/*
	if (current_packet) {
		int channum=timeline->GetSelectedRow();
		CSChannel* chan=cutscene->channels+channum;
		int pack=current_packet-chan->packets;
		int packx=current_packet->start;
		timeline->MarkEntry(channum,packx,current_packet->length,0xff);
		timeline->Repaint();
		CUTSCENE_del_packet(chan, pack);
		timeline->SetReadHead(packx);
	}
*/

}

void DoHandleShit() {
	HWND handle=GetDlgItem(CUTSCENE_edit_wnd,IDC_MAP_VIEW);
	PAINTSTRUCT		ps;
	HDC	 hdc	=	BeginPaint(handle,&ps);
	POINT client_pos;
	RECT src,dst;
	HRESULT result;
	SWORD speed;
	static int lasttick=0;
	SLONG ofsX=0, ofsY=0, ofsZ=0;
	SLONG matrix[9];
	int tick=GetTickCount();


	if (tick-lasttick>40) {
		lasttick=tick;
		if (CUTSCENE_playback) {
			int rh=timeline->GetReadHead();
			if (rh==2000) {
				rh=0; CUTSCENE_playback=0;
			} else {
				if (CUTSCENE_slomo) {
					if (!(--CUTSCENE_slomo_ctr)) {
						CUTSCENE_slomo_ctr=SLOMO_RATE;
						rh++;
					}
				} else rh++;
			}
			timeline->SetReadHead(rh);
		}
	}

	speed=(GetKeyState(VK_CAPITAL)&1)?128:64;
	if (GetAsyncKeyState(VK_CONTROL)&(1<<15)) speed=8;
	if (Keys[KB_LEFT])	ofsX= speed;
	if (Keys[KB_RIGHT])	ofsX=-speed;
	if (Keys[KB_UP])	ofsZ= speed;
	if (Keys[KB_DOWN])	ofsZ=-speed;
	if (Keys[KB_DEL])	cam_yaw+=10;
	if (Keys[KB_PGDN])	cam_yaw-=10;
	if (Keys[KB_HOME])  cam_focus_y+=speed;
	if (Keys[KB_END])   cam_focus_y-=speed;

	FMATRIX_calc(matrix,cam_yaw&2047,cam_pitch&2047,0);
	FMATRIX_TRANSPOSE(matrix);
	FMATRIX_MUL(matrix, ofsX, ofsY, ofsZ);

	cam_focus_x+=ofsX;
	if (GetKeyState(VK_SCROLL)&1) cam_focus_y+=ofsY;
	cam_focus_z+=ofsZ;

	cam_yaw   &= 2047;
	cam_pitch &= 2047;

	new_calc_camera_pos();
	// Draw the engine.
	MFX_set_listener(cam_x<<8,cam_y<<8,cam_z<<8,1024+cam_yaw,-1024,cam_pitch-1024);
	GI_render_view_into_backbuffer	(   cam_x,
										cam_y,
										cam_z,
										cam_yaw,
										cam_pitch,
										0 );
	CONSOLE_draw();

	if (subtitle_str[0]) {
//		the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);
		POLY_frame_init(FALSE, FALSE);
//		MENUFONT_Draw(320,400,256,subtitle_str,0x7fffffff,MENUFONT_CENTRED);
		FONT2D_DrawStringCentred(subtitle_str,320,400,0x7fffffff,16,POLY_PAGE_FONT2D);
		POLY_frame_draw(FALSE, FALSE);
	}

	if (CUTSCENE_fade_level<255) {
		//the_display.lp_D3D_Viewport->Clear(1, &the_display.ViewportRect, D3DCLEAR_ZBUFFER);
		POLY_frame_init(FALSE, FALSE);
		DRAW2D_Box(0, 0, 640, 480, (255-CUTSCENE_fade_level)<<24, 1, 255);
		POLY_frame_draw(FALSE, FALSE);
	}

	client_pos.x	=	0;
	client_pos.y	=	0;
	ClientToScreen(handle,&client_pos);

	GetClientRect(handle,&src);
	dst	=	src;
	OffsetRect(&dst,client_pos.x,client_pos.y);

	//	Set the clipper.
	result	=	the_display.lp_DD_Clipper->SetHWnd(0,handle);

	//	Blit the view.
	result	=	the_display.lp_DD_FrontSurface->Blt(&dst,the_display.lp_DD_BackSurface,&src,DDBLT_WAIT,0);

	EndPaint(handle,&ps);
}


BOOL browserCB(TreeBrowser *tb, int reason, int index, HTREEITEM item, char *str) {
	int image=tb->GetImageFromItem(item);
	CSChannel *chan;
	CSEditChannel *echan;
	SLONG x,y,z,who;

	switch (reason) {
	case TBCB_DBLCLK:
		char msg[800];
		if ((image==IM_SCENE_SPEAKER)||(image==IM_SCENE_PERSON)||(image==IM_SCENE_CAMERA)||(image==IM_SCENE_BUBBLE)) {
			msg[0]=image;
			strcpy(msg+1,str);
			timeline->Add(msg);
			timeline->Repaint();
			chan=CUTSCENE_add_channel(cutscene);
			echan=cutscene->editchannels+(chan-cutscene->channels);
			GI_get_pixel_world_pos(320,240,&x,&y,&z,0);
/*			x=cam_focus_x; z=cam_focus_z;
			y=PAP_calc_map_height_at(x,z);*/
			switch(image) {
			case IM_SCENE_PERSON:
				chan->type=CT_CHAR;
				chan->index=index;
				if ((index>0)&&(index<=PERSON_NUM_TYPES))
					who=index-1;
				else
					who=-1;
/*				switch(index) {
				case 1: who=PERSON_DARCI; break;
				case 2: who=PERSON_ROPER; break;
				default: who=-1;
				}*/
				if (who>-1) echan->thing=CUTSCENE_create_person(who,x,y,z);
				break;
			case IM_SCENE_CAMERA:  chan->type=CT_CAM;  break;
			case IM_SCENE_SPEAKER: chan->type=CT_WAVE; break;
			case IM_SCENE_BUBBLE:  chan->type=CT_TEXT; break;
			}
		}
		return 0;
	case TBCB_DRAG:
		if ((image==IM_SCENE_ANIM)&&!index) return 0;
		return ((image==IM_SCENE_WAVE)||(image==IM_SCENE_ACTION)||(image==IM_SCENE_ANIM));
		break;
	default:
		return 0;
	}

}

void Update3DStuff(CSChannel *chan, CSEditChannel *edit, int cell) {
	CSPacket  *pkt;
	int index,lens;

	pkt=CUTSCENE_get_packet(chan,cell);
	if (!pkt) {
		if (chan->type==CT_CHAR) LERPAnim(chan,edit->thing,cell);
		if ((chan->type==CT_CAM)&&!(GetAsyncKeyState(VK_CONTROL)&(1<<15))) LERPCamera(chan,cell);
		return;
	}
	switch(pkt->type) {
		case PT_ANIM:
			index=pkt->index;
			while (index>1023) { index-=1024; }
			if (edit->thing) {
				//edit->thing->WorldPos=pkt->pos;
				move_thing_on_map(edit->thing,&pkt->pos);
				edit->thing->Draw.Tweened->Angle=pkt->angle;
				set_anim(edit->thing,index);
				if (pkt->flags&1) LERPAnim(chan,edit->thing,cell); // should handle the reversing
			}
			break;

		case PT_CAM:
		    if (!(GetAsyncKeyState(VK_CONTROL)&(1<<15))) {
				cam_focus_x=pkt->pos.X;
				cam_focus_y=pkt->pos.Y;
				cam_focus_z=pkt->pos.Z;
				cam_pitch=pkt->pitch;
				cam_yaw=pkt->angle;
				lens=((pkt->length&0xff)-0x7f);
				lens=(lens*1.5)+0xff;
				FC_cam[0].lens=(lens*0x24000)>>8;
				PolyPage::SetGreenScreen(pkt->flags&PF_SECURICAM);
				CUTSCENE_slomo=pkt->flags&PF_SLOMO;
				CUTSCENE_fade_level=pkt->length>>8;
			}
			break;

		case PT_WAVE:
			  if (CUTSCENE_playback) MFX_play_xyz(MUSIC_REF+(chan-cutscene->channels),pkt->index-2048,0,pkt->pos.X<<8,pkt->pos.Y<<8,pkt->pos.Z<<8);
			break;

		case PT_TEXT:
			if (pkt->pos.X)
			  strcpy(subtitle_str,(CBYTE*)pkt->pos.X);
			else
			  subtitle_str[0]=0;
			break;
	}
}

int LerpModeMangle(int selection, int which) {
	int res=0;
	if (selection>0) res|=1;
	if ((selection==2)||(selection==4)) res|=2;
	if (selection>2) res|=4;
	if (which==1) res<<=1;
	if (which==2) res<<=4;
	return res;
}

int	UnMangleLerp(int flags, int which) {
	if (which==1) flags>>=1;
	if (which==2) flags>>=4;
	flags&=(1|2|4);
	switch (flags) {
	case 0: case 2: case 4: case 6: default: return 0;
	case 1: return 1;
	case 3: return 2;
	case 5: return 3;
	case 7: return 4;
	}
}

BOOL timelineCB(TimeLine *tb, int reason, int index, int subline, int cell) {
	CSChannel *chan;
	CSEditChannel *edit;
	CSPacket  *pkt;
	int channum;
	CBYTE str[255];

	switch(reason) {
/*	case TLCB_GETBARINFO:
		chan=&cutscene->Channels[index];

		break;*/
	case TLCB_SELECT:
		for (channum=0;channum<cutscene->channelcount;channum++)
			if (channum!=index) // we need to update the 3d display, but not the property window
			{
				chan=cutscene->channels+channum;
				edit=cutscene->editchannels+channum;
				Update3DStuff(chan,edit,cell);
			}
		if (index<0) return 0;
		chan=cutscene->channels+index;
		edit=cutscene->editchannels+index;
		current_packet=pkt=CUTSCENE_get_packet(chan,cell);
		pedit->Clear();
		Update3DStuff(chan,edit,cell);
		if (!pkt) return 0;
	/*	if (!pkt) {
			if (chan->type==CT_CHAR) LERPAnim(chan,edit->thing,cell);
			if ((chan->type==CT_CAM)&&!ControlFlag) LERPCamera(chan,cell);
			return 0;
		}*/
		switch(pkt->type) {
		  case PT_ANIM:
			  {
				  HTREEITEM base=darcianim;
				  int		index=pkt->index;
				  if (index>1023) { base=roperanim; index-=1024; }
				  browser->GetTextFromItem(browser->GetChildFromItem(base,index),str,255);
/*				  if (edit->thing) {
					  edit->thing->WorldPos=pkt->pos;
					  edit->thing->Draw.Tweened->Angle=pkt->angle;
					  set_anim(edit->thing,index);
				  }*/
			  }
			  pedit->Add("Type","Animation",PROPTYPE_READONLY);
			  pedit->Add("Anim",str,PROPTYPE_READONLY);
			  pedit->Add("Snap position","(click here)",PROPTYPE_BUTTON);
			  pedit->Add("Play direction","forwards|backwards",PROPTYPE_MULTI);
			  if (pkt->flags&1) pedit->Update(3,"backwards");
			  pedit->Add("Movement",interpolate_strings,PROPTYPE_MULTI);
			  pedit->Add("Rotation",interpolate_strings,PROPTYPE_MULTI);
			  pedit->Update(4,get_string_from_index(UnMangleLerp(pkt->flags,1),interpolate_strings,str));
			  pedit->Update(5,get_string_from_index(UnMangleLerp(pkt->flags,2),interpolate_strings,str));

			  break;
		  case PT_CAM:
			  pedit->Add("Type","Camera",PROPTYPE_READONLY);
			  pedit->Add("Cam mode","normal|security camera",PROPTYPE_MULTI);
			  if (pkt->flags&PF_SECURICAM) pedit->Update(1,"security camera");
			  pedit->Add("Movement",interpolate_strings,PROPTYPE_MULTI);
			  pedit->Add("Rotation",interpolate_strings,PROPTYPE_MULTI);
			  pedit->Update(2,get_string_from_index(UnMangleLerp(pkt->flags,1),interpolate_strings,str));
			  pedit->Update(3,get_string_from_index(UnMangleLerp(pkt->flags,2),interpolate_strings,str));
			  itoa(pkt->length>>8,str,10);
			  pedit->Add("Fade in",str,PROPTYPE_INT);
			  itoa((pkt->length&0xff)-0x7f,str,10);
			  pedit->Add("Zoom ratio",str,PROPTYPE_INT);
			  pedit->Add("Slo-mo","off",PROPTYPE_BOOL);
			  if (pkt->flags&PF_SLOMO) pedit->Update(6,"on");
			  CUTSCENE_fade_level=pkt->length>>8;
			  break;
		  case PT_WAVE:
			  pedit->Add("Type","Wave",PROPTYPE_READONLY);
			  pedit->Add("Wave",sound_list[pkt->index-2048],PROPTYPE_READONLY);
			  pedit->Add("Preview","(click here)",PROPTYPE_BUTTON);
			  //if (CUTSCENE_playback) MFX_play_xyz(MUSIC_REF+channum,pkt->index-2048,0,pkt->pos.X,pkt->pos.Y,pkt->pos.Z);
			  break;
		  case PT_TEXT:
			  pedit->Add("Type","Subtitle",PROPTYPE_READONLY);
			  if (pkt->pos.X)
				  strcpy(subtitle_str,(CBYTE*)pkt->pos.X);
			  else
				  subtitle_str[0]=0;
			  pedit->Add("Text",subtitle_str,PROPTYPE_STRING);
			  //pedit->Add("Style","normal|something else",PROPTYPE_MULTI);
			  break;
		}
		break;
	}
	return 0;
}


BOOL propeditCB(PropertyEditor *tb, int reason, int index, CBYTE *value) {
	int res,i;

	switch(reason) {
	case PECB_EDITMODE:
		CUTSCENE_need_keyboard=index;
		break;
	case PECB_UPDATE:
		if (current_packet) {
			switch(current_packet->type) {
			case PT_CAM:
				switch(index) {
				case 1: // normal/securitycam
					current_packet->flags&=~1;
					current_packet->flags|=(*value=='s');
					PolyPage::SetGreenScreen(current_packet->flags&1);
					break;
				case 2: // motion mode
					current_packet->flags&=~(2|4|8);
					//current_packet->flags|=(*value=='s')<<1;
					res=get_index_from_string(value,interpolate_strings);
					current_packet->flags|=LerpModeMangle(res,1);
					break;
				case 3: // linear/smooth rotation
					current_packet->flags&=~(16|32|64);
					//current_packet->flags|=(*value=='s')<<2;
					res=get_index_from_string(value,interpolate_strings);
					current_packet->flags|=LerpModeMangle(res,2);
					break;
				case 4: // Fade
					res=i=atoi(value);
					SATURATE(res,0,255);
					if (res!=i) {
						itoa(res,value,10);
						pedit->Update(4,value);
					}
					current_packet->length&=0xff;
					current_packet->length|=(res<<8);
					break;
				case 5: // Zoom
					res=i=atoi(value);
					SATURATE(res,-127,128);
					if (res!=i) {
						itoa(res,value,10);
						pedit->Update(5,value);
					}
					current_packet->length&=0xff00;
					current_packet->length|=res+127;
//					i=(current_packet->length&0xff);
					i=(res*1.5)+0xff;
					FC_cam[0].lens=(i*0x24000)>>8;
					break;
				case 6: // Slo-mo
					current_packet->flags&=~PF_SLOMO;
					if (!stricmp("on",value)) current_packet->flags|=PF_SLOMO;
					CUTSCENE_slomo=current_packet->flags&PF_SLOMO;
					break;
				}
				break;
			case PT_ANIM:
				switch(index) {
				case 3:
					current_packet->flags&=~1;
					current_packet->flags|=(*value=='b');
					break;
				case 4: // motion mode
					current_packet->flags&=~(2|4|8);
					res=get_index_from_string(value,interpolate_strings);
					current_packet->flags|=LerpModeMangle(res,1);
					break;
				case 5: // linear/smooth rotation
					current_packet->flags&=~(16|32|64);
					res=get_index_from_string(value,interpolate_strings);
					current_packet->flags|=LerpModeMangle(res,2);
					break;
				}
				break;
			case PT_TEXT:
				switch(index) {
				case 1:
					free((CBYTE*)current_packet->pos.X);
					current_packet->pos.X = (int)malloc(strlen(value)+1);
					strcpy((CBYTE*)current_packet->pos.X,value);
				}
				break;
			}
		}
		break;
	case PECB_BUTTON:
		if (current_packet) {
			switch(current_packet->type) {
			case PT_ANIM:
				if (index==2) SnapAnimation();
				break;
			case PT_WAVE:
				if (index==2)
					//MFX_play_stereo(MUSIC_REF,current_packet->index-2048,0);
					MFX_play_xyz(MUSIC_REF,current_packet->index-2048,0,cam_x<<8,cam_y<<8,cam_z<<8);
				break;
			}
		}
	}
	return FALSE;
}

LRESULT	CALLBACK	scene_map_view_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	static Thing *dragitem=NULL;
	static int dragmode=0;
	SLONG wx,wy,wz,dx,dz, angle;

	switch(message)
	{
		case	WM_LBUTTONDOWN:
		{
			UWORD sx=LOWORD(lParam);
			UWORD sy=HIWORD(lParam);
			if (GI_get_pixel_world_pos(sx,sy,&wx,&wy,&wz,0)) {
				Thing *dragtest=CUTSCENE_item_from_point(cutscene,wx,wy,wz);
				if (dragtest) {
					dragitem=dragtest;
					SetCapture(hWnd);
					CUTSCENE_match_selection_to_item(cutscene,dragitem);
					dragmode=1;
				}
			}
		}
		break;
		case	WM_RBUTTONDOWN:
		{
			UWORD sx=LOWORD(lParam);
			UWORD sy=HIWORD(lParam);
			if (GI_get_pixel_world_pos(sx,sy,&wx,&wy,&wz,0)) {
				Thing *dragtest=CUTSCENE_item_from_point(cutscene,wx,wy,wz);
				if (dragtest) {
					dragitem=dragtest;
					CUTSCENE_match_selection_to_item(cutscene,dragitem);
					SetCapture(hWnd);
					dragmode=2;
				}
			}
		}
		break;
		case	WM_MOUSEMOVE:
			if (dragitem) {
				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hWnd,&pt);
				GI_get_pixel_world_pos(pt.x,pt.y,&wx,&wy,&wz,0);
				switch(dragmode) {
				case 1:
					{
					GameCoord pos={wx<<8,wy<<8,wz<<8};
					move_thing_on_map(dragitem,&pos);
					if (current_packet) current_packet->pos=pos;
					}
					break;
				case 2:
					dx=wx-(dragitem->WorldPos.X>>8);
					dz=(dragitem->WorldPos.Z>>8)-wz;

					angle=(1024+Arctan(dx,dz))&2047;

					if (dragitem->Class==CLASS_PERSON)
						dragitem->Draw.Tweened->Angle=angle;

					if (current_packet) current_packet->angle=angle;

					//dir>>=3;
					break;
				}
//				SetCursor();
			}
		break;
		case	WM_LBUTTONUP:
		case	WM_RBUTTONUP:
			if (current_packet&&dragitem) {
				current_packet->angle=dragitem->Draw.Tweened->Angle;
				current_packet->pos=dragitem->WorldPos;
			}
			dragitem=0;
			ReleaseCapture();
		break;
	}
	return	DefWindowProc(hWnd,message,wParam,lParam);
}

extern SLONG	how_long_is_anim(SLONG anim);

BOOL	CALLBACK	cuts_proc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	HWND the_ctrl;
	static UINT timer;
	int i;
	SLONG wx,wy,wz;

	switch(message)
	{
		case	WM_INITDIALOG:

			CUTSCENE_edit_wnd=hWnd;

			the_ctrl = GetDlgItem(hWnd,IDC_LIST1);
//			InitCols(the_ctrl);
			pedit = new PropertyEditor(the_ctrl);
			//InitProps(pedit,0);
			pedit->SetCallback(propeditCB);

			InitBrowser(the_ctrl = GetDlgItem(hWnd,IDC_TREE1));

			drag = new DragServer(hWnd,GEDIT_hinstance);
			browser->SetDraggable(drag);
			browser->SetCallback(browserCB);

//			timer = SetTimer(hWnd,NULL,40,tf);

			timeline = new TimeLine(
				GetDlgItem(hWnd,IDC_LIST2),
				ruler = new TimeLineRuler(GetDlgItem(hWnd,IDC_BUTTON1)),
				scroll= new TimeLineScroll(GetDlgItem(hWnd,IDC_SCROLLBAR1))
			);
			timeline->SetImageList(GEDIT_hinstance,IDB_SCENE_ICONS1);
			timeline->SetCallback(timelineCB);

			cam_focus_x		=	64 << 8;
			cam_focus_z		=	64 << 8;
			cam_focus_y		=	0x800;
			cam_focus_dist	=	12 << 8;
			cam_pitch		=	1800;
			CUTSCENE_recreate(cutscene);
			return	TRUE;

		case	WM_KEYDOWN:
		case	WM_KEYUP:
			ClearLatchedKeys();
			KeyboardProc(message,wParam,lParam);
			break;

		case WM_USER:
			DoHandleShit();
			return TRUE;

		case	WM_MEASUREITEM:
			if (timeline&&(wParam==IDC_LIST2)) timeline->Measure(lParam);
			break;
		case WM_DRAWITEM:
			if (timeline&&(wParam==IDC_LIST2)) timeline->Draw(lParam);
			if (ruler&&(wParam==IDC_BUTTON1)) ruler->Draw(lParam);
			break;

		case	WM_DESTROY:
//			KillTimer(hWnd,timer);
			break;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case	ID_CEDIT_MOUSELOOK:
				MouselookToggle();
				return TRUE;

			case	ID_CEDIT_PLAYBACK:
				CUTSCENE_playback^=1;
				return TRUE;

			case	ID_CEDIT_REWIND:
				if ((i=timeline->GetReadHead())>0) timeline->SetReadHead(i-1);
				return true;

			case	ID_CEDIT_FFWD:
				if ((i=timeline->GetReadHead())<2000) timeline->SetReadHead(i+1);
				return true;

			case	ID_CEDIT_ERASE:
				DoErase();
				break;

			case	ID_CEDIT_CAMERA_PUNCHIN:
				if (!current_packet) {
					int channum=timeline->GetSelectedRow();
					CSChannel* chan=cutscene->channels+channum;
					if ((channum>=0)&&(channum<cutscene->channelcount)) {
						CSEditChannel* edit=&cutscene->editchannels[channum];
						switch (chan->type) {
						case CT_CAM:
							{
							CSPacket* p=CUTSCENE_add_packet(chan);
							p->start=timeline->GetReadHead();
							p->type=PT_CAM;
							p->length=0xff7f; // fully faded in, and normal lens length
							current_packet=p;
							timeline->MarkEntry(channum,p->start,1,4);
							timeline->Repaint();
							break;
							}
						case CT_TEXT:
							{
							CSPacket* p=CUTSCENE_add_packet(chan);
							p->start=timeline->GetReadHead();
							p->type=PT_TEXT;
							p->length=1;
							p->pos.X=0;
							current_packet=p;
							timeline->MarkEntry(channum,p->start,1,4);
							timeline->Repaint();
							break;
							}
						}
					}
				}
				if (current_packet) {
					switch (current_packet->type) {
					case PT_CAM:
						current_packet->pos.X=cam_focus_x;
						current_packet->pos.Y=cam_focus_y;
						current_packet->pos.Z=cam_focus_z;
						current_packet->angle=cam_yaw;
						current_packet->pitch=cam_pitch;
						break;
					case PT_TEXT:
						// do we need to do anything, really? i doubt it
						break;
					}
					timeline->SetReadHead(current_packet->start); // heh
				}
				return TRUE;

			case	ID_FILE_EXIT:
				PostMessage(hWnd,WM_CLOSE,0,0);
				break;

			case	IDC_BUTTON1:
				return ruler->Process(hWnd,wParam,lParam);

			case	IDC_LIST2:
				return timeline->Process(hWnd,wParam,lParam);

/*				case	IDOK:
					SendMessage(hWnd,WM_CLOSE,0,0);
					return	TRUE;*/
			}
			break;

		case	WM_MOUSEMOVE:
			if (CUTSCENE_mouselook) {
				POINT pt;
				GetCursorPos(&pt);
				SetCursorPos(320,240);
				pt.x-=320; pt.y-=240;
				cam_yaw-=pt.x;
				cam_pitch-=pt.y;
				DoHandleShit();
				ClearLatchedKeys();
				break;
			}
			if (!drag->Process(message,wParam,lParam)) {
				// do stuff here if you care...
			}
			break;

		case	WM_LBUTTONUP:
			if (!drag->Process(message,wParam,lParam)) {
				// do stuff here if you care...
			}
			break;

		case UM_DROP:
			{
				POINT pt,pt2;
				pt.x=LOWORD(lParam);
				pt.y=HIWORD(lParam);
				pt2=pt;
				ClientToScreen(hWnd,&pt);
				HWND target=ChildWindowFromPoint(hWnd,pt2);
				if ((HWND)wParam==browser->hWnd) {
					if (target==timeline->hWnd) {
						ScreenToClient(timeline->hWnd,&pt);
						int chan=timeline->GetRowFromY(pt.y);
						if ((chan>=0)&&(chan<cutscene->channelcount)) {
							CSChannel* chn=cutscene->channels+chan;
							CSEditChannel* edit=&cutscene->editchannels[chan];
							int which,x=timeline->GetCellFromX(pt.x-100);
							int length=1;

							if ((browser->drag_item.iImage==IM_SCENE_WAVE)&&(chn->type!=CT_WAVE)) break;
							if ((browser->drag_item.iImage==IM_SCENE_ACTION)&&(chn->type!=CT_CHAR)) break;
							if ((browser->drag_item.iImage==IM_SCENE_ANIM)&&(chn->type!=CT_CHAR)) break;

							DoErase(chan,x);

							CSPacket* p=CUTSCENE_add_packet(&cutscene->channels[chan]);

							p->start=x;
							switch(browser->drag_item.iImage) {
							case IM_SCENE_WAVE:
								p->type=PT_WAVE;
								p->index=browser->drag_item.lParam;
								/*GI_get_pixel_world_pos(320,240,&wx,&wy,&wz,0);
								p->pos.X=wx;
								p->pos.Y=wy;
								p->pos.Z=wz;*/
								p->pos.X=cam_x;
								p->pos.Y=cam_y;
								p->pos.Z=cam_z;
								p->length=1;
								//which=2;
								which=4;
								break;
							case IM_SCENE_ACTION:
								p->type=PT_ACTION;
								which=1;
								break;
							case IM_SCENE_ANIM:
								p->type=PT_ANIM;
								p->index=browser->drag_item.lParam;
								length=p->index;
								while (length>1023) length-=1024;
								length=/*CUTSCENE_*/how_long_is_anim(/*cutscene->editchannels[chan].thing->Genus.Person->AnimType,*/length);
								which=1;
								p->pos=edit->thing->WorldPos;
								p->angle=edit->thing->Draw.Tweened->Angle;
								p->length=length;
								break;
							}
							timeline->MarkEntry(chan,x,length,which);
							timeline->Repaint();

						}
					}
				}
			}
			break;

		case	WM_NOTIFY:
			switch(LOWORD(wParam))
			{
			case IDC_LIST1:
				return pedit->Process(hWnd,wParam,lParam);
			case IDC_TREE1:
				return browser->Process(hWnd,wParam,lParam);
//				return ProcessListCtl(hWnd,GetDlgItem(hWnd,IDC_LIST1),wParam,lParam);
			break;
			}

		case	WM_HSCROLL:
			return scroll->Process(hWnd,wParam,lParam);

		case	WM_CLOSE:
			ReleaseCapture();
			CUTSCENE_edit_wnd=0;
			delete timeline;
			delete pedit;
			delete browser;
			delete drag;
			delete ruler;
			delete scroll;
			CUTSCENE_remove(cutscene);
//			EndDialog(hWnd,0);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			return	FALSE;
	}
	return	FALSE;
}

//---------------------------------------------------------------

extern	SLONG	load_anim_system(struct GameKeyFrameChunk *game_chunk,CBYTE	*name,SLONG type=0);
extern void	setup_anim_stuff(void);
extern void	setup_global_anim_array(void);

void	do_cutscene_setup(EventPoint *the_ep)
{
	HWND dlg;
	MSG msg;
	HRESULT result;
	HACCEL CEDIT_accel;
	WNDCLASSEX		new_class;
	ATOM	the_class;
	LPCTSTR cname;
	BOOL block_keyboard_messages=0;

	subtitle_str[0]=0;

	//	Create map view window class.
	new_class.cbSize		=	sizeof(WNDCLASSEX);
	new_class.style			=	CS_DBLCLKS|CS_HREDRAW|CS_VREDRAW;
	new_class.lpfnWndProc	=	scene_map_view_proc;
	new_class.cbClsExtra	=	0;
	new_class.cbWndExtra	=	sizeof(HANDLE);
	new_class.hInstance		=	GEDIT_hinstance;
	new_class.hIcon			=	NULL;
	new_class.hCursor		=	GEDIT_arrow;
	new_class.hbrBackground	=	(struct HBRUSH__ *)GetStockObject(BLACK_BRUSH);
	new_class.lpszMenuName	=	NULL;
	new_class.lpszClassName	=	"Scene Map View";
	new_class.hIconSm		=	NULL;
	the_class=RegisterClassEx(&new_class);
	if(!the_class) return;

	if (!the_ep->Data[0]) {
		cutscene=CUTSCENE_data_alloc();
		the_ep->Data[0]=(SLONG)cutscene;
	} else {
		cutscene=(CSData*)the_ep->Data[0];
	}

	CEDIT_accel = LoadAccelerators(GEDIT_hinstance, MAKEINTRESOURCE(IDR_CEDIT_ACCELERATORS));

	// the animations aren't normally loaded in the editor
/*	ANIM_init();
	setup_anim_stuff();
	load_anim_system(&game_chunk[0],"darci1.all");
	load_anim_system(&game_chunk[2],"roper.all");
	load_anim_system(&game_chunk[3],"rthug.all");
	load_anim_system(&game_chunk[4],"roper2.all");
	setup_global_anim_array();*/



	// lets do our own funkathonic message loop thang baby
	dlg=CreateDialog( GEDIT_hinstance, MAKEINTRESOURCE(IDD_SCENE_EDITOR),
		GEDIT_view_wnd, (DLGPROC)cuts_proc );

	ShowWindow(dlg,SW_SHOWNORMAL);

	while(1)
	{
//		MFX_set_listener(cam_x,cam_y,cam_z,-(cam_yaw),0,-(cam_pitch));
		MFX_render();
		if(!PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE))
		{
			// No messages pending- send a user message so we can
			// do our processing and display the engine.

			PostMessage(dlg, WM_USER, 0, 0);

			// clear latched keys in keyboard driver

			ClearLatchedKeys();
		}

		result	=	GetMessage(&msg, NULL, 0, 0);
		if(result==0 || result==-1)
			break;

		// damn you, evil mousewheel handler!
/*		if (msg.message==WM_MOUSEWHEEL) {
			msg.wParam=(short)HIWORD(msg.wParam);
		}
		if (msg.message==GEDIT_wm_mousewheel) {
			msg.message=WM_MOUSEWHEEL;
		}*/

		switch (msg.message) { // special case stuff, whee
		case WM_DESTROY:
		case WM_CLOSE:
			result=-1;
			break;
		}

		if(result==0 || result==-1)
			break;

		// damn you, evil keyboard handler!
		block_keyboard_messages=0;
		switch(msg.message)
		{
			case	WM_KEYDOWN:
			case	WM_KEYUP:
				KeyboardProc(msg.message,msg.wParam,msg.lParam);
				if(ED_KEYS) block_keyboard_messages=1;
				break;
			default:
				TranslateMessage(&msg);
		}


		if (!block_keyboard_messages) {
			BOOL ok=1;
			if (!CUTSCENE_need_keyboard) ok=!TranslateAccelerator(dlg, CEDIT_accel, &msg);
			if (ok) {
				if(dlg==0 || !IsDialogMessage(dlg,&msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
/*		if ((!block_keyboard_messages)&&!TranslateAccelerator(dlg, CEDIT_accel, &msg))
//		if (!block_keyboard_messages)
			if(dlg==0 || !IsDialogMessage(dlg,&msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}*/
	}
	cname=(LPCTSTR)MAKELONG(the_class,0);
	UnregisterClass(cname,GEDIT_hinstance);
//	DestroyWindow(dlg);

}

//---------------------------------------------------------------
