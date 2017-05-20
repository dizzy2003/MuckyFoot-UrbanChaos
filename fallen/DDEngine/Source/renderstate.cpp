// renderstate.cpp
//
// RenderState class

#include <MFStdLib.h>
#include <DDLib.h>
#include <math.h>
#include "poly.h"
#include "env.h"

#include "renderstate.h"





#ifdef TARGET_DC
// Not sure whether to have trilinear or nearest - spped is not much different
// at the moment, but may be important later.
#define MIPMAP_MODE_PLEASE_BOB D3DTFP_LINEAR
//#define MIPMAP_MODE_PLEASE_BOB D3DTFP_POINT
#else
// PC doesn't want any mipmapping.
#define MIPMAP_MODE_PLEASE_BOB D3DTFP_NONE
#endif



// s_State
//
// current state of the 3D card

RenderState	RenderState::s_State;

#ifdef EDITOR
bool	RenderState::AllowFadeOut = false;
#endif




_inline DWORD FloatAsDword ( float fArg )
{
	return ( *((DWORD *)(&fArg)) );
}




// config handlers

#ifndef TARGET_DC
int UseDX5Fix = 0;

void RS_read_config()
{	
	UseDX5Fix = ENV_get_value_number("fix_directx", 0, "Render");
}

void RS_get_config(int* fix_directx)
{
	*fix_directx = UseDX5Fix;
}

void RS_set_config(int fix_directx)
{
	ENV_set_value_number("fix_directx", fix_directx, "Render");
	RS_read_config();
}
#endif

// RenderState
//
// construct a default render state

RenderState::RenderState(DWORD mag_filter, DWORD min_filter)
{
	TextureMap = NULL;
	TextureAddress = D3DTADDRESS_CLAMP;
	TextureMag = mag_filter;
	TextureMin = min_filter;
	TextureMapBlend = D3DTBLEND_MODULATE;

#ifndef TARGET_DC
	ZEnable = D3DZB_TRUE;
	ZWriteEnable = TRUE;
	ZFunc = D3DCMP_LESSEQUAL;
	ColorKeyEnable = FALSE;
#endif
	FogEnable = TRUE;
	AlphaTestEnable = FALSE;
	SrcBlend = D3DBLEND_ONE;
	DestBlend = D3DBLEND_ZERO;
	AlphaBlendEnable = FALSE;
	CullMode = D3DCULL_NONE;

//md	FogEnable = FALSE;

	ZBias = 0;
	WrapOnce = false;

#ifndef TARGET_DC
	TempTransparent = false;
	TempSrcBlend = 0;
	TempDestBlend = 0;
	TempAlphaBlendEnable = 0;
	TempZWriteEnable = 0;
	TempTextureMapBlend = 0;
#endif

	Effect = RS_None;
}

// SetTempTransparent
//
// set to be transparent for this cycle only

#ifndef TARGET_DC
void RenderState::SetTempTransparent()
{
	if (!TempTransparent)
	{
		TempTransparent = true;
		TempSrcBlend = SrcBlend;
		TempDestBlend = DestBlend;
		TempAlphaBlendEnable = AlphaBlendEnable;
		TempZWriteEnable = ZWriteEnable;
		TempTextureMapBlend = TextureMapBlend;
		TempEffect = Effect;

		SrcBlend = D3DBLEND_SRCALPHA;
		DestBlend = D3DBLEND_INVSRCALPHA;
		AlphaBlendEnable = TRUE;
		ZWriteEnable = FALSE;
		TextureMapBlend = D3DTBLEND_MODULATEALPHA;
		Effect = RS_None;
	}
}
#endif

// ResetTempTransparent
//
// reset transparency

#ifndef TARGET_DC
void RenderState::ResetTempTransparent()
{
	if (TempTransparent)
	{
		TempTransparent = false;
		SrcBlend = TempSrcBlend;
		DestBlend = TempDestBlend;
		AlphaBlendEnable = TempAlphaBlendEnable;
		ZWriteEnable = TempZWriteEnable;
		TextureMapBlend = TempTextureMapBlend;
		Effect = TempEffect;
	}
}
#endif

// SetTexture
//
// set texture

void RenderState::SetTexture(LPDIRECT3DTEXTURE2 texture)
{
	TextureMap = texture;
}

