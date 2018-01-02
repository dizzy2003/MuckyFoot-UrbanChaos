// DDLib.h
// Guy Simmons, 20th November 1997

#ifndef	DDLIB_H
#define	DDLIB_H

#include	<MFStdLib.h>

#include	"D3DTexture.h"
#include	"DDManager.h"
#include	"Debug.h"
#include	"DIManager.h"
#include	"GDisplay.h"
#include	"GWorkScreen.h"
#include    "Net.h"

#include	"WindProcs.h"

#include	"resource.h"

#ifdef TARGET_DC
#include "target.h"
#endif


//---------------------------------------------------------------
// mikes mouse stuff

extern	volatile SLONG		MouseDX,MouseDY;
extern	void	RecenterMouse(void);

//---------------------------------------------------------------

#define	SET_BLACK_BACKGROUND						the_display.SetBlackBackground()
#define	SET_WHITE_BACKGROUND						the_display.SetWhiteBackground()
#define SET_BLUE_BACKGROUND							the_display.SetBlueBackground()
#define	SET_USER_BACKGROUND							the_display.SetUserBackground()

#define	BEGIN_SCENE									the_display.BeginScene()
#define	END_SCENE									the_display.EndScene()
#define	CLEAR_VIEWPORT								the_display.ClearViewport()
#define	FLIP(a,f)									the_display.Flip(a,f)
#define	DRAW_PRIMITIVE(pt,vt,v,vc,f)				the_display.DrawPrimitive(pt,vt,v,vc,f)
#define	DRAW_INDEXED_PRIMITIVE(pt,vt,v,vc,i,ic,f)	the_display.DrawIndexedPrimitive(pt,vt,v,vc,i,ic,f)

// macros to set render state in the card
#define	REALLY_SET_RENDER_STATE(t,s)				the_display.SetRenderState(t,s)
#define REALLY_SET_TEXTURE(tex)						the_display.SetTexture(tex)
#define REALLY_SET_NO_TEXTURE						the_display.SetTexture(NULL)
#define	REALLY_SET_TEXTURE_STATE(n,t,s)				the_display.SetTextureState(n,t,s)

/*
// macros used in code - *usually* just call REALLY_*
#define	SET_RENDER_STATE(t,s)		REALLY_SET_RENDER_STATE(t,s)
#define	SET_TEXTURE(tex)			REALLY_SET_TEXTURE(tex)
#define SET_NO_TEXTURE				REALLY_SET_NO_TEXTURE
#define SET_TEXTURE_STATE(n,t,s)	REALLY_SET_TEXTURE_STATE(n,t,s)
*/

//---------------------------------------------------------------

#ifndef NDEBUG

#ifdef TARGET_DC


#undef  FAILED
#undef	SUCCEEDED
//#define FAILED(f)		(ASSERT((signed)(f)>=0),(signed)(f)<0)
#define FAILED(f)		((signed)(f)<0)
#define SUCCEEDED(f)	((signed)(f)>=0)



#else //#ifdef TARGET_DC


inline SLONG check_result(HRESULT f, SLONG line, CBYTE *file)
{
//	if (f == DD_OK)			// this is an *INCORRECT* error test; see e.g. Petzold "Programming Windows 95"
	if ( f >= 0 )
	{
		return FALSE;
	}
	else
	{
		TRACE("%s(%d):\n\t", file, line);
		LogText("Error in %s line %d\n\t", file, line);
		dd_error(f);

		return TRUE;
	}
}

#undef  FAILED
#undef	SUCCEEDED
#define FAILED(f)		(check_result(f, __LINE__, __FILE__))
#define SUCCEEDED(f)	(!FAILED(f))

#endif //#else //#ifdef TARGET_DC


#endif	// else use Micro$oft macros

#endif

