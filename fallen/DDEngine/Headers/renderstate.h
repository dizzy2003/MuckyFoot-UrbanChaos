// renderstate.h
//
// render state class; encapsulates a state of the renderer

#ifndef _RENDERSTATE_
#define _RENDERSTATE_

enum SpecialEffect
{
	RS_None,			// no special effect
	RS_AlphaPremult,	// premultiply vertex colours by alpha and set alpha to 0
	RS_BlackWithAlpha,	// set vertex R,G,B to (0,0,0)
	RS_InvAlphaPremult,	// premultiply vertex colours by 1-alpha and set alpha to 0
	RS_DecalMode		// set vertex A,R,G,B to (255,255,255,255)
};

#pragma pack( push, 4 )
struct RenderState
{
	RenderState(DWORD mag_filter = D3DFILTER_LINEAR, DWORD min_filter = D3DFILTER_NEAREST);

	void		SetTexture(LPDIRECT3DTEXTURE2 texture);
	void		SetRenderState(DWORD index, DWORD value);
	void		SetEffect(DWORD effect);

#ifndef TARGET_DC
	void		SetTempTransparent();
	void		ResetTempTransparent();
#endif

	LPDIRECT3DTEXTURE2	GetTexture()	{ return TextureMap; }
	DWORD				GetEffect()		{ return Effect; }

	void		InitScene(DWORD fog_colour);

	void		SetChanged();		// set changed members

#ifdef TARGET_DC
	bool		NeedsSorting()	{ return FALSE; }
#else
	bool		NeedsSorting()	{ return !ZWriteEnable; }
#endif
	DWORD		ZLift()			{ return ZBias; }

	void		WrapJustOnce()	{WrapOnce = true;}	// Temporarily sets the state to wrapping just for one call to SetChanged()

#ifdef _DEBUG
	char*		Validate();	// returns a string error if not OK
#endif

	bool		IsSameRenderState ( RenderState *pRS );


	static RenderState		s_State;

#ifdef EDITOR
	static bool				AllowFadeOut;
#endif

	bool	IsAlphaBlendEnabled() { return ( AlphaBlendEnable != FALSE );}

private:
	LPDIRECT3DTEXTURE2		TextureMap;
	DWORD					TextureAddress;
	DWORD					TextureMag;
	DWORD					TextureMin;
	DWORD					TextureMapBlend;

#ifndef TARGET_DC
	DWORD					ZEnable;
	DWORD					ZWriteEnable;
#endif
	DWORD					AlphaTestEnable;
	DWORD					SrcBlend;
	DWORD					DestBlend;
#ifndef TARGET_DC
	DWORD					ZFunc;
#endif
	DWORD					AlphaBlendEnable;
	DWORD					FogEnable;
#ifndef TARGET_DC
	DWORD					ColorKeyEnable;
#endif
	DWORD					CullMode;
	DWORD					ZBias;
	DWORD					Effect;
	bool					WrapOnce;

#ifndef TARGET_DC
	bool					TempTransparent;
	DWORD					TempSrcBlend;
	DWORD					TempDestBlend;
	DWORD					TempAlphaBlendEnable;
	DWORD					TempZWriteEnable;
	DWORD					TempTextureMapBlend;
	DWORD					TempEffect;
#endif
};
#pragma pack( pop )

void	RS_read_config();
void	RS_get_config(int* fix_directx);
void	RS_set_config(int fix_directx);

#endif