// SetRenderState
//
// emulate SetRenderState() call

void RenderState::SetRenderState(DWORD ix, DWORD value)
{
	if ((ix == D3DRENDERSTATE_TEXTUREMAPBLEND) && (value == D3DTBLEND_MODULATEALPHA) && !the_display.GetDeviceInfo()->ModulateAlphaSupported())
	{
		value = D3DTBLEND_MODULATE;	// why doesn't DX do this?
	}

	if ((ix == D3DRENDERSTATE_TEXTUREMAPBLEND) && (value == D3DTBLEND_DECAL))
	{
		if (the_display.GetDeviceInfo()->DestInvSourceColourSupported() &&
			!the_display.GetDeviceInfo()->ModulateAlphaSupported())
		{
			// Rage Pro - don't do this
		}
		else
		{
			value = D3DTBLEND_MODULATE;
			Effect = RS_DecalMode;
		}
	}

	switch (ix)
	{
	// things we can set
	case D3DRENDERSTATE_TEXTUREADDRESS:		TextureAddress = value;		break;
	case D3DRENDERSTATE_TEXTUREMAG:			TextureMag = value;			break;
	case D3DRENDERSTATE_TEXTUREMIN:			TextureMin = value;			break;
	case D3DRENDERSTATE_TEXTUREMAPBLEND:	TextureMapBlend = value;	break;
#ifdef TARGET_DC
	// These states are ignored on DC.
	case D3DRENDERSTATE_ZENABLE:			break;
	case D3DRENDERSTATE_ZWRITEENABLE:		break;
	case D3DRENDERSTATE_ZFUNC:				break;
	case D3DRENDERSTATE_COLORKEYENABLE:		break;
#else
	case D3DRENDERSTATE_ZENABLE:			ZEnable = value;			break;
	case D3DRENDERSTATE_ZWRITEENABLE:		ZWriteEnable = value;		break;
	case D3DRENDERSTATE_ZFUNC:				ZFunc = value;				break;
	case D3DRENDERSTATE_COLORKEYENABLE:		ColorKeyEnable = value;		break;
#endif

	case D3DRENDERSTATE_FOGENABLE:			FogEnable = value;			break;
	case D3DRENDERSTATE_ALPHATESTENABLE:	AlphaTestEnable = value;	break;
	case D3DRENDERSTATE_SRCBLEND:			SrcBlend = value;			break;
	case D3DRENDERSTATE_DESTBLEND:			DestBlend = value;			break;
	case D3DRENDERSTATE_ALPHABLENDENABLE:	AlphaBlendEnable = value;	break;
	case D3DRENDERSTATE_ZBIAS:				ZBias = value;				break;
	case D3DRENDERSTATE_CULLMODE:			CullMode = value;			break;

	// defaults which can't change
	case D3DRENDERSTATE_TEXTUREPERSPECTIVE:	ASSERT(value == TRUE);				break;
	case D3DRENDERSTATE_ALPHAREF:			ASSERT(value == 0);					break;
	case D3DRENDERSTATE_ALPHAFUNC:			ASSERT(value == D3DCMP_NOTEQUAL);	break;
	case D3DRENDERSTATE_DITHERENABLE:		ASSERT(value == TRUE);				break;
	case D3DRENDERSTATE_SPECULARENABLE:		ASSERT(value == TRUE);				break;
	case D3DRENDERSTATE_SUBPIXEL:			ASSERT(value == TRUE);				break;
	case D3DRENDERSTATE_SHADEMODE:			ASSERT(value == D3DSHADE_GOURAUD);	break;

	default:								ASSERT(0);
	}
}

// SetEffect
//
// set a special vertex effect

void RenderState::SetEffect(DWORD effect)
{
	Effect = effect;

	switch (effect)
	{
	case RS_AlphaPremult:
	case RS_InvAlphaPremult:
		SrcBlend = D3DBLEND_ONE;
		DestBlend = D3DBLEND_ONE;
		TextureMapBlend = D3DTBLEND_MODULATE;
		break;

	case RS_BlackWithAlpha:
		SrcBlend = D3DBLEND_SRCALPHA;
		DestBlend = D3DBLEND_INVSRCALPHA;
		TextureMapBlend = D3DTBLEND_MODULATEALPHA;
		break;

	case RS_DecalMode:
		TextureMapBlend = D3DTBLEND_MODULATE;
		break;
	}
}

