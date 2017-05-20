//	WSpace.cpp
//	Guy Simmons, 11th August 1998.

#include	<MFStdLib.h>
#include	<windows.h>
#include	<windowsx.h>
#include	<ddlib.h>
#include	<commctrl.h>
#include	"resource.h"

#include	"gi.h"

#include	"GEdit.h"
#include	"Mission.h"
#include	"SubClass.h"
#include	"WSpace.h"
#include	"MapView.h"

//---------------------------------------------------------------

BOOL				workspace_changed	=	FALSE;
CBYTE				map_default_dir[_MAX_PATH],
					map_file_name[_MAX_PATH],
					map_path_name[_MAX_PATH],
					mission_name[_MAX_PATH],
					workspace_dir[_MAX_PATH],
					workspace_name[_MAX_PATH],
					workspace_path[_MAX_PATH];
HANDLE				image_list;
HWND				ws_tree,wpt_tree;
HTREEITEM			last_clicked, selected_mission_doohickey=0;
WSElement			root_item;

extern HINSTANCE	GEDIT_hinstance;

extern BOOL		map_valid;

//	The camera.
extern SLONG	cam_x,
				cam_y,
				cam_z,
				cam_yaw,
				cam_pitch,
				cam_focus_x,
				cam_focus_z,
				cam_focus_dist,
				cam_matrix[9],
				cam_forward[3],
				cam_left[3];

//	The mouse.
extern SLONG	mouse_valid,
				mouse_over,
				mouse_world_x,
				mouse_world_y,
				mouse_world_z,
				mouse_waypoint;

void		calc_camera_pos(void);
void		remove_children(HTREEITEM parent);

//---------------------------------------------------------------

BOOL	init_workspace(HWND parent)
{
	TV_INSERTSTRUCT		tv_is;


	//	Subclass the tree control windproc.
	ws_tree	=	GetDlgItem(parent,IDC_WORKSPACE_TREE);
	tree_proc	=	(WNDPROC)SetWindowLong(ws_tree,GWL_WNDPROC,(long)sc_tree_proc);

	//	Set up the trees image list.
	image_list	=	ImageList_LoadBitmap(
											GEDIT_hinstance,
											MAKEINTRESOURCE(IDB_WORKSPACE),
											16,
											1,
											RGB (255, 0, 255)
										);

/*	image_list	=	ImageList_LoadImage(GEDIT_hinstance,MAKEINTRESOURCE(IDB_WORKSPACE2),16,1,
		RGB(255,0,255),IMAGE_BITMAP,LR_DEFAULTCOLOR);*/

	TreeView_SetImageList(ws_tree,image_list,TVSIL_NORMAL);

	//	Setup working directories.
	sprintf(workspace_dir,"c:\\Fallen\\Editor");
	sprintf(map_default_dir,"c:\\Fallen\\Data");

	//	Set up the tree root.
	tv_is.hParent				=	TVI_ROOT;
	tv_is.hInsertAfter			=	TVI_FIRST;
	tv_is.item.mask				=	TVIF_TEXT|TVIF_PARAM;
	tv_is.item.pszText			=	"Workspace : NONE";
	tv_is.item.lParam			=	(LPARAM)&root_item;
	root_item.TreeItem			=	TreeView_InsertItem(ws_tree,&tv_is);
	root_item.ElementType		=	ET_NONE;

	//	Set the workspace state & corresponding menus.
	workspace_changed	=	FALSE;
	menu_no_workspace();


	return	TRUE;
}

//---------------------------------------------------------------

void	fini_workspace(void)
{
	
}

//---------------------------------------------------------------

