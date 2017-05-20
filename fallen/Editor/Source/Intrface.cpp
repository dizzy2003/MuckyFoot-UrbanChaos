// Intrface.cpp
// Guy Simmons, 26th October 1996.

#include	"Editor.hpp"

#ifdef	EDITOR

//****************************************************************************

Interface::Interface()
{
	interface_sprite_data	=	0;
	interface_sprites		=	0;
}

//****************************************************************************

Interface::~Interface()
{
	if(interface_sprite_data)
		MemFree(interface_sprite_data);
	if(interface_sprites)
		MemFree(interface_sprites);
}

//****************************************************************************

void Interface::SetupInterfaceDefaults(void)
{
	SLONG			blue,
					green,
					red;
	SLONG			file_size;
	MFFileHandle	file_handle;


	memset(InterfacePalette,0,3*256);
	FileLoadAt("Editor\\Data\\palette.pal",InterfacePalette);


#ifdef	_DEBUG
	{
		SLONG	c0;
		for(c0=0;c0<256*3;c0++)
		{
			LogText(" pal c0 %d \n",c0);
			red				=	InterfacePalette[(c0)];
			red+=32;
			if(red>255)
				red=255;
			InterfacePalette[(c0)]=red;
		}
	}
#endif
	SetPalette(InterfacePalette);

   	ContentColourBr	=	FindColour(InterfacePalette,115+20,128+20,156+20);
   	ContentColour	=	FindColour(InterfacePalette,115,128,156);
	TextColour		=	FindColour(InterfacePalette,256,256,256);
	HiliteColour	=	FindColour(InterfacePalette,115+30,128+30,156+30);
	LoliteColour	=	FindColour(InterfacePalette,115-20,128-20,156-20);
	SelectColour	=	FindColour(InterfacePalette,115+10,128+10,156+10);
	ActiveColour	=	FindColour((UBYTE*)InterfacePalette,130,130,130);
	WhiteColour		=	FindColour((UBYTE*)InterfacePalette,255,255,255);
	GreyColour		=	FindColour((UBYTE*)InterfacePalette,128,128,128);
	YellowColour	=	FindColour((UBYTE*)InterfacePalette,255,255,0);
	RedColour		=	FindColour((UBYTE*)InterfacePalette,255,0,0);
	GreenColour		=	FindColour((UBYTE*)InterfacePalette,0,255,0);
	BlueColour		=	FindColour((UBYTE*)InterfacePalette,0,0,255);

	InactiveColour	=	ContentColour;

	file_handle	=	FileOpen("Editor\\Data\\intrface.spr");
	ERROR_MSG(!(file_handle==FILE_OPEN_ERROR),"Can't open sprite ref file.");
	if(file_handle!=FILE_OPEN_ERROR)
	{
		file_size	=	FileSize(file_handle);
		interface_sprites	=	(BSprite*)MemAlloc(file_size);
		if(interface_sprites)
		{
			FileRead(file_handle,interface_sprites,file_size);
		}
		FileClose(file_handle);
	}

	file_handle	=	FileOpen("Editor\\Data\\intrface.spd");
	ERROR_MSG(!(file_handle==FILE_OPEN_ERROR),"Can't open sprite data file.");
	if(file_handle!=FILE_OPEN_ERROR)
	{
		file_size	=	FileSize(file_handle);
		interface_sprite_data	=	(UBYTE*)MemAlloc(file_size);
		if(interface_sprite_data)
		{
			FileRead(file_handle,interface_sprite_data,file_size);
		}
		FileClose(file_handle);
	}

	if(interface_sprites && interface_sprite_data)
		SetupBSprites(interface_sprites,interface_sprite_data);	
/*
	{
		Alert			*quit_alert;
		quit_alert		=	new	Alert;
		editor_status	=	!quit_alert->HandleAlert("Gourad People|Stand on Feet");
		delete	quit_alert;
	}
*/
	
/*
	red				=	58;
	green			=	58;
	blue			=	58;
	ActiveColour	=	20; //FindColor(palette,red,green,blue);

	InactiveColour	=	ContentColour;

	interface_data[0].Start	=  (void**)&interface_sprites;
	interface_data[0].SEnd	=  (void**)&end_interface_sprites;
	interface_data[1].Start	=  (void**)&interface_sprite_data;
	interface_data[2].Start	=  (void**)&interface_pointers;
	interface_data[2].SEnd	=  (void**)&end_interface_pointers;
	interface_data[3].Start	=  (void**)&interface_pointer_data;
	interface_data[4].Start	=  (void**)&interface_font;
	interface_data[4].SEnd	=  (void**)&end_interface_font;
	interface_data[5].Start	=  (void**)&interface_font_data;
	LoadAllData(&interface_data[0]);

	setup_interface_sprites[0].Start	=	&interface_sprites;
	setup_interface_sprites[0].End		=	&end_interface_sprites;
	setup_interface_sprites[0].Data		=	&interface_sprite_data;
	setup_interface_sprites[1].Start	=	&interface_pointers;
	setup_interface_sprites[1].End		=	&end_interface_pointers;
	setup_interface_sprites[1].Data		=	&interface_pointer_data;
	setup_interface_sprites[2].Start	=	&interface_font;
	setup_interface_sprites[2].End		=	&end_interface_font;
	setup_interface_sprites[2].Data		=	&interface_font_data;
	SetupAllSprites(setup_interface_sprites);

	SetFont(interface_font);
*/
}

//****************************************************************************

Interface	*InterfaceDefaults;

//****************************************************************************

#endif
