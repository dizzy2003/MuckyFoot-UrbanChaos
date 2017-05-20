// DSManager.cpp
// Guy Simmons, 22nd February 1998

#include	"DDLib.h"


DSDriverManager		the_sound_manager;

//---------------------------------------------------------------
//
//	Callback functions.
//
//---------------------------------------------------------------


//---------------------------------------------------------------
//
//	CLASS	:	DSDriverManager
//
//---------------------------------------------------------------

DSDriverManager::DSDriverManager()
{
	ManagerFlags		=	0;
}

//---------------------------------------------------------------

DSDriverManager::~DSDriverManager()
{
	Fini();
}

//---------------------------------------------------------------

HRESULT	DSDriverManager::Init(void)
{
	HRESULT			result;


	if(!IsInitialised())
	{
		// Create the DirectSound object.
		result	=	DirectSoundCreate	(
											NULL,
											&lp_DS,
											NULL
										);
		if(FAILED(result))
			return	result;

		// Set the cooperative level.
		lp_DS->SetCooperativeLevel	(
										hDDLibWindow,
//										DSSCL_NORMAL
										DSSCL_EXCLUSIVE
									);
		if(FAILED(result))
			return	result;

		InitOn();
	}
	return	DS_OK;
}

//---------------------------------------------------------------

HRESULT	DSDriverManager::Fini(void)
{
	if(IsInitialised())
	{
		// Release DirectSound.
		lp_DS->Release();

		InitOff();
	}
	return	DS_OK;
}

//---------------------------------------------------------------