BOOL	get_element_at_point(POINT *click_point,WSElement **the_element)
{
	HTREEITEM			item_handle;
	TV_HITTESTINFO		hit_test;
	TV_ITEM				the_item;


	//	Validate the parameters.
	if(click_point && the_element)
	{
		//	Check to see if we clicked on a tree item.
		hit_test.pt		=	*click_point;
		ScreenToClient(ws_tree,&hit_test.pt);
		item_handle		=	TreeView_HitTest(ws_tree,&hit_test);
		if(item_handle && hit_test.flags&TVHT_ONITEM)
		{
			//	Yes, so get a pointer to its WSElement structure.
			the_item.hItem	=	item_handle;
			the_item.mask	=	TVIF_PARAM;
			if(TreeView_GetItem(ws_tree,&the_item))
			{
				*the_element	=	(WSElement*)the_item.lParam;
				if(*the_element) {
					last_clicked=item_handle;
					return	TRUE;
				}
			}
		}
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	handle_ws_context(POINT *click_point)
{
	HMENU				ws_menu;
	WSElement			*the_element;


	//	Validate the parameters.
	if(ws_tree && click_point)
	{
		if(get_element_at_point(click_point,&the_element))
		{
			//	Select the item.
			TreeView_Select(ws_tree,the_element->TreeItem,TVGN_CARET);

			if (the_element->ElementType==ET_LMAP) return;

			//	Bring up the relevent context menu.
			ws_menu	=	GetSubMenu(LoadMenu(GEDIT_hinstance,MAKEINTRESOURCE(IDR_GEDIT_POPUPS)),the_element->ElementType-1);
			TrackPopupMenu	(
								ws_menu,
								TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
								click_point->x,click_point->y,
								0,ws_tree,NULL
							);
			DestroyMenu(ws_menu);
		}
	}
}

//---------------------------------------------------------------

BOOL	handle_ws_dblclk(POINT *click_point)
{
//	CBYTE				item_text[_MAX_PATH];
//	TV_ITEM				the_item;
	WSElement			*the_element;
	HTREEITEM			next;

	//	Validate the parameters.
	if(ws_tree && click_point)
	{
		if(get_element_at_point(click_point,&the_element))
		{
			switch(the_element->ElementType)
			{
				case	ET_NONE:
					break;
				case	ET_ROOT:
					break;
				case	ET_MAP:

					// Expand the workspace
					TreeView_Expand(ws_tree,last_clicked,TVE_EXPAND);

					//	Set the 'busy' cursor.
					SetCursor(GEDIT_busy);

					//	Try to open the map.
					SetCurrentDirectory("c:\\Fallen");
					//GI_init();
					map_valid		=	GI_load_map(current_map->MapName);
					if(map_valid)
					{
						//	Default setup for map view.
						cam_focus_x		=	64 << 8;
						cam_focus_z		=	64 << 8;
						cam_focus_dist	=	14 << 8;
						cam_pitch		=	1600;
						cam_yaw			=	0;
						calc_camera_pos();
						GI_render_view_into_backbuffer	(
															cam_x,
															cam_y,
															cam_z,
															cam_yaw,
															cam_pitch,
															0
														);
					}
					//	Set the 'arrow' cursor.
					SetCursor(GEDIT_arrow);

					//  Select the first mission (if present)
					next=TreeView_GetChild(ws_tree,last_clicked);
					if (next) TreeView_SelectItem(ws_tree,next);


					return	TRUE;

				case	ET_MISSION:
					break;
				case	ET_LMAP:
					break;
				case	ET_WAYPOINT:
					break;
				case	ET_BRIEF:
					break;
			}
		}
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	handle_ws_select(WSElement *the_element)
{
	switch(the_element->ElementType)
	{
		case	ET_NONE:
			break;

		case	ET_ROOT:
			break;

		case	ET_MAP:
			current_map		=	&game_maps[the_element->MapRef];
			break;

		case	ET_MISSION:
			if	(
					current_map && 
					MAP_NUMBER(current_map)==TO_MISSION(the_element->MissionRef)->MapIndex
				)
			{
				TVITEM item;
				// clear old bold
				item.mask=TVIF_STATE|TVIF_HANDLE;
				item.stateMask=TVIS_BOLD;
				if (selected_mission_doohickey) {
					item.hItem=selected_mission_doohickey;
					item.state=0;
					TreeView_SetItem(ws_tree,&item);
				}
				// set new bold
				item.hItem=the_element->TreeItem;
				item.state=TVIS_BOLD;
				TreeView_SetItem(ws_tree,&item);
				current_mission	=	&mission_pool[the_element->MissionRef];
				selected_mission_doohickey = the_element->TreeItem;
				reset_wptlist();
				fill_wptlist(current_mission);
			}
			else
			{
				//	We probably need to prompt here, to open the missions associated map.
//				current_mission	=	NULL;
				current_mission	=	&mission_pool[the_element->MissionRef];
			}
			break;

		case	ET_LMAP:
			break;

		case	ET_WAYPOINT:
			break;

		case	ET_BRIEF:
			break;
	}
}

//---------------------------------------------------------------

void	ws_add_map(void)
{
	CBYTE				*text_buffer;
	UWORD				item_count,
						new_map;
	HTREEITEM			map_item;
	OPENFILENAME		open_map;
	TV_INSERTSTRUCT		tv_is;
	TV_ITEM				map_info;
	WSElement			*new_element;


	//	Set up the open file structure.
	ZeroMemory(&open_map,sizeof(OPENFILENAME));
	open_map.lStructSize		=	sizeof(OPENFILENAME);
	open_map.hwndOwner			=	NULL;
	open_map.lpstrFilter		=	"Game map files\0*.iam\0\0";
	open_map.lpstrFile			=	map_path_name;
	open_map.nMaxFile			=	_MAX_PATH;
	open_map.lpstrFileTitle		=	map_file_name;
	open_map.nMaxFileTitle		=	_MAX_PATH;
	open_map.lpstrInitialDir	=	map_default_dir;
	open_map.lpstrTitle			=	"Load a game map";
	open_map.Flags				=	OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	open_map.lpstrDefExt		=	"iam";

	if(GetOpenFileName(&open_map))
	{
		//	Check to see if this map is part of the workspace.
		text_buffer	=	(CBYTE*)malloc(_MAX_PATH);
		map_item	=	TreeView_GetChild(ws_tree,root_item.TreeItem);
		while(map_item)
		{
			map_info.mask		=	TVIF_TEXT;
			map_info.hItem		=	map_item;
			map_info.pszText	=	text_buffer;
			map_info.cchTextMax	=	_MAX_PATH;
			TreeView_GetItem(ws_tree,&map_info);

			if(!strcmp(map_file_name,text_buffer))
			{
				//	Yes, so show a message & get outta here.
				MessageBox	(
								NULL,
								"This map is already part of the workspace",
								"DOH!!",
								MB_OK
							);
				free(text_buffer);
				return;
			}

			//	Get next sibling.				
			map_item	=	TreeView_GetNextSibling(ws_tree,map_item);
		}
		free(text_buffer);

		//	No, so find a spare map structure.
		new_map	=	alloc_map();
		if(new_map)
		{
			//	Set up the new map structure.
			strcpy(game_maps[new_map].MapName,"Data\\");
			strcat(game_maps[new_map].MapName,map_file_name);

			//	Now add it to the workspace root.
			new_element	=	new WSElement;
			if(new_element)
			{
				tv_is.hParent				=	root_item.TreeItem;
				tv_is.hInsertAfter			=	TVI_LAST;
				tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
				tv_is.item.iImage			=	IM_MAP;
				tv_is.item.iSelectedImage	=	IM_MAP;
				tv_is.item.pszText			=	map_file_name;
				tv_is.item.lParam			=	(LPARAM)new_element;
				new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
				new_element->ElementType	=	ET_MAP;
				new_element->MapRef			=	new_map;

				//	Expand the root if this is the first map to be added.
				item_count	=	TreeView_GetCount(ws_tree);
				if(item_count<=2)
					TreeView_Expand(ws_tree,root_item.TreeItem,TVE_EXPAND);

				//	Update the window.
				InvalidateRect(ws_tree, NULL, FALSE);
			}
		}
		else
		{
			//	Error no more maps.
			MessageBox	(
							NULL,
							"Run out of Map structures.\n\nClick OK, save your work & then tell Guy.",
							"Oh bugger!!",
							MB_OK
						);
		}
	}
}

//---------------------------------------------------------------

BOOL CALLBACK new_mish_proc	(
								HWND hWnd,
								UINT message,
								WPARAM wParam,
								LPARAM lParam
							)
{
	switch(message)
	{
		case	WM_INITDIALOG:
			//	Make sure the edit control has the focus.
			SetFocus(GetDlgItem(hWnd,IDC_EDIT_MNAME));
			return	FALSE;

		case	WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case	IDOK:
					EndDialog(hWnd,TRUE);
					return	TRUE;

				case	IDCANCEL:
					EndDialog(hWnd,FALSE);
					return	TRUE;

				case	IDC_EDIT_MNAME:
					if(HIWORD(wParam)==EN_CHANGE)
					{
						//	The user has changed the text, if there's any
						//	text in the control enable the 'Ok' button else
						//	disable it.
						if(GetWindowText((HWND)lParam,mission_name,_MAX_PATH))
							EnableWindow(GetDlgItem(hWnd,IDOK),TRUE);
						else
							EnableWindow(GetDlgItem(hWnd,IDOK),FALSE);
						return	TRUE;
					}
					break;
			}
			break;
	}
	return	FALSE;
}

//---------------------------------------------------------------

void	ws_new_mission(void)
{
	UWORD				new_mission;
	HTREEITEM			map_handle;
	TV_INSERTSTRUCT		tv_is;
	TV_ITEM				map_item;
	WSElement			*map_element,
						*new_element;


	//	First get the parent map info.
	map_handle		=	TreeView_GetSelection(ws_tree);
	map_item.hItem	=	map_handle;
	map_item.mask	=	TVIF_PARAM;
	TreeView_GetItem(ws_tree,&map_item);
	map_element		=	(WSElement*)map_item.lParam;

	//	Now find a spare mission structure.
	new_mission	=	alloc_mission(map_element->MapRef);
	if(new_mission)
	{
		//	Bring up a dialog box for the user to enter the mission name.
		if	(
				DialogBox	(
								GEDIT_hinstance,
								MAKEINTRESOURCE(IDD_NEWMISSION),
								ws_tree,
								(DLGPROC)new_mish_proc
							)
			)
		{
			//	Set up the mission structure.
			init_mission(new_mission,mission_name);

			//	Now create the new mission.
			new_element	=	new WSElement;
			if(new_element)
			{
				tv_is.hParent				=	map_handle;
				tv_is.hInsertAfter			=	TVI_LAST;
				tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
				tv_is.item.iImage			=	IM_MISSION;
				tv_is.item.iSelectedImage	=	IM_MISSION;
				tv_is.item.pszText			=	mission_name;
				tv_is.item.lParam			=	(LPARAM)new_element;
				new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
				new_element->ElementType	=	ET_MISSION;
				new_element->MapRef			=	map_element->MapRef;
				new_element->MissionRef		=	new_mission;

				//	Update the window.
				InvalidateRect(ws_tree, NULL, FALSE);
			}
		}
	}
	else
	{
		//	Error no more missions.
		MessageBox	(
						NULL,
						"Run out of Mission structures.\n\nClick OK, save your work & then tell Guy.",
						"Oh bugger!!",
						MB_OK
					);
	}
}

//---------------------------------------------------------------

void	ws_del_mission(void) {
	HTREEITEM current = TreeView_GetSelection(ws_tree);
	SLONG c1;
	
	for(c1=0;c1<MAX_EVENTPOINTS;c1++)
		if (current_mission->EventPoints[c1].Used)
			free_eventpoint(&current_mission->EventPoints[c1]);

	ZeroMemory(current_mission,sizeof(Mission));
	current_mission=NULL; // hmm

	remove_children(current);

}

//---------------------------------------------------------------

void	ws_add_light_map(void)
{
	CBYTE				temp[_MAX_PATH];
	OPENFILENAME		open_map;
	TV_INSERTSTRUCT		tv_is;
	WSElement			*new_element;


	//	Set up the open file structure.
	sprintf(current_mission->LightMapName,"*.lgt");
	ZeroMemory(&open_map,sizeof(OPENFILENAME));
	open_map.lStructSize		=	sizeof(OPENFILENAME);
	open_map.hwndOwner			=	NULL;
	open_map.lpstrFilter		=	"Light Map files\0*.lgt\0\0";
//	open_map.lpstrFile			=	current_mission->LightMapName;
//	open_map.nMaxFile			=	_MAX_PATH;
	open_map.lpstrFileTitle		=	temp;
	open_map.nMaxFileTitle		=	_MAX_PATH;
	open_map.lpstrInitialDir	=	map_default_dir;
	open_map.lpstrTitle			=	"Load a light map";
	open_map.Flags				=	OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	open_map.lpstrDefExt		=	"lgt";

	if(GetOpenFileName(&open_map))
	{
		strcpy(current_mission->LightMapName,"Data\\Lighting\\");
		strcat(current_mission->LightMapName,temp);

		//	Add it to the mission root.
		new_element	=	new WSElement;
		if(new_element)
		{
			tv_is.hParent				=	TreeView_GetSelection(ws_tree);
			tv_is.hInsertAfter			=	TVI_LAST;
			tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
			tv_is.item.iImage			=	IM_LMAP;
			tv_is.item.iSelectedImage	=	IM_LMAP;
			tv_is.item.pszText			=	temp;
			tv_is.item.lParam			=	(LPARAM)new_element;
			new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
			new_element->ElementType	=	ET_LMAP;

			//	Update the window.
			InvalidateRect(ws_tree, NULL, FALSE);
		}
	}
}

//---------------------------------------------------------------

void	ws_add_citsez_map(void)
{
	CBYTE				temp[_MAX_PATH];
	OPENFILENAME		open_map;
	TV_INSERTSTRUCT		tv_is;
	WSElement			*new_element;


	//	Set up the open file structure.
	sprintf(current_mission->CitSezMapName,"*.txt");
	ZeroMemory(&open_map,sizeof(OPENFILENAME));
	open_map.lStructSize		=	sizeof(OPENFILENAME);
	open_map.hwndOwner			=	NULL;
	open_map.lpstrFilter		=	"Text files\0*.txt\0\0";
	open_map.lpstrFile			=	current_mission->CitSezMapName;
	open_map.nMaxFile			=	_MAX_PATH;
	open_map.lpstrFileTitle		=	temp;
	open_map.nMaxFileTitle		=	_MAX_PATH;
	open_map.lpstrInitialDir	=	"c:\\fallen\\text\\";
	open_map.lpstrTitle			=	"Load a Citizen-Sez text file";
	open_map.Flags				=	OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	open_map.lpstrDefExt		=	"txt";

	if(GetOpenFileName(&open_map))
	{
		strcpy(current_mission->CitSezMapName,"Text\\");
		strcat(current_mission->CitSezMapName,temp);

		//	Add it to the mission root.
		new_element	=	new WSElement;
		if(new_element)
		{
			tv_is.hParent				=	TreeView_GetSelection(ws_tree);
			tv_is.hInsertAfter			=	TVI_LAST;
			tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
			tv_is.item.iImage			=	IM_SEWER;
			tv_is.item.iSelectedImage	=	IM_SEWER;
			tv_is.item.pszText			=	temp;
			tv_is.item.lParam			=	(LPARAM)new_element;
			new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
			new_element->ElementType	=	ET_LMAP;

			//	Update the window.
			InvalidateRect(ws_tree, NULL, FALSE);
		}
	}
}


//---------------------------------------------------------------

BOOL	create_workspace(void)
{
	int					result;
	CBYTE				temp[_MAX_PATH];
	OPENFILENAME		save_workspace;
	TV_ITEM				set_item;


	if(workspace_changed)
	{
		//	Ask the user to save etc.
		result	=	MessageBox	(
									NULL,
									"Changes have been made to the current workspace.\nDo you want to save?",
									"Urban Chaos Mission Editor",
									MB_ICONQUESTION|MB_YESNOCANCEL
								);
		if(result==IDCANCEL)
		{
			return	FALSE;
		}
		else if(result==IDYES)
		{
//			save_workspace();
		}
	}

	//	Set up the default directory & file name.
	sprintf(workspace_path,"*.ucw");

	//	Set up the save file structure.
	ZeroMemory(&save_workspace,sizeof(OPENFILENAME));
	save_workspace.lStructSize		=	sizeof(OPENFILENAME);
	save_workspace.hwndOwner		=	NULL;
	save_workspace.lpstrFilter		=	"Editor Workspace Files\0*.ucw\0\0";
	save_workspace.lpstrFile		=	workspace_path;
	save_workspace.nMaxFile			=	_MAX_PATH;
	save_workspace.lpstrFileTitle	=	workspace_name;
	save_workspace.nMaxFileTitle	=	_MAX_PATH;
	save_workspace.lpstrInitialDir	=	workspace_dir;
	save_workspace.lpstrTitle		=	"Create a new Workspace";
	save_workspace.Flags			=	OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	save_workspace.lpstrDefExt		=	"ucw";

	if(GetOpenFileName(&save_workspace))
	{
		//	Change the root item.
		sprintf(temp,"Workspace : %s",workspace_name);
		set_item.mask			=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
		set_item.hItem			=	root_item.TreeItem;
		set_item.pszText		=	temp;
		set_item.iImage			=	IM_ROOT;
		set_item.iSelectedImage	=	IM_ROOT;
		root_item.ElementType	=	ET_ROOT;
		TreeView_SetItem(ws_tree,&set_item);

		//	Set the workspace state & corresponding menus.
		workspace_changed	=	TRUE;
		menu_has_workspace();
	}

	return	TRUE;
}

//---------------------------------------------------------------

BOOL	close_workspace(void)
{
	int					result;
	UWORD				c0,c1;
	Mission				*the_mission;
	TV_ITEM				set_item;

	
	if(workspace_changed)
	{
		//	Ask the user to save etc.
		result	=	MessageBox	(
									NULL,
									"Changes have been made to the workspace.\nDo you want to save?",
									"Urban Chaos Mission Editor",
									MB_ICONQUESTION|MB_YESNOCANCEL
								);
		if(result==IDCANCEL)
		{
			return	FALSE;
		}
		else if(result==IDYES)
		{
			save_workspace();
		}
	}

	//	Remove eveything from the tree.
	remove_children(TreeView_GetRoot(ws_tree));

	//	Go thru' the missions & clear out ep data.
	for(c0=0;c0<MAX_MISSIONS;c0++)
	{
		if(mission_pool[c0].Flags & MISSION_FLAG_USED)
		{
			the_mission	=	&mission_pool[c0];
			for(c1=0;c1<MAX_EVENTPOINTS;c1++)
				if (the_mission->EventPoints[c1].Used)
					free_eventpoint(&the_mission->EventPoints[c1]);
		}
	}
	MISSION_init();

	//	Reset the root item.
	set_item.mask			=	TVIF_TEXT;
	set_item.hItem			=	root_item.TreeItem;
	set_item.pszText		=	"Workspace : NONE";
	root_item.ElementType	=	ET_NONE;
	TreeView_SetItem(ws_tree,&set_item);

	//	Setup the workspace state & corresponding menus.
	workspace_changed	=	FALSE;
	menu_no_workspace();

	return	TRUE;
}

//---------------------------------------------------------------

BOOL load_workspace(BOOL try_loading_default)
{
	int					result;
	CBYTE				temp[_MAX_PATH];
	UBYTE				gm_vers;
	UWORD				item_count;
	ULONG				c0,c1,
						count,
						ep_count,
						size;
	FILE				*file_handle;
	HTREEITEM			current_item,
						map_handle,
						mission_handle;
	OPENFILENAME		load_workspace;
	TV_INSERTSTRUCT		tv_is;
	TV_ITEM				set_item;
	WSElement			*map_element,
						*new_element;


	if(workspace_changed)
	{
		//	Ask the user to save etc.
		result	=	MessageBox	(
									NULL,
									"Changes have been made to the current workspace.\nDo you want to save?",
									"Urban Chaos Mission Editor",
									MB_ICONQUESTION|MB_YESNOCANCEL
								);
		if(result==IDCANCEL)
		{
			return	FALSE;
		}
		else if(result==IDYES)
		{
			save_workspace();
		}
	}

	//	Clean up.
	close_workspace();

	workspace_path[0] = '\000';

	CBYTE curr_directory[_MAX_PATH];

	GetCurrentDirectory(_MAX_PATH, curr_directory);

	if (try_loading_default)
	{
		FILE *handle = fopen("last_workspace.ini", "rb");

		if (handle)
		{
			fread(workspace_path, 1, _MAX_PATH, handle);

			fclose(handle);
		}
	}

	if (workspace_path[0] == '\000')
	{
		//	Set up the default directory & file name.
		sprintf(workspace_path,"*.ucw");

		//	Set up the save file structure.
		ZeroMemory(&load_workspace,sizeof(OPENFILENAME));
		load_workspace.lStructSize		=	sizeof(OPENFILENAME);
		load_workspace.hwndOwner		=	NULL;
		load_workspace.lpstrFilter		=	"Editor Workspace Files\0*.ucw\0\0";
		load_workspace.lpstrFile		=	workspace_path;
		load_workspace.nMaxFile			=	_MAX_PATH;
		load_workspace.lpstrFileTitle	=	workspace_name;
		load_workspace.nMaxFileTitle	=	_MAX_PATH;
		load_workspace.lpstrInitialDir	=	workspace_dir;
		load_workspace.lpstrTitle		=	"Open a Workspace";
		load_workspace.Flags			=	OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
		load_workspace.lpstrDefExt		=	"ucw";

		if (!GetOpenFileName(&load_workspace))
		{
			return FALSE;
		}
	}

	//	Load the workspace.

	SetCurrentDirectory(curr_directory);

	{
		file_handle	=	fopen(workspace_path,"rb");

		if(file_handle)
		{
			{
				//
				// Make this the default workspace to load next time.
				//

				FILE *handle = fopen("last_workspace.ini", "wb");

				if (handle)
				{
					fwrite(workspace_path, 1, _MAX_PATH, handle);

					fclose(handle);
				}
			}

			//	Read the file version.
			fread(&gm_vers,sizeof(gm_vers),1,file_handle);

			//	Read the number of game maps & the structure size.
			fread(&count,sizeof(count),1,file_handle);
			fread(&size,sizeof(size),1,file_handle);

			//	Read in the maps.
			for(c0=0;c0<count;c0++)
			{
				fread(&game_maps[c0],size,1,file_handle);

				//	If map exists add it to the workspace tree.
				if(game_maps[c0].Used)
				{
					new_element	=	new WSElement;
					if(new_element)
					{
						tv_is.hParent				=	root_item.TreeItem;
						tv_is.hInsertAfter			=	TVI_LAST;
						tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
						tv_is.item.iImage			=	IM_MAP;
						tv_is.item.iSelectedImage	=	IM_MAP;
						tv_is.item.pszText			=	&game_maps[c0].MapName[5];	//	Assumes name always has 'Data\' preceding it.
						tv_is.item.lParam			=	(LPARAM)new_element;
						new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
						new_element->ElementType	=	ET_MAP;
						new_element->MapRef			=	(UWORD)c0;

						//	Expand the root if this is the first map to be added.
						item_count	=	TreeView_GetCount(ws_tree);
						if(item_count<=2)
							TreeView_Expand(ws_tree,root_item.TreeItem,TVE_EXPAND);
					}
				}
			}

			//	Read the number of missions, number of event points & the structure size.
			fread(&count,sizeof(count),1,file_handle);
			fread(&ep_count,sizeof(ep_count),1,file_handle);
			fread(&size,sizeof(size),1,file_handle);

			//	Read the missions.
			for(c0=0;c0<count;c0++)
			{
				//	Suck in the mission. Blank it first as 'old' missions are shorter
				ZeroMemory(&mission_pool[c0],sizeof(mission_pool[c0]));
				fread(&mission_pool[c0],size,1,file_handle);

				ResetFreelist(&mission_pool[c0]);
				ResetUsedlist(&mission_pool[c0]);
				ResetFreepoint(&mission_pool[c0]);
				ResetUsedpoint(&mission_pool[c0]);

				//	If mission is used then do some additional stuff.
				if(mission_pool[c0].Flags & MISSION_FLAG_USED)
				{
					//  Fix changed stuff:
					if (gm_vers<4) {
						EventPoint *ep;
						for (c1=0,ep=mission_pool[c0].EventPoints;c1<ep_count;c1++,ep++) {
							if (ep->WaypointType==WPT_CREATE_ENEMIES) {
								ep->Data[0]=MAKELONG(ep->Data[0],ep->Data[1]);
								ep->Data[1]=0;
							}
						}
					}
//					if (gm_vers<10) mission_pool[c0].BoredomRate=4;
					if (!mission_pool[c0].BoredomRate) mission_pool[c0].BoredomRate=4;
					//	Set up eventpoint data areas.
					for(c1=0;c1<ep_count;c1++)
						read_event_extra(file_handle,&mission_pool[c0].EventPoints[c1],mission_pool[c0].EventPoints,gm_vers);

					//	Find the map tree item that's going to be the missions parent.
					map_handle	=	NULL;
					current_item	=	TreeView_GetChild(ws_tree,TreeView_GetRoot(ws_tree));
					while(current_item)
					{
						set_item.hItem	=	current_item;
						set_item.mask	=	TVIF_PARAM;
						TreeView_GetItem(ws_tree,&set_item);
						map_element		=	(WSElement*)set_item.lParam;
						if(map_element && mission_pool[c0].MapIndex==map_element->MapRef)
						{
							map_handle	=	current_item;
							break;
						}
						current_item	=	TreeView_GetNextSibling(ws_tree,current_item);
					}

					//	Now add it to the tree.
					if(map_element && map_handle)
					{
						new_element	=	new WSElement;
						if(new_element)
						{
							tv_is.hParent				=	map_handle;
							tv_is.hInsertAfter			=	TVI_LAST;
							tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
							tv_is.item.iImage			=	IM_MISSION;
							tv_is.item.iSelectedImage	=	IM_MISSION;
							tv_is.item.pszText			=	mission_pool[c0].MissionName;
							tv_is.item.lParam			=	(LPARAM)new_element;
							new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
							new_element->ElementType	=	ET_MISSION;
							new_element->MapRef			=	map_element->MapRef;
							new_element->MissionRef		=	(UWORD)c0;
							mission_handle				=	new_element->TreeItem;
						}

						//	Add the light map.
						if(mission_pool[c0].LightMapName[0])
						{
							new_element	=	new WSElement;
							if(new_element)
							{
								tv_is.hParent				=	mission_handle;
								tv_is.hInsertAfter			=	TVI_LAST;
								tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
								tv_is.item.iImage			=	IM_LMAP;
								tv_is.item.iSelectedImage	=	IM_LMAP;
								tv_is.item.pszText			=	&mission_pool[c0].LightMapName[5];	//	Assumes name always has 'Data\' preceding it.
								tv_is.item.lParam			=	(LPARAM)new_element;
								new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
								new_element->ElementType	=	ET_LMAP;
							}
						}

						//	Add the sewer map.
						if(mission_pool[c0].CitSezMapName[0])
						{
							new_element	=	new WSElement;
							if(new_element)
							{
								tv_is.hParent				=	mission_handle;
								tv_is.hInsertAfter			=	TVI_LAST;
								tv_is.item.mask				=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_PARAM;
								tv_is.item.iImage			=	IM_SEWER;
								tv_is.item.iSelectedImage	=	IM_SEWER;
								tv_is.item.pszText			=	&mission_pool[c0].CitSezMapName[5];
								tv_is.item.lParam			=	(LPARAM)new_element;
								new_element->TreeItem		=	TreeView_InsertItem(ws_tree,&tv_is);
								new_element->ElementType	=	ET_LMAP;
							}
						}
					}
				}

				if (gm_vers>=3) { // includes zoning info
					fread(MissionZones[c0],128*128,1,file_handle);
				}
			}
			
			//	Force a redraw on the workspace tree.
			InvalidateRect(ws_tree, NULL, FALSE);
			
			//	Change the root item.
			sprintf(temp,"Workspace : %s",workspace_name);
			set_item.mask			=	TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
			set_item.hItem			=	root_item.TreeItem;
			set_item.pszText		=	temp;
			set_item.iImage			=	IM_ROOT;
			set_item.iSelectedImage	=	IM_ROOT;
			root_item.ElementType	=	ET_ROOT;
			TreeView_SetItem(ws_tree,&set_item);

			//	Set the workspace state & corresponding menus.
			workspace_changed	=	FALSE;
			menu_has_workspace();

			fclose(file_handle);

			return TRUE;
		}
	}
	
	return FALSE;
}

//---------------------------------------------------------------

#define	GM_VERSION	8

BOOL	save_workspace(void)
{
	UBYTE		gm_vers;
	ULONG		c0,c1,
				data;
	FILE		*file_handle;
//	EventPoint	*ep_base;


	//	Set the 'busy' cursor.
	SetCursor(GEDIT_busy);


	//	Save the workspace.
	file_handle	=	fopen(workspace_path,"wb");

	if(file_handle)
	{
		//	Save the file version.
		gm_vers	=	GM_VERSION;
		fwrite(&gm_vers,sizeof(gm_vers),1,file_handle);

		//	Save the number of game maps & the structure size.
		data	=	MAX_MAPS;
		fwrite(&data,sizeof(data),1,file_handle);
		data	=	sizeof(GameMap);
		fwrite(&data,sizeof(data),1,file_handle);

		//	Save the maps.
		fwrite(&game_maps,sizeof(GameMap),MAX_MAPS,file_handle);

		//	Save the number of missions, the number of event points & the structure size.
		data	=	MAX_MISSIONS;
		fwrite(&data,sizeof(data),1,file_handle);
		data	=	MAX_EVENTPOINTS;
		fwrite(&data,sizeof(data),1,file_handle);
		data	=	sizeof(Mission);
		fwrite(&data,sizeof(data),1,file_handle);

		//	Save the missions.
		for(c0=0;c0<MAX_MISSIONS;c0++)
		{
			//	Spit out the mission.
			fwrite(&mission_pool[c0],sizeof(Mission),1,file_handle);

			//	Save out the text.
			for(c1=0;c1<MAX_EVENTPOINTS;c1++)
				write_event_extra(file_handle,&mission_pool[c0].EventPoints[c1]);

			// Write out the zoning information
			fwrite((void*)MissionZones[c0],128*128,1,file_handle);

		}
				
		fclose(file_handle);
	}

	//	Set the 'arrow' cursor.
	SetCursor(GEDIT_arrow);

	workspace_changed	=	FALSE;
	menu_workspace_saved();

	return	TRUE;
}

//---------------------------------------------------------------

void	remove_children(HTREEITEM parent)
{
	HTREEITEM		current_item,
					next_item;
	TVITEM			item_info;


	if(parent)
	{
		//	Sort out its children.
		current_item	=	TreeView_GetChild(ws_tree,parent);
		while(current_item)
		{
			next_item	=	TreeView_GetNextSibling(ws_tree,current_item);
			remove_children(current_item);
			current_item	=	next_item;
		}
		
		//	Delete the parent only if it isn't the root.
		if(parent!=TreeView_GetRoot(ws_tree))
		{
			//	Get the items WSElement.
			item_info.hItem	=	parent;
			item_info.mask	=	TVIF_PARAM;
			TreeView_GetItem(ws_tree,&item_info);

			//	Delete the WSElement.
			delete	(WSElement*)item_info.lParam;

			//	Delete the tree item.
			TreeView_DeleteItem(ws_tree,parent);
		}
	}
}

//---------------------------------------------------------------
// Waypointy stuff


BOOL	init_wptlist(HWND parent)
{

	//	Subclass the tree control windproc.
	wpt_tree	=	GetDlgItem(parent,IDC_WORKSPACE_TREE2);
//	tree_proc	=	(WNDPROC)SetWindowLong(ws_tree,GWL_WNDPROC,(long)sc_tree_proc);

	//	Set up the trees image list.
/*	image_list	=	ImageList_LoadBitmap(
											GEDIT_hinstance,
											MAKEINTRESOURCE(IDB_WORKSPACE),
											16,
											1,
											RGB (255, 0, 255)
										);
*/
	TreeView_SetImageList(wpt_tree,image_list,TVSIL_NORMAL);
	TreeView_SetImageList(wpt_tree,image_list,TVSIL_STATE);

	reset_wptlist();

	return	TRUE;
}

void	reset_wptlist(void) {

	//	Set up the tree roots.
	TreeView_DeleteAllItems(wpt_tree);
/*	ws_root_waypoint("Map exits", IM_MAPEXIT,6);
	ws_root_waypoint("Misc", IM_WAYPOINT,5);
	ws_root_waypoint("Traps", IM_TRAP,4);
	ws_root_waypoint("Cameras", IM_CAMERA,3);
	ws_root_waypoint("Items", IM_ITEM,2);
	ws_root_waypoint("Enemies", IM_COP,1);
	ws_root_waypoint("Create Player", IM_PLAYER,0);
	*/
}


void	fill_wptlist(Mission *mish) {
	SLONG ndx=mish->UsedEPoints;
	EventPoint *ep_base = mish->EventPoints;

	EventPoint *ep;

	while(ndx) {
		ep = TO_EVENTPOINT(ep_base,ndx);
		ws_add_waypoint(ep);
		ndx= ep->Next;
	}
	
}


void	fini_wptlist(void)
{
	
}

//---------------------------------------------------------------


HTREEITEM ws_root_waypoint(CBYTE *msg, SLONG type, LPARAM param) {
	TV_INSERTSTRUCT		tv_is;

	if (msg[0]==0) {
		switch(param) {
		case 0:
			strcpy(msg,"Create Player");
			type=IM_PLAYER;
			break;
		case 1:
			strcpy(msg,"Enemies");
			type=IM_COP;
			break;
		case 2:
			strcpy(msg,"Items");
			type=IM_ITEM;
			break;
		case 3:
			strcpy(msg,"Traps");
			type=IM_TRAP;
			break;
		case 4:
			strcpy(msg,"Cameras");
			type=IM_CAMERA;
			break;
		case 5:
			strcpy(msg,"Misc");
			type=IM_WAYPOINT;
			break;
		case 6:
			strcpy(msg,"Map exits");
			type=IM_MAPEXIT;
			break;
		case 7:
			strcpy(msg,"Text messages");
			type=IM_MESSAGE;
			break;
		}
	}
	tv_is.hParent				=	TVI_ROOT;
	tv_is.hInsertAfter			=	TVI_SORT;
	tv_is.item.mask				=	TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
	tv_is.item.stateMask		=	TVIS_STATEIMAGEMASK;
	tv_is.item.state			=	0;
	tv_is.item.pszText			=	msg;
	tv_is.item.lParam			=	param;
	tv_is.item.iImage			=	
	  tv_is.item.iSelectedImage = type;
	return TreeView_InsertItem(wpt_tree,&tv_is);
}

HTREEITEM ws_find_child(HTREEITEM parent, LPARAM param) {
	HTREEITEM kid;
	TVITEM item;

	if (parent)
		kid=TreeView_GetChild(wpt_tree,parent);
	else
		kid=TreeView_GetRoot(wpt_tree);
	while (kid) {
		item.mask=TVIF_HANDLE;
		item.hItem=kid;
		TreeView_GetItem(wpt_tree,&item);
		if (item.lParam==param) return kid;
		kid=TreeView_GetNextSibling(wpt_tree,kid);
	}
	return 0;
}

SLONG	ws_image_from_type(EventPoint *ep) {
	switch(ep->WaypointType) {
	case	WPT_NONE:			
	case	WPT_SIMPLE:
	case	WPT_CREATE_VEHICLE:
	case	WPT_VISUAL_EFFECT:
	case	WPT_END_GAME_LOSE:
	case	WPT_END_GAME_WIN:
	case	WPT_SHOUT:
	case	WPT_NAV_BEACON:
	case	WPT_CUT_SCENE:
	case	WPT_TELEPORT:
	case	WPT_TELEPORT_TARGET:
	case	WPT_ACTIVATE_PRIM:
	case	WPT_LINK_PLATFORM:
	case	WPT_BURN_PRIM:
	case	WPT_SPOT_EFFECT:
	case	WPT_INTERESTING:
	case	WPT_MAKE_SEARCHABLE:
	case	WPT_LOCK_VEHICLE:
	case	WPT_STALL_CAR:
	case	WPT_EXTEND:
	case	WPT_MOVE_THING:
	case	WPT_CONE_PENALTIES:
	case	WPT_SIGN:
	case	WPT_NO_FLOOR:
	case	WPT_SHAKE_CAMERA:
		return IM_WAYPOINT;

	case	WPT_CREATE_PLAYER:
		return IM_PLAYER;

	case	WPT_CREATE_ENEMIES:
	case	WPT_ADJUST_ENEMY:
	case	WPT_CREATE_CREATURE:
		return IM_COP;

	case	WPT_MESSAGE:
	case	WPT_CONVERSATION:
		return IM_MESSAGE;

	case	WPT_CREATE_ITEM:
	case	WPT_CREATE_BARREL:
		return IM_ITEM;

	case	WPT_CREATE_CAMERA:
	case	WPT_CREATE_TARGET:
	case	WPT_CAMERA_WAYPOINT:
	case	WPT_TARGET_WAYPOINT:
		return IM_CAMERA;

	case	WPT_CREATE_MAP_EXIT:
		return IM_MAPEXIT;

	case	WPT_SOUND_EFFECT:
	case	WPT_WAREFX:
		return IM_SOUND;

	case	WPT_CREATE_TRAP:
	case	WPT_KILL_WAYPOINT:
		return IM_TRAP;	

	case	WPT_CREATE_BOMB:
		return IM_BOMB;

	default:
		return IM_WAYPOINT;
	}
	return -1;
}

SLONG	ws_category_from_type(EventPoint *ep) {
	switch(ep->WaypointType) {
// misc
	case	WPT_NONE:			
	case	WPT_SIMPLE:
	case	WPT_CREATE_VEHICLE:
	case	WPT_VISUAL_EFFECT:
	case	WPT_END_GAME_WIN:
	case	WPT_END_GAME_LOSE:
	case	WPT_SHOUT:
	case	WPT_CUT_SCENE:
	case	WPT_TELEPORT:
	case	WPT_TELEPORT_TARGET:
	case	WPT_ACTIVATE_PRIM:
	case	WPT_SOUND_EFFECT:
	case	WPT_SPOT_EFFECT:
	case	WPT_DYNAMIC_LIGHT:
	case	WPT_LINK_PLATFORM:
	case	WPT_NAV_BEACON:
	case	WPT_BURN_PRIM:
	case	WPT_KILL_WAYPOINT:
	case	WPT_GROUP_LIFE:
	case	WPT_GROUP_DEATH:
	case	WPT_INTERESTING:
	case	WPT_GOTHERE_DOTHIS:
	case	WPT_MAKE_SEARCHABLE:
	case	WPT_LOCK_VEHICLE:
	case	WPT_GROUP_RESET:
	case	WPT_COUNT_UP_TIMER:
	case	WPT_CREATE_MIST:
	case	WPT_STALL_CAR:
	case	WPT_MOVE_THING:
	case	WPT_EXTEND:
	case	WPT_CONE_PENALTIES:
	case	WPT_SIGN:
	case	WPT_WAREFX:
	case	WPT_SHAKE_CAMERA:
	case	WPT_NO_FLOOR:
		return 5;
// texty things
	case	WPT_MESSAGE:
	case	WPT_CONVERSATION:
		return 7;
// player
	case	WPT_CREATE_PLAYER:
	case	WPT_AUTOSAVE:
		return 0;
// enemies
	case	WPT_CREATE_ENEMIES:
	case	WPT_ADJUST_ENEMY:
	case	WPT_CREATE_CREATURE:
	case	WPT_ENEMY_FLAGS:
		return 1;
// items
	case	WPT_CREATE_ITEM:
	case	WPT_CREATE_BARREL:
	case	WPT_CREATE_TREASURE:
	case	WPT_BONUS_POINTS:
		return 2;
// cam stuff
	case	WPT_CREATE_CAMERA:
	case	WPT_CREATE_TARGET:
	case	WPT_CAMERA_WAYPOINT:
	case	WPT_TARGET_WAYPOINT:
		return 4;
// exits
	case	WPT_CREATE_MAP_EXIT:
		return 6;
// traps
	case	WPT_CREATE_TRAP:
	case	WPT_CREATE_BOMB:
		return 3;	
	default:
		return 5; // misc
	}
	return -1;
}

void	ws_add_waypoint(EventPoint *ep) {
	HTREEITEM			parent;
	SLONG				code;
	TV_INSERTSTRUCT		tv_is;
	CBYTE				msg[800];
	
	if (!ep->Used) return;

	msg[0]=0;
	parent=ws_find_child(0,code=ws_category_from_type(ep));
	if (code==-1) return;
	if (!parent) parent=ws_root_waypoint(msg, 0, code);
	tv_is.hParent				=	parent;
	tv_is.hInsertAfter			=	TVI_SORT;
	tv_is.item.mask				=	TVIF_TEXT|TVIF_PARAM|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
	tv_is.item.stateMask		=	TVIS_STATEIMAGEMASK;
	if (ep->Flags&WPT_FLAGS_SUCKS)
		tv_is.item.state		=	INDEXTOSTATEIMAGEMASK(IM_BROKEN);
	else
		tv_is.item.state		=	0;
	tv_is.item.pszText			=	WaypointTitle(ep, msg);
	tv_is.item.lParam			=	(LPARAM)ep;
	tv_is.item.iImage			=	
	  tv_is.item.iSelectedImage = ws_image_from_type(ep);
	TreeView_InsertItem(wpt_tree,&tv_is);

	// double check -- if this one sucks, it's parent sucks too, to help find suckage.
	if (ep->Flags&WPT_FLAGS_SUCKS) {
		tv_is.item.hItem		=	parent;
		tv_is.item.mask			=	TVIF_HANDLE|TVIF_STATE;
		tv_is.item.stateMask	=	TVIS_STATEIMAGEMASK;
		tv_is.item.state		=	INDEXTOSTATEIMAGEMASK(IM_BROKEN);
		TreeView_SetItem(wpt_tree,&tv_is.item);
	}

}

void	ws_set_waypoint(EventPoint *ep, CBYTE ndx) {
//  HTREEITEM item;
  
//  item=ws_find_child(0,ws_category_from_type(ep));
  if (ep->WaypointType!=ndx) {
	  ws_del_waypoint(ep);
	  ep->WaypointType=ndx;
	  ws_add_waypoint(ep);
  }
/*  if (item) {
		item=ws_find_child(item,(LPARAM)ep);
		if (ep->WaypointType!=ndx) {
			ep->WaypointType=ndx;
			TreeView_DeleteItem(wpt_tree,item);
			ws_add_waypoint(ep);
		}
  }*/
}

void	ws_sel_waypoint(EventPoint *ep) {
	HTREEITEM			item;
	if (!ep) {
		TreeView_SelectItem(wpt_tree,0);
		return;
	}
	item=ws_find_child(0,ws_category_from_type(ep));
	if (item) {
		item=ws_find_child(item,(LPARAM)ep);
		TreeView_SelectItem(wpt_tree,item);
	}
}

void	ws_del_waypoint(EventPoint *ep) {
	HTREEITEM			parent,item;
	parent=ws_find_child(0,ws_category_from_type(ep));
	if (parent) {
		item=ws_find_child(parent,(LPARAM)ep);
		if (item) TreeView_DeleteItem(wpt_tree,item);
		item=TreeView_GetChild(wpt_tree,parent);
		if (!item) TreeView_DeleteItem(wpt_tree,parent);
	}
}

