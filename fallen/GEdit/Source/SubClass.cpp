//	SubClass.cpp
//	Guy Simmons, 15th August 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"

#include	"GEdit.h"
#include	"MapView.h"
#include	"WSpace.h"

#include	"pap.h"
#include	"gi.h"

#include	"Anim.h"
#include	"io.h"
#include	"ob.h"
#include	"Editor/Headers/map.h"
#include	"Editor/Headers/Thing.h"

#include	"supermap.h"
#include	"inside2.h"
#include	"memory.h"

#include	"inputbox.h"

extern void SetSkills(UBYTE skills[254]);
extern void refresh_mission(void);

//---------------------------------------------------------------


//	Window procedures used for subclassing.
WNDPROC		check_procs[50],
			combo_procs[50],
			dialog_proc,
			edit_procs[50],
			radio_procs[50],
			tree_proc;

extern BOOL		map_valid;

LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------
/*

  none of these need subclassing any more -- new keyboard thingy

LRESULT	CALLBACK	sc_combo_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	ULONG	c0	=	GetWindowLong(hWnd,GWL_ID);


	switch(message)
	{
		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			if(ED_KEYS)
				return	0;
			break;

	}


	return	CallWindowProc(combo_procs[c0-IDC_COMBO1],hWnd,message,wParam,lParam);
}

//---------------------------------------------------------------

LRESULT	CALLBACK	sc_check_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	ULONG	c0	=	GetWindowLong(hWnd,GWL_ID);


	switch(message)
	{
		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			if(ED_KEYS)
				return	0;
			break;

	}

	return	CallWindowProc(check_procs[c0-IDC_CHECK_1],hWnd,message,wParam,lParam);
}


//---------------------------------------------------------------

LRESULT	CALLBACK	sc_edit_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	ULONG	c0	=	GetWindowLong(hWnd,GWL_ID);


	switch(message)
	{
		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			if(ED_KEYS)
				return	0;
			break;

	}

	return	CallWindowProc(edit_procs[c0-IDC_EDIT1],hWnd,message,wParam,lParam);
}



//---------------------------------------------------------------

LRESULT	CALLBACK	sc_radio_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	ULONG	c0	=	GetWindowLong(hWnd,GWL_ID);


	switch(message)
	{
		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			if(ED_KEYS)
				return	0;
			break;

	}

	return	CallWindowProc(radio_procs[c0-IDC_RADIO1],hWnd,message,wParam,lParam);
}
*/

//------------------------------------------------------------------------
// Guess

void SetCrimeRate(Mission *mission) {
	CBYTE txt[255];
	itoa(mission->CrimeRate,txt,10);
	InputBox("Set crime rate", "Enter new crime rate:", txt);
	mission->CrimeRate=atoi(txt);
	
}

void SetWanderingCivsRate(Mission *mission) {
	CBYTE txt[255];
	itoa(mission->CivsRate,txt,10);
	InputBox("Set fake wandering people rate", "Enter number of civs:", txt);
	mission->CivsRate=atoi(txt);
	
}

void SetWanderingCarsRate(Mission *mission) {
	CBYTE txt[255];
	itoa(mission->CarsRate,txt,10);
	InputBox("Set wandering cars rate", "Enter number of cars:", txt);
	mission->CarsRate=atoi(txt);
}

void SetMusicWorld(Mission *mission) {
	CBYTE txt[255];
	itoa(mission->MusicWorld,txt,10);
	InputBox("Set music world", "Enter Music World:", txt);
	mission->MusicWorld=atoi(txt);
	if (mission->MusicWorld<1) mission->MusicWorld=1;
	if (mission->MusicWorld>20) mission->MusicWorld=20;
}

void SetBoredomRate(Mission *mission) {
	CBYTE txt[255];
	SWORD a;
	a=mission->BoredomRate;
	if (a==255) a=0;
	a*=5;
	itoa(a,txt,10);
	InputBox("Set Mike's strange boredom rate", "Enter Boredom Rate (0-254, 0 is disabled):", txt);
	a=atoi(txt);
	if ((a>0)&&(a<5)) a=1; else a/=5;
	SATURATE(a,0,254);
	if (a==0) a=255;
	mission->BoredomRate=a;
}