#ifdef _DEBUG
// Validate
//
// validate the render state for the PerMedia 2 ;)

char* RenderState::Validate()
{
	// check which alpha modes are used
	if (AlphaBlendEnable)
	{
		if ((SrcBlend == D3DBLEND_ONE) && (DestBlend == D3DBLEND_ZERO))	return NULL;
		if ((SrcBlend == D3DBLEND_ZERO) && (DestBlend == D3DBLEND_ONE))	return NULL;
		if ((SrcBlend == D3DBLEND_ONE) && (DestBlend == D3DBLEND_ONE))	return NULL;
		if ((SrcBlend == D3DBLEND_SRCALPHA) && (DestBlend == D3DBLEND_INVSRCALPHA))	return NULL;
		if ((SrcBlend == D3DBLEND_ONE) && (DestBlend == D3DBLEND_SRCALPHA))	return NULL;

		//TRACE("SrcBlend = %d DestBlend = %d\n", SrcBlend, DestBlend);
		//return "Renderstate has a bad alpha blend mode";
		return "";
	}

	return NULL;
}
#endif

#if 0
/*

NOTES:

Rs == Rv * Rt
As == Av * At

TODO:

Add textures in (change filenames loaded for the pages shown)

SRCALPHA:ONE

(As,As,As,As):(1,1,1,1)

SrcBlend = 5 DestBlend = 2		Page 1353 : Renderstate has a bad alpha blend mode	BIG_RAIN			raindrop2.tga	bigrain				-
SrcBlend = 5 DestBlend = 2		Page 1362 : Renderstate has a bad alpha blend mode	NEWFONT_INVERSE		olyfont2.tga	lcdfont_alpha		+
SrcBlend = 5 DestBlend = 2		Page 1375 : Renderstate has a bad alpha blend mode	SMOKER				smoker2.tga		smoker				-
SrcBlend = 5 DestBlend = 2		Page 1378 : Renderstate has a bad alpha blend mode	ADDITIVEALPHA		n/a
SrcBlend = 5 DestBlend = 2		Page 1383 : Renderstate has a bad alpha blend mode	HITSPANG			hitspang.tga	hitspang			-
SrcBlend = 5 DestBlend = 2		Page 1384 : Renderstate has a bad alpha blend mode	BLOOM2				bloom4.tga		bloom1				-
SrcBlend = 5 DestBlend = 2		Page 1385 : Renderstate has a bad alpha blend mode	BLOOM1				bloom2.tga		bloom2				-
SrcBlend = 5 DestBlend = 2		Page 1402 : Renderstate has a bad alpha blend mode	FLAMES2				explode4.tga	flame2				-
SrcBlend = 5 DestBlend = 2		Page 1406 : Renderstate has a bad alpha blend mode	FLAMES				flame1.tga		flames_alpha		+

SOLUTION:

Premultiply vertex colour by vertex alpha, set vertex alpha to 0
Premultiply texture colour by texture alpha, set texture alpha to 0
Use TBLEND_MODULATE and ONE:ONE

So blender red = (Rv*Av*Rt*At*1) + Rd == Rs*As + Rd

ZERO:INVSRCALPHA

(0,0,0,0):(1-As,1-As,1-As,1-As)

SrcBlend = 1 DestBlend = 6		Page 1349 : Renderstate has a bad alpha blend mode	SMOKECLOUD
SrcBlend = 1 DestBlend = 6		Page 1357 : Renderstate has a bad alpha blend mode	TYRESKID
SrcBlend = 1 DestBlend = 6		Page 1361 : Renderstate has a bad alpha blend mode	SUBTRACTIVEALPHA
SrcBlend = 1 DestBlend = 6		Page 1373 : Renderstate has a bad alpha blend mode	NEWFONT
SrcBlend = 1 DestBlend = 6		Page 1432 : Renderstate has a bad alpha blend mode	(POLY_NUM_PAGES - 2)

Set vertex colour to (0,0,0,Av)
Set texture colour to (Rt,Gt,Bt,At) (do nothing!)
Use TBLEND_MODULATEALPHA and SRCALPHA:INVSRCALPHA

So blender red = (0*0*Av*At) + (1-Av*At)*Rd == 0 + (1-As)*Rd

SRCALPHA:INVSRCCOLOR

(As,As,As,As):(1-Rs,1-Gs,1-Bs,1-As)

SrcBlend = 5 DestBlend = 4		Page 1379 : Renderstate has a bad alpha blend mode	TYRETRACK			tyremark.tga	tyretrack_alpha		+
SrcBlend = 5 DestBlend = 4		Page 1397 : Renderstate has a bad alpha blend mode	FOOTPRINT			footprint.tga	footprint			-

Use a white+alpha texture, TBLEND_MODULATEALPHA and SRCALPHA:INVSRCALPHA
Diffuse colour sets colour; diffuse alpha sets fade-out

INVSRCALPHA:ONE

(1-As,1-As,1-As,1-As):(1,1,1,1)

SrcBlend = 6 DestBlend = 2		Page 1387 : Renderstate has a bad alpha blend mode	FLAMES3				flame3.tga		flames3				-
SrcBlend = 6 DestBlend = 2		Page 1388 : Renderstate has a bad alpha blend mode	DUSTWAVE			dustwave.tga	dustwave			-
SrcBlend = 6 DestBlend = 2		Page 1389 : Renderstate has a bad alpha blend mode	BIGBANG				exp_gunk.tga	bigbang				-

Premultiply vertex colour by 1 - vertex alpha, set vertex alpha to 0
Premultiply texture colour by 1 - texture alpha, set texture colour to 0
Use TBLEND_MODULATE and ONE:ONE

So blender red = (Rv*(1-Av)*Rt*(1-At)*1) + Rd = Rs*(1-Av-At+Av*At) + Rd
We want (Rv*Rt*(1-Av*At)) + Rd 

ZERO:SRCCOLOR

(0,0,0,0):(Rs,Gs,Bs,As)

SrcBlend = 1 DestBlend = 3		Page 1398 : Renderstate has a bad alpha blend mode	BARBWIRE			barbed.tga		barbwire			-

Use an alpha texture and invert (so white is wire, black is space) and the RGB of the texture is all black, then
use TBLEND_MODULATEALPHA and SRCALPHA:INVSRCALPHA

ZERO:INVSRCCOLOR

(0,0,0,0):(1-Rs,1-Gs,1-Bs,1-As)

SrcBlend = 1 DestBlend = 4		Page 1403 : Renderstate has a bad alpha blend mode	SMOKE

Never used.  Fuck it off.

*/
#endif

