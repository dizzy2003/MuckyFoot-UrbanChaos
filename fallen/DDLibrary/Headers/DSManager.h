// DSManager.h
// Guy Simmons, 22nd February 1998.

#ifndef	DSMANAGER_H
#define	DSMANAGER_H

//---------------------------------------------------------------

#define	DS_DRIVER_INIT			(1<<0)

#define	DS_DRIVER_VALID			(1<<0)

//---------------------------------------------------------------

class	DSDriverManager
{
	private:
	protected:
	public:
		ULONG				ManagerFlags;				// Global flags
		LPDIRECTSOUND		lp_DS;						// The DirectSound object.

			
							DSDriverManager();
							~DSDriverManager();

		// Methods.
		HRESULT				Init(void);
		HRESULT				Fini(void);


		inline	BOOL		IsInitialised(void)			{	return	ManagerFlags&DS_DRIVER_INIT;		}
		inline	void		InitOn(void)				{	ManagerFlags	|=	DS_DRIVER_INIT;			}
		inline	void		InitOff(void)				{	ManagerFlags	&=	~DS_DRIVER_INIT;		}

		inline	HRESULT		CreateSoundBuffer(LPCDSBUFFERDESC desc,LPLPDIRECTSOUNDBUFFER buff,IUnknown FAR * u)
														{
															return	lp_DS->CreateSoundBuffer(desc,buff,u);
														}

};

extern DSDriverManager		the_sound_manager;

//---------------------------------------------------------------

#endif