void DeleteCivs(Mission *mission) {
	EventPoint *ep, *ep_tst, *ep_base, *TrashList[MAX_EVENTPOINTS];
	SLONG current_ep, walk_ep, i=0, civctr=0, msgctr=0, noskip;
	CBYTE str[_MAX_PATH];

	ZeroMemory(TrashList,sizeof(TrashList));

	// pass one - civvies
	ep_base		=	mission->EventPoints;
	current_ep	=	mission->UsedEPoints;
	while(current_ep) {
		ep	=	TO_EVENTPOINT(ep_base,current_ep);

		if (ep->WaypointType==WPT_CREATE_ENEMIES) {

			walk_ep = mission->UsedEPoints;

			// check the wpt itself doesn't rule out trashing it
			if (LOWORD(ep->Data[0])>2) walk_ep=0; // isn't a civvy
			switch (LOWORD(ep->Data[5])) {
			case 0: case 1: break;
			default: walk_ep=0; // doesn't obey civvy AI
			}
/*			switch (ep->Data[3]) {
			case 0: case 3: break;
			default: walk_ep=0; // doesn't stand still or wander
			}*/
			if (ep->Data[3]!=3) // doesn't wander
				walk_ep=0;

			if ((ep->Data[8]>0)||(ep->Data[9]>0)) // has some goodies
				walk_ep=0;

			/*

			Simons wants us to ignore the flags.

			if (ep->Data[4]) // has some special flags set
				walk_ep=0;

			*/

			noskip = walk_ep;

			// check other wpts don't save it
			while (walk_ep) {
				ep_tst = TO_EVENTPOINT(ep_base,walk_ep);

				if ((ep_tst->EPRef==current_ep)||(ep_tst->EPRefBool==current_ep)) {
					if ((ep_tst->WaypointType==WPT_MESSAGE)&&(ep_tst->TriggeredBy==TT_PERSON_USED))
						;// this one will die later
					else
						break;
				}
				walk_ep = ep_tst->Next;
			}
			if (noskip&&!walk_ep) { // we got to the end of the list without a reason to save
			  TrashList[i++]=ep; civctr++;
			}
		}

		current_ep	=	ep->Next;
	}

	{
		SLONG k;
		SLONG messcount;
		CBYTE fname[256];
		FILE *handle;
	
		extern CBYTE old_path[_MAX_PATH];

		SetCurrentDirectory(old_path);

		for (k = 1; k < 100; k++)
		{
			sprintf(fname, "text\\%s%d.txt", current_mission->MissionName, k);

			handle = fopen(fname, "rb");

			if (handle)
			{
				fclose(handle);
			}
			else
			{
				//
				// Found a spare filename!
				//

				break;
			}
		}

		//
		// Create a new text file.
		//

		handle = fopen(fname, "wb");

		if (handle)
		{
			fprintf(handle, "NORMAL\n\n");
		}

		messcount = 1;

		// pass two - their text
		current_ep	=	mission->UsedEPoints;
		while(current_ep) {
			ep	=	TO_EVENTPOINT(ep_base,current_ep);

			if ((ep->WaypointType==WPT_MESSAGE)&&(ep->TriggeredBy==TT_PERSON_USED)) {
				for (walk_ep=0;walk_ep<i;walk_ep++) {
					if (TO_EVENTPOINT(ep_base,ep->EPRef)==TrashList[walk_ep]) {

						if (ep->Data[0] && handle)
						{
							fprintf(handle, "%d. %s\n", messcount++, ep->Data[0]);
						}

						TrashList[i++]=ep; msgctr++;
						break;
					}
				}
			}
			current_ep	=	ep->Next;
		}

		if (handle)
		{
			sprintf(str, "I've create a citsez file called \"%s\". Shall I make it the citsez text for this mission?", fname);
			
			if (MessageBox(0,str,"Update CitSez text",MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
			{
				strcpy(current_mission->CitSezMapName, fname);
			}

			fclose(handle);
		}

	}

	// pass three - trash 'em all
	for (walk_ep=0;walk_ep<i;walk_ep++) {
		ep=TrashList[walk_ep];
		if (ep==selected_ep) selected_ep=0;
		if (ep==hilited_ep) hilited_ep=0;
		free_eventpoint(ep);
	}

	sprintf(str,"Deleted %d waypoints: %d civies and %d messages.",i,civctr,msgctr);
	MessageBox(0,str,"Delete Civvies",MB_OK|MB_ICONINFORMATION);
}



void DeleteCars(Mission *mission) {
	EventPoint *ep, *ep_tst, *ep_base, *TrashList[MAX_EVENTPOINTS];
	SLONG current_ep, walk_ep, i=0, civctr=0, carctr=0, /*noskip,*/ j;
	CBYTE str[_MAX_PATH];
	UBYTE winner, TrashFlags[MAX_EVENTPOINTS];

	ZeroMemory(TrashList,sizeof(TrashList));
	ZeroMemory(TrashFlags,sizeof(TrashFlags));

	// pass one - find cars, taxis etc (not police cars, ambulances, etc)
	ep_base		=	mission->EventPoints;
	current_ep	=	mission->UsedEPoints;
	while(current_ep) {
		ep	=	TO_EVENTPOINT(ep_base,current_ep);

		if (ep->WaypointType==WPT_CREATE_VEHICLE) {
			switch(ep->Data[0]) {
			case 1: case 2: case 3: case 9: case 10:
				// this is a potential target...
				for (walk_ep=mission->UsedEPoints;walk_ep;walk_ep=ep_tst->Next) {
					ep_tst = TO_EVENTPOINT(ep_base,walk_ep);
					if ((ep_tst->EPRef==current_ep)||(ep_tst->EPRefBool==current_ep)) {
						// saved
						ep_tst=0; break;
					}
					if ((ep_tst->EPRefBool)&&(ep_tst->TriggeredBy==TT_PERSON_IN_VEHICLE)&&(ep_tst->EPRefBool==current_ep)) { 
						ep_tst=0; break; 
					}
					if ((ep_tst->WaypointType==WPT_STALL_CAR)||(ep_tst->WaypointType==WPT_LOCK_VEHICLE)) {
						if ((ep_tst->Data[0])&&(ep_tst->Data[0]==current_ep)) { 
							ep_tst=0; break;
						}
					}
				}
				if (ep_tst) TrashList[i++]=ep;
			}
		}

		current_ep	=	ep->Next;
	}

	// we now have in trashlish, a list of non-policey cars that aren't referred to by other wpts

	// pass two - find matching drivers
	current_ep	=	mission->UsedEPoints;
	while(current_ep) {
		ep	=	TO_EVENTPOINT(ep_base,current_ep);

		if (ep->WaypointType==WPT_CREATE_ENEMIES) {
			if (	(LOWORD(ep->Data[5])==9)  // a driver
				  &&(LOWORD(ep->Data[0])==1)  // a civillian
				  &&(ep->Data[3]==3)		  // wandering
			   ) {
				winner=0;
				for (j=0;j<i;j++) {
					if ((ep->Group==TrashList[j]->Group)&&(ep->Colour==TrashList[j]->Colour)&&(TrashList[j]->WaypointType==WPT_CREATE_VEHICLE)) {
						// we have a winner... check it's not referenced...

						for (walk_ep=mission->UsedEPoints;walk_ep;walk_ep=ep_tst->Next) {
							ep_tst = TO_EVENTPOINT(ep_base,walk_ep);
							if ((ep_tst->EPRef==current_ep)||(ep_tst->EPRefBool==current_ep)) {
								// saved
								ep_tst=0; break;
							}
							/*if ((ep_tst->WaypointType==WPT_STALL_CAR)||(ep_tst->WaypointType==WPT_LOCK_VEHICLE)) {
								if ((ep_tst->Data[0])&&(ep_tst->Data[0]==current_ep)) { 
									ep_tst=0; break;
								}
							}*/
						}
						if (ep_tst) TrashFlags[j]|=1;
						winner=1;
						break;
					}
				}
				if (winner) { TrashFlags[i]|=2; TrashList[i++]=ep; }
			}
		} 

		current_ep	=	ep->Next;
	}

	// pass three - trash 'em all
	j=0;
	for (walk_ep=0;walk_ep<i;walk_ep++) {
		if (TrashFlags[walk_ep]) {
			ep=TrashList[walk_ep];
			if (ep->WaypointType==WPT_CREATE_VEHICLE) carctr++;
			if (ep->WaypointType==WPT_CREATE_ENEMIES) civctr++;
			if (ep==selected_ep) selected_ep=0;
			if (ep==hilited_ep) hilited_ep=0;
			j++;
			free_eventpoint(ep);
		}
	}

	sprintf(str,"Deleted %d waypoints: %d cars and %d civvies.",j,carctr,civctr);
	MessageBox(0,str,"Delete Cars",MB_OK|MB_ICONINFORMATION);
}


//------------------------------------------------------------------------
// This counts the unique prims on the map

void	count_prims_map() {
	BOOL	prim_seen[256]; // ...
	CBYTE*	msg=(CBYTE*)prim_seen;
	UWORD	count=0;
//	OB_Ob *walk=OB_ob;
	SWORD x,z;

	ZeroMemory(prim_seen,sizeof(prim_seen));

	/*	for(i=0;i<OB_ob_upto;i++,walk++)
		if (!prim_seen[walk->prim]) {
			prim_seen[walk->prim]=1;
			count++;
		}*/

	for (x=0;x<OB_SIZE;x++)
		for(z=0;z<OB_SIZE;z++) {
			OB_Info *oi=OB_find(x,z);
			UBYTE oict=0;
			while((oict<31)&&oi->prim)
			{
				if (!prim_seen[oi->prim]) {
					prim_seen[oi->prim]=1;
					count++;
				}
				oi++;
			}

		}

	sprintf(msg,"There are %d unique prim types on this map.",count);
	MessageBox(GEDIT_edit_wnd,msg,"Count Prims",MB_OK);
}

//------------------------------------------------------------------------
// This writes out updated .IAM files

void	save_prim_map(CBYTE *name)
{
	// block-copies all except the prim section which is replaced

//	UWORD	temp;
	SLONG	save_type=1, ob_size1,ob_size2, size;
	//SWORD	c0;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	MFFileHandle	handout	=	FILE_OPEN_ERROR;
	CBYTE name2[_MAX_PATH],name3[_MAX_PATH];
	CBYTE*ptr;
	SLONG copied;
	UBYTE buff[100];
	OB_Mapwho dummy_OB_mapwho[OB_SIZE][OB_SIZE];
	OB_Ob dummy_OB_ob[OB_MAX_OBS];
	SLONG dummy_obctr;

	handle=FileOpen(name);
	if(handle==FILE_OPEN_ERROR) {
		MessageBox(GEDIT_edit_wnd,"File open error.","Error.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}
	strcpy(name2,name);
	ptr=strchr(name2,'.');
	ptr++;
	strcpy(ptr,"ia_");
	handout=FileCreate(name2,TRUE);
	if(handout==FILE_OPEN_ERROR) {
		FileClose(handle);
		MessageBox(GEDIT_edit_wnd,"File create error.","Error.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}
	
	FileRead(handle,(UBYTE*)&save_type,4);
	FileWrite(handout,(UBYTE*)&save_type,4);

	ob_size2=sizeof(OB_ob_upto) + (sizeof(OB_Ob)*OB_ob_upto) + (sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);

	if (save_type<24) {
		FileClose(handle);
		FileClose(handout);
		MessageBox(GEDIT_edit_wnd,"This is an out-of-date map file. Please try again with a version 24 or later file.","Error.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}
	
	FileRead(handle,(UBYTE*)&ob_size1,4);
	FileWrite(handout,(UBYTE*)&ob_size2,4);

	size=FileSize(handle)-12; // 4 bytes save_type plus 4 bytes ob_size1 4 bytes bloody texture thingy
	if (save_type>=25) {
		size-=2000;			  // and another 2000 bytes of psx texturey stuff
	}
	size-=ob_size1;           // size is now the amount of data before the obs

	ptr = new CBYTE[size];

	FileRead(handle,(UBYTE*)ptr,size);
	FileWrite(handout,(UBYTE*)ptr,size);

	delete [] ptr;


/*
	FileRead(handle,(UBYTE*)&PAP_2HI(0,0),sizeof(PAP_Hi)*PAP_SIZE_HI*PAP_SIZE_HI);
	FileWrite(handout,(UBYTE*)&PAP_2HI(0,0),sizeof(PAP_Hi)*PAP_SIZE_HI*PAP_SIZE_HI);

	if(save_type==18)
	{
		FileRead(handle,(UBYTE*)&temp,sizeof(temp));
		FileWrite(handout,(UBYTE*)&temp,sizeof(temp));
		for(c0=0;c0<temp;c0++)
		{
			ASSERT(0);
		}
	} else
		if(save_type>18)
		{
			FileRead(handle,(UBYTE*)&temp,sizeof(temp));
			FileWrite(handout,(UBYTE*)&temp,sizeof(temp));
			for(c0=0;c0<temp;c0++) {
				struct	LoadGameThing	map_thing;
				FileRead(handle,(UBYTE*)&map_thing,sizeof(struct LoadGameThing));
				FileWrite(handout,(UBYTE*)&map_thing,sizeof(struct LoadGameThing));
			}
		}
*/
/* say goodbye to alllllll this shit

	// now for some fat ugly building stuff.
	{
		UWORD next_dbuilding,next_dfacet,next_dstyle,next_paint_mem,next_dstorey;
		UBYTE *data1,*data2,*data3,*data4,*data5;
		SLONG size1,size2,size3,size4,size5;

		FileRead(handle,&next_dbuilding,2);
		FileRead(handle,&next_dfacet,2);
		FileRead(handle,&next_dstyle,2);
		FileRead(handle,&next_paint_mem,2);
		FileRead(handle,&next_dstorey,2);

		FileWrite(handout,&next_dbuilding,2);
		FileWrite(handout,&next_dfacet,2);
		FileWrite(handout,&next_dstyle,2);
		FileWrite(handout,&next_paint_mem,2);
		FileWrite(handout,&next_dstorey,2);

		size1 = sizeof(struct DBuilding)*next_dbuilding;
		size2 = sizeof(struct DFacet)*next_dfacet;
		size3 = sizeof(UWORD)*next_dstyle;
		size4 = sizeof(UBYTE)*next_paint_mem;
		size5 = sizeof(struct DStorey)*next_dstorey;

		data1 = new UBYTE[size1];
		data2 = new UBYTE[size2];
		data3 = new UBYTE[size3];
		data4 = new UBYTE[size4];
		data5 = new UBYTE[size5];
		
		FileRead(handle,data1, size1);
		FileRead(handle,data2, size2);
		FileRead(handle,data3, size3);
		FileRead(handle,data4, size4);
		FileRead(handle,data5, size5);

		FileWrite(handout,data1,size1);
		FileWrite(handout,data2,size2);
		FileWrite(handout,data3,size3);
		FileWrite(handout,data4,size4);
		FileWrite(handout,data5,size5);

		delete [] data1;
		delete [] data2;
		delete [] data3;
		delete [] data4;
		delete [] data5;
	}

	// now for some equally ugly, but slightly slimmer insides stuff
	{
		UWORD next_inside_storey, next_inside_stair;
		SLONG next_inside_block;
		UBYTE *data1,*data2,*data3;
		SLONG size1,size2,size3;

		FileRead(handle,&next_inside_storey,sizeof(next_inside_storey));
		FileRead(handle,&next_inside_stair,sizeof(next_inside_stair));
		FileRead(handle,&next_inside_block,sizeof(next_inside_block));

		FileWrite(handout,&next_inside_storey,sizeof(next_inside_storey));
		FileWrite(handout,&next_inside_stair,sizeof(next_inside_stair));
		FileWrite(handout,&next_inside_block,sizeof(next_inside_block));

		size1=sizeof(struct InsideStorey)*next_inside_storey;
		size2=sizeof(struct Staircase)*next_inside_stair;
		size3=sizeof(UBYTE)*next_inside_block;

		data1 = new UBYTE[size1];
		data2 = new UBYTE[size2];
		data3 = new UBYTE[size3];

		FileRead(handle,data1,size1);
		FileRead(handle,data2,size2);
		FileRead(handle,data3,size3);

		FileWrite(handout,data1,size1);
		FileWrite(handout,data2,size2);
		FileWrite(handout,data3,size3);

		delete [] data1;
		delete [] data2;
		delete [] data3;

	}
*/


	//
	// All this ob nonsense are the objects on the map (like lamposts)
	//
	// load the old ones 

	FileRead(handle,(UBYTE*)&dummy_obctr,sizeof(OB_ob_upto));
	FileRead(handle,(UBYTE*)&dummy_OB_ob[0],sizeof(OB_Ob)*dummy_obctr);
	FileRead(handle,(UBYTE*)&dummy_OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
/*
	FileRead(handle,(UBYTE*)&OB_ob_upto,sizeof(OB_ob_upto));
	FileRead(handle,(UBYTE*)&OB_ob[0],sizeof(OB_Ob)*OB_ob_upto);
	FileRead(handle,(UBYTE*)&OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);
*/
	// then throw em away, we got new ones
	FileWrite(handout,(UBYTE*)&OB_ob_upto,sizeof(OB_ob_upto));
	FileWrite(handout,(UBYTE*)&OB_ob[0],sizeof(OB_Ob)*OB_ob_upto);
	FileWrite(handout,(UBYTE*)&OB_mapwho[0][0],sizeof(OB_Mapwho)*OB_SIZE*OB_SIZE);

	// blockdump the rest

	do {
		copied=FileRead(handle,(UBYTE*)&buff[0],100);
		FileWrite(handout,(UBYTE*)&buff[0],copied);
	} while (copied==100);

/*
	//
	// load super map
	//

	load_super_map(handle,save_type);

	if (save_type >= 20)
	{
		SLONG texture_set;

		FileRead(handle,(UBYTE*)&texture_set,sizeof(texture_set));

		ASSERT(WITHIN(texture_set, 1, 8));

		TEXTURE_choose_set(texture_set);
	}
	else
	{
		TEXTURE_choose_set(1);
	}
*/
	FileClose(handle);
	FileClose(handout);

	// erase backup. :P
	strcpy(name3,name);
	strcat(name3,".bak");
	DeleteFile(name3);
	
	// shift original to backup
	rename(name,name3);

	// shift new to original
	rename(name2,name);



}

SLONG add_prim_to(MapThing *map, SLONG pos, OB_Info *oi) {
	if (pos>=MAX_MAP_THINGS) return pos;
	while (map[pos].Type) {
		pos++;
		if (pos>=MAX_MAP_THINGS) return pos;
	}
	memset(&map[pos],0,sizeof(MapThing));
	map[pos].Type=MAP_THING_TYPE_PRIM;
	map[pos].AngleY=oi->yaw;
	map[pos].X=oi->x;
	map[pos].Y=oi->y;
	map[pos].Z=oi->z;
	map[pos].IndexOther=oi->prim;
	pos++;
	return pos;
}

// This writes out updated .MAP files

void update_prims_on_map(CBYTE *orig_name) {
	CBYTE name[_MAX_PATH],name2[_MAX_PATH],name3[_MAX_PATH],msg[_MAX_PATH], *ptr;
	MFFileHandle	handle	=	FILE_OPEN_ERROR;
	MFFileHandle	handout	=	FILE_OPEN_ERROR;
	SLONG temp, size, c0,x,z;
//	UWORD point,face4,face3,obj;
	UBYTE *buffer;
	MapThing some_map_things[MAX_MAP_THINGS];
	OB_Info *oi;


	strcpy(name,orig_name);
	ptr=strchr(name,'.');
	ptr++;
	strcpy(ptr,"map");
	handle=FileOpen(name);
	if(handle==FILE_OPEN_ERROR) {
		sprintf(msg,"File open error: %s",name);
		MessageBox(GEDIT_edit_wnd,msg,"Error.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}
	strcpy(name2,orig_name);
	ptr=strchr(name2,'.');
	ptr++;
	strcpy(ptr,"ma_");
	handout=FileCreate(name2,TRUE);
	if(handout==FILE_OPEN_ERROR) {
		FileClose(handle);
		sprintf(msg,"File create error: %s",name2);
		MessageBox(GEDIT_edit_wnd,msg,"Error.",MB_ICONEXCLAMATION|MB_OK);
		return;
	}


	// block copy everything up-to the mapthings

	FileRead(handle,(UBYTE*)&temp,4);
	FileWrite(handout,(UBYTE*)&temp,4);

	if (temp<26) {
		FileClose(handle);
		FileClose(handout);
		sprintf(msg,"File update error: %s is a version %d file; prim editing requires version 26 or later.",name,temp);
		MessageBox(GEDIT_edit_wnd,msg,"Error:",MB_ICONEXCLAMATION|MB_OK);
		return;
	}

	c0=sizeof(struct MapThing)*MAX_MAP_THINGS;
	size=FileSize(handle)-4;
	size-=c0;
	buffer=(UBYTE*)malloc(size);
	FileRead(handle,buffer,size);
	FileWrite(handout,buffer,size);
	delete [] buffer;
	
/*
	size=sizeof(struct DepthStrip)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH;
	buffer=(UBYTE*)malloc(size);

	FileRead(handle,buffer,size);
	FileWrite(handout,buffer,size);

	size=sizeof(UWORD)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH;
	FileRead(handle,buffer,size);
	FileWrite(handout,buffer,size);

	size=sizeof(SBYTE)*EDIT_MAP_WIDTH*EDIT_MAP_DEPTH;
	FileRead(handle,buffer,size);
	FileWrite(handout,buffer,size);

	FileRead(handle,buffer,sizeof(UWORD)*4);
	FileWrite(handout,buffer,sizeof(UWORD)*4);

	free(buffer);

	FileRead(handle,(UBYTE*)&point,sizeof(UWORD));
	FileWrite(handout,(UBYTE*)&point,sizeof(UWORD));
	FileRead(handle,(UBYTE*)&face4,sizeof(UWORD));
	FileWrite(handout,(UBYTE*)&face4,sizeof(UWORD));
	FileRead(handle,(UBYTE*)&face3,sizeof(UWORD));
	FileWrite(handout,(UBYTE*)&face3,sizeof(UWORD));
	FileRead(handle,(UBYTE*)&obj,sizeof(UWORD));
	FileWrite(handout,(UBYTE*)&obj,sizeof(UWORD));

	size=sizeof(struct PrimPoint) * point;
	buffer=(UBYTE*)malloc(size);
	FileRead(handle,(UBYTE*)buffer ,size);
	FileWrite(handout,(UBYTE*)buffer ,size);
	free(buffer);

	size=sizeof(struct PrimFace4) * face4;
	buffer=(UBYTE*)malloc(size);
	FileRead(handle,(UBYTE*)buffer ,size);
	FileWrite(handout,(UBYTE*)buffer ,size);
	free(buffer);

	size=sizeof(struct PrimFace3) * face3;
	buffer=(UBYTE*)malloc(size);
	FileRead(handle,(UBYTE*)buffer ,size);
	FileWrite(handout,(UBYTE*)buffer ,size);
	free(buffer);

	size=sizeof(struct PrimObject) * obj;
	buffer=(UBYTE*)malloc(size);
	FileRead(handle,(UBYTE*)buffer ,size);
	FileWrite(handout,(UBYTE*)buffer ,size);
	free(buffer);

	FileRead(handle,(UBYTE*)&temp,sizeof(UWORD));
	FileWrite(handout,(UBYTE*)&temp,sizeof(UWORD));
*/
	// read old prims and stuff
	FileRead(handle,(UBYTE*)&some_map_things[0],c0);

	// clear prims, leave stuff

	for(c0=0;c0<MAX_MAP_THINGS;c0++) {
		if (some_map_things[c0].Type==MAP_THING_TYPE_PRIM) some_map_things[c0].Type=0;
	}

	// insert prims between stuff
	size=0;
/*	for(c0=0;c0<OB_ob_upto;c0++) {
		if (size>=MAX_MAP_THINGS) break; // tuff luck
		while (some_map_things[size].Type!=0) size++;
		some_map_things[size].Type=MAP_THING_TYPE_PRIM;
		some_map_things[size].AngleY=OB_ob[c0].yaw<<3;
//		some_map_things[size].X= ... the x, somehow
		size++;
	}
*/

	size=0;
	for (x=0;x<OB_SIZE;x++)
		for (z=0;z<OB_SIZE;z++) {
			oi = OB_find(x,z);
			while(oi->prim) {
				size=add_prim_to(some_map_things,size,oi);
				oi++;
			}
		}


	// write new prims and stuff
	FileWrite(handout,(UBYTE*)&some_map_things[0],sizeof(struct MapThing)*MAX_MAP_THINGS);

	// blockdump the rest

	buffer=(UBYTE*)malloc(2048);
	do {
		size=FileRead(handle,buffer,2048);
		FileWrite(handout,buffer,size);
	} while (size==2048);
	free(buffer);


	FileClose(handle);
	FileClose(handout);

	// erase backup. :P
	strcpy(name3,name);
	strcat(name3,".bak");
	DeleteFile(name3);
	
	// shift original to backup
	rename(name,name3);

	// shift new to original
	rename(name2,name);

}

//
// Tells you how many treaures and health items, ammo
//

void show_mission_info()
{
	SLONG i;

	CBYTE message[512];

	SLONG num_treasures    = 0;
	SLONG num_health       = 0;
	SLONG num_ammo_pistol  = 0;
	SLONG num_ammo_shotgun = 0;
	SLONG num_ammo_ak47    = 0;
	SLONG num_pistol       = 0;
	SLONG num_shotgun      = 0;
	SLONG num_ak47         = 0;

	EventPoint *ep;

	for (i = 0; i < MAX_EVENTPOINTS; i++)
	{
		ep = &current_mission->EventPoints[i];
		
		if (ep->WaypointType == WPT_CREATE_ITEM)
		{
			switch(ep->Data[0])
			{
				case IT_HEALTH:       num_health       += 1; break;
				case IT_AMMO_PISTOL:  num_ammo_pistol  += 1; break;
				case IT_AMMO_SHOTGUN: num_ammo_shotgun += 1; break;
				case IT_AMMO_AK47:    num_ammo_ak47    += 1; break;
				case IT_PISTOL:       num_pistol       += 1; break;
				case IT_SHOTGUN:      num_shotgun      += 1; break;
				case IT_AK47:         num_ak47         += 1; break;
			}
		}
		else
		if (ep->WaypointType == WPT_CREATE_TREASURE)
		{
			num_treasures += 1;
		}
	}

	sprintf(
		message,
		"Current mission info:\n\n"
		"\tTreasures:   \t%d\n"
		"\tHealths:     \t%d\n"
		"\tPistols:     \t%d (Ammo %d)\t\n"
		"\tShotguns:    \t%d (Ammo %d)\t\n"
		"\tAK47s:       \t%d (Ammo %d)\t\n\n",
		num_treasures,
		num_health,
		num_pistol,  num_ammo_pistol,
		num_shotgun, num_ammo_shotgun,
		num_ak47,    num_ammo_ak47);

	MessageBox(NULL, message, "Mission info", MB_OK|MB_ICONEXCLAMATION);
}


void set_car_collision_with_road_prims()
{
	switch(MessageBox(
			NULL,
			(current_mission->Flags & MISSION_FLAG_CARS_WITH_ROAD_PRIMS) ? "Collision is currently ON" : "Collision is now OFF",
			"Set car collision with prims on the roads",
			MB_YESNOCANCEL|MB_ICONQUESTION))
	{
		case IDNO:
			current_mission->Flags &= ~MISSION_FLAG_CARS_WITH_ROAD_PRIMS;
			break;

		case IDYES:
			current_mission->Flags |=  MISSION_FLAG_CARS_WITH_ROAD_PRIMS;
			break;

		case IDCANCEL:
			break;

		default:
			ASSERT(0);
			break;
	}
}

//---------------------------------------------------------------

void ws_refresh_all()
{
	SLONG i;
	Mission* previous_mission = current_mission;

	if (!previous_mission)
	{
		MessageBox(0,"You must load at least one map & mission first.","Refresh Missions",MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	// iterate current_mission, call refresh_mission() a fuck of a lot.
	for (i=1;i<MAX_MISSIONS;i++)
	{
		current_mission	=	&mission_pool[i];
		if (current_mission->Flags & MISSION_FLAG_USED)
			refresh_mission();
	}
	
	current_mission=previous_mission;

	ResetFreelist(current_mission);
	ResetUsedlist(current_mission);
	ResetFreepoint(current_mission);
	ResetUsedpoint(current_mission);

	reset_wptlist();
	fill_wptlist(current_mission);

	MessageBox(0,"Done.","Refresh Missions",MB_OK|MB_ICONINFORMATION);

}

//---------------------------------------------------------------

void remove_map_from_wspace(GameMap* current_map)
{
  HTREEITEM current = TreeView_GetSelection(ws_tree);
  SLONG c0, c1;

  //	Free up associated missions.
	for(c0=1;c0<MAX_MISSIONS;c0++)
	{
		if(current_map->Missions[c0])
		{
			for(c1=0;c1<MAX_EVENTPOINTS;c1++)
				if (mission_pool[current_map->Missions[c0]].EventPoints[c1].Used)
					free_eventpoint(&mission_pool[current_map->Missions[c0]].EventPoints[c1]);
			free_mission(current_map->Missions[c0]);
		}
	}

void		remove_children(HTREEITEM parent);

  remove_children(current);
  free_map(current_map-game_maps);
  
}

//---------------------------------------------------------------

LRESULT	CALLBACK	sc_tree_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	POINT			click_point;


	switch(message)
	{
		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
//--- workspace ---//
				case	ID_WORKSPACEROOT_ADD:
					ws_add_map();
					return	0;

				case	ID_WORKSPACEROOT_REFRESH:
					ws_refresh_all();
					return	0;
//--- map ---//
				case	ID_MAPROOT_NEWMISSION:
					ws_new_mission();
					return	0;

				case	ID_MAPROOT_IMPORTMISSION:
					import_mission();
					return 0;

				case	ID_MAPROOT_RESETPRIMS:
					if (current_map&&map_valid) {
						if (MessageBox(GEDIT_edit_wnd,"Discard all changes to the prims on this map?","Are you sure?",MB_ICONQUESTION|MB_YESNO)==IDYES)
							GI_load_map(current_map->MapName);
					}
					return 0;

				case	ID_MAPROOT_COUNTPRIMS:
					if (current_map&&map_valid)
						count_prims_map();
					return 0;

				case	ID_MAPROOT_SAVEPRIMS:
//					MessageBox(GEDIT_edit_wnd,"Saving prims is temporarily disabled while a bug is fixed. Try again later...","Arse off...",MB_OK);
//					return 0;
					if (current_map&&map_valid) {
						if (MessageBox(GEDIT_edit_wnd,"Permanently save prims for this map?","Are you sure?",MB_ICONQUESTION|MB_YESNO)==IDYES) {
							save_prim_map(current_map->MapName);
							update_prims_on_map(current_map->MapName);
						}
					}
					return 0;

				case	ID_MAPROOT_DELETEMAP:
					if (!current_map)
						MessageBox(GEDIT_edit_wnd,"Select a map first.","Error",MB_ICONEXCLAMATION|MB_OK);
					else
						if (MessageBox(GEDIT_edit_wnd,"Remove this map and all associated missions from the workspace?","Are you sure?",MB_ICONQUESTION|MB_YESNO)==IDYES)
							remove_map_from_wspace(current_map);
					return 0;

//--- mission ---//
				case	ID_MISSIONROOT_ADDMISSIONBRIEF:
					return	0;
				
				case	ID_MISSIONROOT_ADDLIGHTMAP:
					ws_add_light_map();
					return	0;
				
				case	ID_MISSIONROOT_ADDSEWERMAP:
					ws_add_citsez_map();
					return	0;

				case	ID_MISSIONROOT_SETDEFAULTSKILLS:
					SetSkills(current_mission->SkillLevels);
					return 0;

				case ID_MISSIONROOT_SETCRIMERATE:
					SetCrimeRate(current_mission);
					return 0;

				case ID_MISSIONROOT_SETCIVVYCOUNT :
					SetWanderingCivsRate(current_mission);
					return 0;

				case ID_MISSIONROOT_SETCARSCOUNT:
					SetWanderingCarsRate(current_mission);
					return 0;

				case ID_MISSIONROOT_SETMUSICWORLD:
					SetMusicWorld(current_mission);
					return 0;

				case ID_MISSIONROOT_CRIMERATE_VISIBILITY:

					{
						switch(	MessageBox(0,"Do you want the crime rate visible on this map?", "Set crime rate visible/invisible", MB_ICONQUESTION|MB_YESNOCANCEL))
						{
							case IDYES:
								current_mission->Flags |= MISSION_FLAG_SHOW_CRIMERATE;
								break;

							case IDNO:
								current_mission->Flags &= ~MISSION_FLAG_SHOW_CRIMERATE;
								break;
						}
					}
					
					return 0;

				case ID_MISSIONROOT_BOREDOM:
					SetBoredomRate(current_mission);
					return 0;

				case ID_MISSIONROOT_SETCARCOLLISIONWITHROADPRIMS:
					set_car_collision_with_road_prims();
					break;


				case ID_MISSIONROOT_DELETECIVS:
					if (MessageBox(0,"This will delete ALL civillians on the level, along with their text, if they aren't referred to by other waypoints. Are you sure??",
						"Confirm: Delete Civs",MB_OKCANCEL|MB_ICONEXCLAMATION)==IDOK) DeleteCivs(current_mission);
					return 0;

				case ID_MISSIONROOT_DELETECARS:
					if (MessageBox(0,"This will delete all 'wandering' cars on the level, along with their drivers, if they aren't referred to by other waypoints. Are you sure??",
						"Confirm: Delete Cars",MB_OKCANCEL|MB_ICONEXCLAMATION)==IDOK) DeleteCars(current_mission);
					return 0;

				case	ID_MISSIONROOT_DELETEMISSION:
					ws_del_mission();
					return 0;

				case	ID_MISSIONROOT_EXPORTMISSION:
					//	Export the mission in game form.
					export_mission();
					return	0;

				case	ID_MISSIONROOT_VALIDATEMISSION:
					valid_mission();
					return 0;

				case	ID_MISSIONROOT_REFRESHMISSION:
					refresh_mission();
					return 0;

				case	ID_MISSIONROOT_SHOWMISSIONINFO:
					show_mission_info();
					return 0;

			}
			break;

		case	WM_KEYDOWN:
		case	WM_KEYUP:
			KeyboardProc(message,wParam,lParam);
			if(ED_KEYS)
				return	0;
			break;

		case	WM_LBUTTONDBLCLK:
			//	Make point relative to screen coords instead of window.
			click_point.x	=	LOWORD(lParam);
			click_point.y	=	HIWORD(lParam);
			ClientToScreen(hWnd,&click_point);
			if(handle_ws_dblclk(&click_point))
				return	0;
			break;
	}

	return	CallWindowProc(tree_proc,hWnd,message,wParam,lParam);
}


//---------------------------------------------------------------

LRESULT	CALLBACK	sc_dialog_proc	(
										HWND hWnd,
										UINT message,
										WPARAM wParam,
										LPARAM lParam
									)
{
	switch(message)
	{
		case	WM_ENTERIDLE:
			process_view_wind();
			return	0;
	}
	return	CallWindowProc(dialog_proc,hWnd,message,wParam,lParam);
}

//---------------------------------------------------------------