// InitScene
//
// set all state variables and remember

void RenderState::InitScene(DWORD fog_colour)
{
	// set constant state variables

	extern int AENG_detail_perspective;

	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREPERSPECTIVE, AENG_detail_perspective ? TRUE : FALSE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_CULLMODE, CullMode);

	// we use > 0x07 instead of != 0x00 because it's
	// (a) more robust
	// (b) works on the Permedia 2 drivers
	// (c) looks better when alpha is interpolated
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, 0x07);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
	
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DITHERENABLE, TRUE);

	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE, TRUE); 
//	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SPECULARENABLE, FALSE); 



	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SUBPIXEL, TRUE);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGCOLOR, fog_colour);
#ifdef TARGET_DC
	// These really should have been set up for the PC as well!
#ifdef FAKE_UP_VERTEX_FOG_PLEASE_BOB
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_NONE);
#else
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR);
#endif


	// Tweak these to your liking.

#if 0
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLESTART, FloatAsDword(30.0f));
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEEND, FloatAsDword(60.0f));
#else
extern SLONG	CurDrawDistance;
	float fFogDist = CurDrawDistance * ( 60.0f / ( 22.f * 256.0f ) );
	float fFogDistNear = fFogDist * 0.7f;
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLESTART, FloatAsDword(fFogDistNear));
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEEND, FloatAsDword(fFogDist));
#endif
	// Density isn't actually used.
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGTABLEDENSITY, FloatAsDword(0.5f));
#endif

	// set variable state variables

#ifdef TARGET_DC
	// Don't use these blends - use the TSS ones instead.
	switch ( TextureMapBlend )
	{
	case D3DTBLEND_MODULATE:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		// Erk - how do you do this then?
		//if ( the_texture_has_an_alpha_channel )
		//{
		//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		//}
		//else
		//{
		//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
		//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		//}
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	case D3DTBLEND_MODULATEALPHA:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		break;
	case D3DTBLEND_ADD:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_ADD);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		break;
	case D3DTBLEND_COPY:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		break;
	default:
		// If this triggers, I'll need to add the blend mode.
		ASSERT ( FALSE );
		break;
	}

	// Also sets up some default multitexture stuff, coz there now is some in the game.
	switch ( TextureMin )
	{
	case D3DFILTER_NEAREST:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_MINFILTER,     D3DTFN_POINT);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_MIPFILTER,     D3DTFP_POINT);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_MINFILTER,     D3DTFN_POINT);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_MIPFILTER,     D3DTFP_POINT);
		break;
	case D3DFILTER_LINEAR:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_MINFILTER,     D3DTFN_LINEAR);
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_MIPFILTER,     MIPMAP_MODE_PLEASE_BOB);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_MINFILTER,     D3DTFN_LINEAR);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_MIPFILTER,     MIPMAP_MODE_PLEASE_BOB);
		break;
	default:
		// Need to add the others.
		ASSERT ( FALSE );
		break;
	}
	switch ( TextureMag )
	{
	case D3DFILTER_NEAREST:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_MAGFILTER,     D3DTFG_POINT);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_MAGFILTER,     D3DTFG_POINT);
		break;
	case D3DFILTER_LINEAR:
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_MAGFILTER,     D3DTFG_LINEAR);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_MAGFILTER,     D3DTFG_LINEAR);
		break;
	default:
		// Need to add the others.
		ASSERT ( FALSE );
		break;
	}

    REALLY_SET_TEXTURE_STATE(0, D3DTSS_TEXCOORDINDEX, 0);
	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ADDRESS, TextureAddress);
    REALLY_SET_TEXTURE_STATE(1, D3DTSS_TEXCOORDINDEX, 0);
	REALLY_SET_TEXTURE_STATE(1, D3DTSS_ADDRESS, TextureAddress);

	REALLY_SET_TEXTURE_STATE(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	REALLY_SET_TEXTURE_STATE(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	REALLY_SET_TEXTURE(TextureMap);



	// Set the mipmap LOD bias.
	static float fTemp = 0.0f;
 	DWORD dwTemp = *((DWORD*) (&fTemp));
	REALLY_SET_TEXTURE_STATE ( 0, D3DTSS_MIPMAPLODBIAS, dwTemp );
	REALLY_SET_TEXTURE_STATE ( 1, D3DTSS_MIPMAPLODBIAS, dwTemp );



#else
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND, TextureMapBlend);
	REALLY_SET_TEXTURE(TextureMap);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREADDRESS, TextureAddress);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAG, TextureMag);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMIN, TextureMin);
#endif

#ifndef TARGET_DC
	// Ignored on DC.
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZENABLE, ZEnable);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZWRITEENABLE, ZWriteEnable);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZFUNC, ZFunc);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_COLORKEYENABLE, ColorKeyEnable);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FogEnable);
#else //#ifndef TARGET_DC

	// Hack the fog off - it doesn't seem to work ATM.
	// Does now.
#ifndef FAKE_UP_VERTEX_FOG_PLEASE_BOB
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_FOGENABLE, FogEnable);
#endif

#endif //#else //#ifndef TARGET_DC

	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, AlphaTestEnable);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_SRCBLEND, SrcBlend);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_DESTBLEND, DestBlend);
	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ALPHABLENDENABLE, AlphaBlendEnable);

//	REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ZBIAS, ZBias);	// doesn't work - done in PolyPage::AddFan instead

	// remember the state

	s_State = *this;
}

#define	MAYBE_SET(CAPS, SMALL)		if (s_State.SMALL != SMALL)	{ REALLY_SET_RENDER_STATE(D3DRENDERSTATE_ ## CAPS, SMALL); s_State.SMALL = SMALL; }

// SetChanged
//
// set just the different render states




#ifndef TARGET_DC


void RenderState::SetChanged()
{
	DWORD old_texture_address = TextureAddress;

	if (WrapOnce)
	{
		TextureAddress = D3DTADDRESS_WRAP;
	}

	if (s_State.TextureMap != TextureMap)
	{
		REALLY_SET_TEXTURE(TextureMap);
		s_State.TextureMap = TextureMap;
	}

#ifdef EDITOR
	DWORD	old_TextureMapBlend = TextureMapBlend;
	DWORD	old_AlphaBlendEnable = AlphaBlendEnable;
	DWORD	old_SrcBlend = SrcBlend;
	DWORD	old_DestBlend = DestBlend;

	if (AllowFadeOut)
	{
		TextureMapBlend = D3DTBLEND_MODULATEALPHA;
		AlphaBlendEnable = TRUE;
		SrcBlend = D3DBLEND_SRCALPHA;
		DestBlend = D3DBLEND_INVSRCALPHA;
	}
#endif




#ifdef TARGET_DC
	if ( TextureMapBlend != s_State.TextureMapBlend )
	{
		s_State.TextureMapBlend = TextureMapBlend;
		switch ( TextureMapBlend )
		{
		case D3DTBLEND_MODULATE:
#if 1
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			// Erk - how do you do this then?
			//if ( the_texture_has_an_alpha_channel )
			//{
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			//}
			//else
			//{
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			//}
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#else
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#endif


		case D3DTBLEND_MODULATEALPHA:
#if 1
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#else
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#endif
			break;
		case D3DTBLEND_ADD:
			// Does anyone use this?
			ASSERT(FALSE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_ADD);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			break;
		case D3DTBLEND_COPY:
			// Does anyone use this?
			ASSERT(FALSE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			break;
		default:
			// If this triggers, I'll need to add the blend mode.
			ASSERT ( FALSE );
			break;
		}
	}

	if ( TextureMin != s_State.TextureMin )
	{
		s_State.TextureMin = TextureMin;
		switch ( TextureMin )
		{
		case D3DFILTER_NEAREST:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MINFILTER,     D3DTFN_POINT);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MIPFILTER,     D3DTFP_POINT);
			break;
		case D3DFILTER_LINEAR:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MINFILTER,     D3DTFN_LINEAR);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MIPFILTER,     MIPMAP_MODE_PLEASE_BOB);
			break;
		default:
			// Need to add the others.
			ASSERT ( FALSE );
			break;
		}
	}

	if ( TextureMag != s_State.TextureMag )
	{
		s_State.TextureMag = TextureMag;
		switch ( TextureMag )
		{
		case D3DFILTER_NEAREST:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MAGFILTER,     D3DTFG_POINT);
			break;
		case D3DFILTER_LINEAR:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MAGFILTER,     D3DTFG_LINEAR);
			break;
		default:
			// Need to add the others.
			ASSERT ( FALSE );
			break;
		}
	}

	if ( TextureAddress != s_State.TextureAddress )
	{
		s_State.TextureAddress = TextureAddress;
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ADDRESS, TextureAddress);
	}

    REALLY_SET_TEXTURE_STATE(0, D3DTSS_TEXCOORDINDEX, 0);
	REALLY_SET_TEXTURE_STATE(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	REALLY_SET_TEXTURE_STATE(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


#else

	MAYBE_SET(TEXTUREADDRESS,	TextureAddress);
	MAYBE_SET(TEXTUREMAG,		TextureMag);
	MAYBE_SET(TEXTUREMIN,		TextureMin);
#ifndef TARGET_DC
	if (!UseDX5Fix)
	{
		MAYBE_SET(TEXTUREMAPBLEND,	TextureMapBlend);
	}
	else
#endif
	{
		if (s_State.TextureMapBlend != TextureMapBlend)
		{
			if (TextureMapBlend == D3DTBLEND_MODULATEALPHA)
			{
				// This makes MODULATEALPHA work on the Permedia 2
				// DO NOT CHANGE THE ORDER OF THESE LINES
				// They work around a bug in DirectX 6
				REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE); 
				REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
				REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

				REALLY_SET_TEXTURE_STATE(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
				REALLY_SET_TEXTURE_STATE(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			}
			else
			{
				REALLY_SET_RENDER_STATE(D3DRENDERSTATE_TEXTUREMAPBLEND, TextureMapBlend);
			}
			s_State.TextureMapBlend = TextureMapBlend;
		}
	}
#endif

	MAYBE_SET(ZENABLE,			ZEnable);
	MAYBE_SET(ZWRITEENABLE,		ZWriteEnable);
	MAYBE_SET(ALPHATESTENABLE,	AlphaTestEnable);
	MAYBE_SET(SRCBLEND,			SrcBlend);
	MAYBE_SET(DESTBLEND,		DestBlend);
	MAYBE_SET(ALPHABLENDENABLE,	AlphaBlendEnable);
	MAYBE_SET(ZFUNC,			ZFunc);
	MAYBE_SET(CULLMODE,			CullMode);


	// Hack the fog off for the DC - it doesn't seem to work ATM.
	// Does now.
	MAYBE_SET(FOGENABLE,		FogEnable);

#ifndef TARGET_DC
	MAYBE_SET(COLORKEYENABLE,	ColorKeyEnable);
#endif
//	MAYBE_SET(ZBIAS,			ZBias);	// doesn't work - done in PolyPage::AddFan instead

	if (WrapOnce)
	{
		TextureAddress = old_texture_address;
		WrapOnce       = false;
	}

#ifdef EDITOR
	TextureMapBlend = old_TextureMapBlend;
	AlphaBlendEnable = old_AlphaBlendEnable;
	SrcBlend = old_SrcBlend;
	DestBlend = old_DestBlend;
#endif
}




#else


float fMipMapLodBias = 0.0f;


void RenderState::SetChanged()
{
	DWORD old_texture_address = TextureAddress;

	if (WrapOnce)
	{
		TextureAddress = D3DTADDRESS_WRAP;
	}

	if (s_State.TextureMap != TextureMap)
	{
		// Do I need this line?
		// I have a vague feeling that the DC doesn't like you sending NULL textures down. Not sure.
		// FIXME! Why does this fall over on the first frame?
		//char cTemp[30];
		//sprintf ( cTemp, "TM:0x%x ", TextureMap );
		//OutputDebugString ( cTemp );
		//MAKE_MY_TEXTURES_WORK_PLEASE_BOB_YOU_SCUMBAG

#if 0
extern int iPolyRenderStateFrameNum;
		if ( iPolyRenderStateFrameNum < 100 )
		{
			char cTemp[30];
			sprintf ( cTemp, "TM:0x%x\n", TextureMap );
			OutputDebugString ( cTemp );
			REALLY_SET_TEXTURE(NULL);
		}
		else
#endif
		{
			REALLY_SET_TEXTURE(TextureMap);
		}
		s_State.TextureMap = TextureMap;
	}
	else
	{
		//char cTemp[30];
		//sprintf ( cTemp, "TMx:0x%x\n", TextureMap );
		//OutputDebugString ( cTemp );
	}


	if ( AlphaTestEnable )
	{
		// Override - use alpha blend instead.
		if ( AlphaBlendEnable && ( SrcBlend != D3DBLEND_SRCALPHA ) && ( DestBlend != D3DBLEND_INVSRCALPHA ) )
		{
			// Oh - that's a shame.
			ASSERT ( FALSE );
		}
		AlphaBlendEnable = TRUE;
		SrcBlend = D3DBLEND_SRCALPHA;
		DestBlend = D3DBLEND_INVSRCALPHA;
		//AlphaBlendEnable = TRUE;
		//SrcBlend = D3DBLEND_ONE;
		//DestBlend = D3DBLEND_ONE;
		AlphaTestEnable = FALSE;
	}


#ifndef TARGET_DC
	// Ignored states on DC.
	MAYBE_SET(ZENABLE,			ZEnable);
	MAYBE_SET(ZFUNC,			ZFunc);
	MAYBE_SET(ZWRITEENABLE,		ZWriteEnable);
	REALLY_SET_RENDER_STATE ( D3DRENDERSTATE_COLORKEYENABLE, FALSE );
	MAYBE_SET(COLORKEYENABLE,	ColorKeyEnable);
#else
	// Hack the fog off for the DC - it doesn't seem to work ATM.
	// Does now!
	//FogEnable = FALSE;
	MAYBE_SET(FOGENABLE,		FogEnable);
#endif

	MAYBE_SET(ALPHATESTENABLE,	AlphaTestEnable);
	MAYBE_SET(SRCBLEND,			SrcBlend);
	MAYBE_SET(DESTBLEND,		DestBlend);
	MAYBE_SET(ALPHABLENDENABLE,	AlphaBlendEnable);

	// Forgot this one last time!
	MAYBE_SET(CULLMODE,			CullMode);


	if ( TextureMapBlend != s_State.TextureMapBlend )
	{
		s_State.TextureMapBlend = TextureMapBlend;
		switch ( TextureMapBlend )
		{
		case D3DTBLEND_MODULATE:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			// Erk - how do you do this then?
			//if ( the_texture_has_an_alpha_channel )
			//{
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			//}
			//else
			//{
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
			//	REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			//}
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			//REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);


		case D3DTBLEND_MODULATEALPHA:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			break;
		default:
			// If this triggers, I'll need to add the blend mode.
			ASSERT ( FALSE );
			break;
		}

		REALLY_SET_TEXTURE_STATE(0, D3DTSS_TEXCOORDINDEX, 0);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		REALLY_SET_TEXTURE_STATE(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	}

	if ( TextureMin != s_State.TextureMin )
	{
		s_State.TextureMin = TextureMin;
		switch ( TextureMin )
		{
		case D3DFILTER_NEAREST:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MINFILTER,     D3DTFN_POINT);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MIPFILTER,     D3DTFP_POINT);
			break;
		case D3DFILTER_LINEAR:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MINFILTER,     D3DTFN_LINEAR);
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MIPFILTER,     MIPMAP_MODE_PLEASE_BOB);
			break;
		default:
			// Need to add the others.
			ASSERT ( FALSE );
			break;
		}
	}

	if ( TextureMag != s_State.TextureMag )
	{
		s_State.TextureMag = TextureMag;
		switch ( TextureMag )
		{
		case D3DFILTER_NEAREST:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MAGFILTER,     D3DTFG_POINT);
			break;
		case D3DFILTER_LINEAR:
			REALLY_SET_TEXTURE_STATE(0, D3DTSS_MAGFILTER,     D3DTFG_LINEAR);
			break;
		default:
			// Need to add the others.
			ASSERT ( FALSE );
			break;
		}
	}

	if ( TextureAddress != s_State.TextureAddress )
	{
		s_State.TextureAddress = TextureAddress;
		REALLY_SET_TEXTURE_STATE(0, D3DTSS_ADDRESS, TextureAddress);
	}




	if (WrapOnce)
	{
		TextureAddress = old_texture_address;
		WrapOnce       = false;
	}


}







#endif

// Returns TRUE if the renderstates are equivalent,
// ie you don't need any renderstate changes to go from one to the other.
bool RenderState::IsSameRenderState ( RenderState *pRS )
{
	bool bRes = TRUE; 

#define CHECK_RS(name) if ( name != pRS->name ) { bRes = FALSE; }


	CHECK_RS ( TextureMap );
	CHECK_RS ( TextureAddress );
	CHECK_RS ( TextureMag );
	CHECK_RS ( TextureMin );
	CHECK_RS ( TextureMapBlend );

#ifndef TARGET_DC
	CHECK_RS ( ZEnable );
	CHECK_RS ( ZWriteEnable );
#endif
	CHECK_RS ( AlphaTestEnable );
	CHECK_RS ( SrcBlend );
	CHECK_RS ( DestBlend );
#ifndef TARGET_DC
	CHECK_RS ( ZFunc );
#endif
	CHECK_RS ( AlphaBlendEnable );
	CHECK_RS ( FogEnable );
#ifndef TARGET_DC
	CHECK_RS ( ColorKeyEnable );
#endif

	CHECK_RS ( CullMode );
	CHECK_RS ( ZBias );
	CHECK_RS ( Effect );
#if 0
	CHECK_RS ( WrapOnce );
#endif

#if 0
	CHECK_RS ( TempTransparent );
	CHECK_RS ( TempSrcBlend );
	CHECK_RS ( TempDestBlend );
	CHECK_RS ( TempAlphaBlendEnable );
	CHECK_RS ( TempZWriteEnable );
	CHECK_RS ( TempTextureMapBlend );
	CHECK_RS ( TempEffect );
#endif


#undef CHECK_RS

	return ( bRes );
}
