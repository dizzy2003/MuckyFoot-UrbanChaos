#include	"demo.h"
#include	"DDLib.h"
#include "frontend.h"
#include "xlat_str.h"
#include "menufont.h"
#include "font2d.h"
#include "env.h"
#include "drive.h"
#include "snd_type.h"
#include "sound.h"
#include "MFX.h"
#include "MFX_Miles.h"
#include "music.h"
#include "poly.h"
#include "drawxtra.h"
#include "fmatrix.h"
#include    "C:\fallen\DDLibrary\headers\D3DTexture.h"
#include    "C:\fallen\DDLibrary\headers\GDisplay.h"
#include	"C:\fallen\DDEngine\headers\polypage.h"
#include "io.h"
#include "truetype.h"
#include "c:\fallen\ddlibrary\headers\dclowlevel.h"
#include "interfac.h"


#ifdef TARGET_DC
#include <platutil.h>
#include <shsgintr.h>
#endif


#ifdef TARGET_DC






// Cool particle system.

struct WibbleComponent
{
	//float fValue;
	float fOffset;

	float fCos1Amp;
	float fCos1Off;
	float fCos1Pos;
	float fCos1Inc;

	float fCos2Amp;
	float fCos2Off;
	float fCos2Pos;
	float fCos2Inc;

	float fRampTime;
	float fRampTimeInc;
	float fRampUpStart;
	float fRampUpEnd;
	float fRampMidValue;
	float fRampDownStart;
	float fRampDownEnd;
	float fRampBaseValue;

private:
	float fRampUpMul;
	float fRampDownMul;
	float fRampMidExtra;

public:

	void ChangedSomething ( void )
	{
		// Recalculate the cached stuff.
		ASSERT ( fRampUpStart <= fRampUpEnd );
		ASSERT ( fRampUpEnd <= fRampDownStart );
		ASSERT ( fRampDownStart <= fRampDownEnd );

		if ( fRampUpStart == fRampUpEnd )
		{
			fRampUpMul = 1.0f;
		}
		else
		{
			fRampUpMul = 1.0f / ( fRampUpEnd - fRampUpStart );
		}

		if ( fRampDownStart == fRampDownEnd )
		{
			fRampDownMul = 1.0f;
		}
		else
		{
			fRampDownMul = 1.0f / ( fRampDownEnd - fRampDownStart );
		}

		fRampMidExtra = fRampMidValue - fRampBaseValue;
	}

	// Set to known standard state.
	void Reset ( void )
	{
		fOffset = 1.0f;

		fCos1Inc = 0.0f;
		fCos1Pos = 0.0f;
		fCos1Amp = 1.0f;
		fCos1Off = 0.0f;
		
		fCos2Inc = 0.0f;
		fCos2Pos = 0.0f;
		fCos2Amp = 1.0f;
		fCos2Off = 0.0f;

		fRampTime		= 0.5f;
		fRampTimeInc	= 0.0f;
		fRampUpStart	= 0.0f;
		fRampUpEnd		= 0.0f;
		fRampMidValue	= 1.0f;
		fRampDownStart	= 1.0f;
		fRampDownEnd	= 1.0f;
		fRampBaseValue	= 0.0f;

		ChangedSomething();
	}


	WibbleComponent ( float fStartValue = 1.0f, float fStartOffset = 0.0f )
	{
		//fValue = fStartValue;
		Reset();
		fOffset = fStartOffset;
		fRampMidValue	= fStartValue;
		ChangedSomething();
	}

	float Eval ( void )
	{
		// Kick off a cosine.
		float fCos1, fCos2, fDiscardedSin;
		_SinCosA ( &fDiscardedSin, &fCos1, fCos1Pos );

		// Do the ramp.
		ASSERT ( fRampUpStart <= fRampUpEnd );
		ASSERT ( fRampUpEnd <= fRampDownStart );
		ASSERT ( fRampDownStart <= fRampDownEnd );

		float fVal;
		if ( fRampTime < fRampDownStart )
		{
			if ( fRampTime < fRampUpStart )
			{
				// Before ramp up.
				ASSERT ( fRampTime <= fRampUpStart );
				fVal = 0.0f;
			}
			else if ( fRampTime >= fRampUpEnd )
			{
				// In the mid section.
				ASSERT ( fRampTime >= fRampUpEnd );
				ASSERT ( fRampTime <= fRampDownStart );
				fVal = fRampMidExtra;
			}
			else
			{
				// In the ramp up.
				ASSERT ( fRampTime <= fRampUpEnd );
				ASSERT ( fRampTime >= fRampUpStart );
				fVal = fRampMidExtra * ( fRampTime - fRampUpStart ) * fRampUpMul;
			}
		}
		else
		{
			if ( fRampTime >= fRampDownEnd )
			{
				// After ramp down.
				ASSERT ( fRampTime >= fRampDownEnd );
				fVal = 0.0f;
			}
			else
			{
				// In the ramp down.
				ASSERT ( fRampTime <= fRampDownEnd );
				ASSERT ( fRampTime >= fRampDownStart );
				fVal = fRampMidExtra * ( fRampDownEnd - fRampTime ) * fRampDownMul;
			}
		}

		// Kick off the other cosine.
		_SinCosA ( &fDiscardedSin, &fCos2, fCos2Pos );

		fVal += fRampBaseValue;

		fCos1 *= fCos1Amp;
		fCos1 += fCos1Off;
		fVal *= fCos1;

		fCos2 *= fCos2Amp;
		fCos2 += fCos2Off;
		fVal *= fCos2;

		fVal += fOffset;

		return fVal;
	}

	void Run ( float fTimeStep )
	{
		fCos1Pos += fCos1Inc * fTimeStep;
		fCos2Pos += fCos2Inc * fTimeStep;
		fRampTime += fRampTimeInc * fTimeStep;
	}

	float RunEval ( float fTimeStep )
	{
		Run ( fTimeStep );
		return Eval();
	}

	float EvalAt ( float fTimeStep )
	{
		fCos1Pos = 0.0f;
		fCos2Pos = 0.0f;
		fRampTime = 0.0f;
		return RunEval ( fTimeStep );
	}

	// Some standard settings.
	void SetConstant ( float fVal )
	{
		fOffset = fVal;

		fCos1Inc = 0.0f;
		fCos1Pos = 0.0f;
		fCos1Amp = 1.0f;
		fCos1Off = 0.0f;
		
		fCos2Inc = 0.0f;
		fCos2Pos = 0.0f;
		fCos2Amp = 1.0f;
		fCos2Off = 0.0f;

		fRampTime		= 0.5f;
		fRampTimeInc	= 0.0f;
		fRampUpStart	= 0.0f;
		fRampUpEnd		= 0.0f;
		fRampMidValue	= 0.0f;
		fRampDownStart	= 1.0f;
		fRampDownEnd	= 1.0f;
		fRampBaseValue	= 0.0f;

		ChangedSomething();
	}

	void SetRamp ( float fDownVal, float fUpVal, float fStartUp, float fEndUp, float fStartDown, float fEndDown )
	{
		fOffset = 0.0f;

		fCos1Inc = 0.0f;
		fCos1Pos = 0.0f;
		fCos1Amp = 1.0f;
		fCos1Off = 0.0f;
		
		fCos2Inc = 0.0f;
		fCos2Pos = 0.0f;
		fCos2Amp = 1.0f;
		fCos2Off = 0.0f;

		fRampTime		= 0.0f;
		fRampTimeInc	= 1.0f;
		fRampUpStart	= fStartUp;
		fRampUpEnd		= fEndUp;
		fRampMidValue	= fUpVal;
		fRampDownStart	= fStartDown;
		fRampDownEnd	= fEndDown;
		fRampBaseValue	= fDownVal;

		ChangedSomething();
	}

	void SetIncreasing ( float fVal, float fInc )
	{
		fOffset = fVal;

		fCos1Inc = 0.0f;
		fCos1Pos = 0.0f;
		fCos1Amp = 1.0f;
		fCos1Off = 0.0f;
		
		fCos2Inc = 0.0f;
		fCos2Pos = 0.0f;
		fCos2Amp = 1.0f;
		fCos2Off = 0.0f;

		// But not too big or you get numerical precision problems.
		const float fBigNumber = 10000.0f;

		fRampTime		= 0.0f;
		fRampTimeInc	= 1.0f;
		fRampUpStart	= -fBigNumber;
		fRampUpEnd		= fBigNumber;
		fRampMidValue	= fBigNumber * fInc;
		fRampDownStart	= fBigNumber * 2.0f;
		fRampDownEnd	= fBigNumber * 2.0f;
		fRampBaseValue	= -fBigNumber * fInc;

		ChangedSomething();
	}


};


// The table goes red, orange, yellow, green, gryan, cyan, blue, purple.
#define NUM_SHADES_IN_TABLE 8
float fShadeTableR[NUM_SHADES_IN_TABLE] = { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
float fShadeTableG[NUM_SHADES_IN_TABLE] = { 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f };
float fShadeTableB[NUM_SHADES_IN_TABLE] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f };


struct TomPart
{
	WibbleComponent wcX;
	WibbleComponent wcY;
	WibbleComponent wcZ;
	WibbleComponent wcXSize;
	WibbleComponent wcYSize;
	WibbleComponent wcRotation;
	WibbleComponent wcShade;
	WibbleComponent wcBrightness;
	float fLifetime;

	float fU1;
	float fU2;
	float fV1;
	float fV2;

#if 0
	float fSpawnPeriod;
	float fSpawnCountdown;
	WibbleComponent wcXSpawn;
	WibbleComponent wcYSpawn;
	WibbleComponent wcZSpawn;
	WibbleComponent wcXSizeSpawn;
	WibbleComponent wcYSizeSpawn;
	WibbleComponent wcRotationSpawn;
	WibbleComponent wcShadeSpawn;
	WibbleComponent wcBrightnessSpawn;
#endif

	TomPart ( void )
	{
	}

	bool IsAlive ( void )
	{
		return ( fLifetime >= 0.0f );
	}

	void Kill ( void )
	{
		fLifetime = -1.0f;
	}

	// Returns TRUE if still alive.
	bool Run ( float fTimeStep )
	{
		if ( fLifetime < 0.0f )
		{
			// Dead.
			return FALSE;
		}
		fLifetime -= fTimeStep;

		wcX.Run ( fTimeStep );
		wcY.Run ( fTimeStep );
		wcZ.Run ( fTimeStep );
		wcXSize.Run ( fTimeStep );
		wcYSize.Run ( fTimeStep );
		wcRotation.Run ( fTimeStep );
		wcShade.Run ( fTimeStep );
		wcBrightness.Run ( fTimeStep );

		return TRUE;
	}

	// Returns TRUE if it used the vertices.
	bool Draw ( D3DTLVERTEX *d3dtlvFirst, float fGlobalFade )
	{
		if ( fLifetime < 0.0f )
		{
			// Particle is dead.
			return FALSE;
		}

		float fX		= wcX.Eval();
		float fY		= wcY.Eval();
		float fZ		= wcZ.Eval();
		float fXSize	= wcXSize.Eval();
		float fYSize	= wcYSize.Eval();
		float fRot		= wcRotation.Eval();
		float fShade	= wcShade.Eval();
		float fBright	= wcBrightness.Eval();

		// Kick off the sincos.
		float fCosR, fSinR;
		// Offset of 45 degrees
		fRot += ( 3.141592f / 4.0f );
		_SinCosA ( &fSinR, &fCosR, fRot );

		// Find the RGB shade.
		fShade *= NUM_SHADES_IN_TABLE;
		int iShade1 = (int)( fShade );
		int iShade2 = iShade1 + 1;
		float fShadeTween2 = fShade - (float)( iShade1 );
		iShade1 &= ( NUM_SHADES_IN_TABLE - 1 );
		iShade2 &= ( NUM_SHADES_IN_TABLE - 1 );
		float fShadeTween1 = 1.0f - fShadeTween2;

		float fColR = fShadeTableR[iShade1] * fShadeTween1 + fShadeTableR[iShade2] * fShadeTween2;
		float fColG = fShadeTableG[iShade1] * fShadeTween1 + fShadeTableG[iShade2] * fShadeTween2;
		float fColB = fShadeTableB[iShade1] * fShadeTween1 + fShadeTableB[iShade2] * fShadeTween2;
		float fColA = 1.0f;

		// Modulate by brightness.
		fColR *= fBright;
		fColG *= fBright;
		fColB *= fBright;
		fColA *= fBright;

		if ( fBright > 1.0f )
		{
			// Saturate to white.
			fColR += fBright - 1.0f;
			fColG += fBright - 1.0f;
			fColB += fBright - 1.0f;
			fColA += fBright - 1.0f;
		}

		fColA *= fGlobalFade;

		fColR *= 255.0f;
		fColG *= 255.0f;
		fColB *= 255.0f;
		fColA *= 255.0f;

		// Compile to DWORD.
		int iColR = (int)fColR;
		int iColG = (int)fColG;
		int iColB = (int)fColB;
		int iColA = (int)fColA;
		if ( iColR < 0 )
		{
			iColR = 0;
		}
		else if ( iColR > 255 )
		{
			iColR = 255;
		}
		if ( iColG < 0 )
		{
			iColG = 0;
		}
		else if ( iColG > 255 )
		{
			iColG = 255;
		}
		if ( iColB < 0 )
		{
			iColB = 0;
		}
		else if ( iColB > 255 )
		{
			iColB = 255;
		}
		if ( iColA < 0 )
		{
			iColA = 0;
		}
		else if ( iColA > 255 )
		{
			iColA = 255;
		}

		DWORD dwColour = ( iColA << 24 ) | ( iColR << 16 ) | ( iColG << 8 ) | ( iColB << 0 );

		// Find the positions.
		fXSize *= fZ * 1.414f;
		fYSize *= fZ * 1.414f;
		float fVec1X = fCosR * fXSize;
		float fVec1Y = fSinR * fYSize;
		float fVec2X = -fSinR * fXSize;
		float fVec2Y = fCosR * fYSize;

		// Top left.
		d3dtlvFirst[0].sx = fX + fVec2X;
		d3dtlvFirst[0].sy = fY - fVec2Y;
		d3dtlvFirst[0].sz = fZ;
		d3dtlvFirst[0].rhw = fZ;
		d3dtlvFirst[0].color = dwColour;
		d3dtlvFirst[0].specular = 0;
		d3dtlvFirst[0].tu = fU1;
		d3dtlvFirst[0].tv = fV1;

		// Top right.
		d3dtlvFirst[1].sx = fX + fVec1X;
		d3dtlvFirst[1].sy = fY - fVec1Y;
		d3dtlvFirst[1].sz = fZ;
		d3dtlvFirst[1].rhw = fZ;
		d3dtlvFirst[1].color = dwColour;
		d3dtlvFirst[1].specular = 0;
		d3dtlvFirst[1].tu = fU2;
		d3dtlvFirst[1].tv = fV1;

		// Bottom right.
		d3dtlvFirst[2].sx = fX - fVec2X;
		d3dtlvFirst[2].sy = fY + fVec2Y;
		d3dtlvFirst[2].sz = fZ;
		d3dtlvFirst[2].rhw = fZ;
		d3dtlvFirst[2].color = dwColour;
		d3dtlvFirst[2].specular = 0;
		d3dtlvFirst[2].tu = fU2;
		d3dtlvFirst[2].tv = fV2;

		// Bottom left.
		d3dtlvFirst[3].sx = fX - fVec1X;
		d3dtlvFirst[3].sy = fY + fVec1Y;
		d3dtlvFirst[3].sz = fZ;
		d3dtlvFirst[3].rhw = fZ;
		d3dtlvFirst[3].color = dwColour;
		d3dtlvFirst[3].specular = 0;
		d3dtlvFirst[3].tu = fU1;
		d3dtlvFirst[3].tv = fV2;
		
		return TRUE;
	}

	// Returns success.
	// Sets up text to fade in at the bottom of the screen,
	// scroll to the top,
	// then fade out.
	bool CreateStandard (	int iXPos, int iWidth, int iHeight,
							float fU1In, float fV1In, float fU2In, float fV2In )
	{

		static float fShadeStart = 0.0f;

		fShadeStart += 0.02f;

		fU1 = fU1In;
		fV1 = fV1In;
		fU2 = fU2In;
		fV2 = fV2In;

		const float fTimeAlive = 20.0f;
		const float fFadeTime = 4.0f;
		const float fYStart = 480.0f;
		const float fYEnd = 0.0f;

		wcX				.SetConstant ( (float)iXPos );
		wcY				.SetIncreasing ( fYStart, ( fYEnd - fYStart ) / fTimeAlive );
		wcZ				.SetConstant ( 0.5f );
		wcXSize			.SetConstant ( (float)iWidth );
		wcYSize			.SetConstant ( (float)iHeight );
		wcRotation		.SetConstant ( 0.0f );
		wcShade			.SetIncreasing ( fShadeStart, 0.2f );
		wcBrightness	.SetRamp ( 0.0f, 2.0f, 0.0f, fFadeTime, fTimeAlive - fFadeTime, fTimeAlive );

		fLifetime = fTimeAlive;
		return TRUE;
	}

	// Returns success.
	// Copies the given WCs to the particle.
	bool CreateWibbling (	float fU1In, float fV1In, float fU2In, float fV2In,
							WibbleComponent &rwcX, WibbleComponent &rwcY, WibbleComponent &rwcZ,
							WibbleComponent &rwcXSize, WibbleComponent &rwcYSize,
							WibbleComponent &rwcRotation, WibbleComponent &rwcShade, WibbleComponent &rwcBrightness
							)
	{
		fU1 = fU1In;
		fV1 = fV1In;
		fU2 = fU2In;
		fV2 = fV2In;

		const float fTimeAlive = 20.0f;

		wcX				= rwcX			;	
		wcY				= rwcY			;	
		wcZ				= rwcZ			;	
		wcXSize			= rwcXSize		;	
		wcYSize			= rwcYSize		;	
		wcRotation		= rwcRotation	;	
		wcShade			= rwcShade		;	
		wcBrightness	= rwcBrightness	;

		fLifetime = fTimeAlive;
		return TRUE;
	}

};



#define MAX_PARTICLES 1024
TomPart *tpParticles;

D3DTLVERTEX *tlvertParticles;
WORD *wParticleIndices;


extern SLONG FONT2D_GetIndex(CBYTE chr);
extern SLONG FONT2D_GetLetterWidth(CBYTE chr);

extern FONT2D_Letter FONT2D_letter[];



#define GLOBAL_TEXT_SCALE 1.8f


#if 0
int GetLetterWidth ( char pc )
{
	return ( FontInfo[pc].width );
}
#else
int GetLetterWidth ( char pc )
{
	return ( GLOBAL_TEXT_SCALE * FONT2D_GetLetterWidth ( pc ) );
}
#endif






void OhNoThisIsActually ( char *pcString, int iX, int iY, float fZ )
{
	while ( *pcString != '\0' )
	{
		if ( *pcString != ' ' )
		{
			TomPart *tpCur = tpParticles;
			int iCurTPNum = 0;
			while ( TRUE )
			{
				if ( !tpCur->IsAlive() )
				{
					// Got one.
					break;
				}
				tpCur++;
				iCurTPNum++;
				if ( iCurTPNum == MAX_PARTICLES )
				{
					// Need more particles!
					ASSERT ( FALSE );
					tpCur = NULL;
					break;
				}
			}

			if ( tpCur != NULL )
			{
				int iLetter = FONT2D_GetIndex ( *pcString );
				FONT2D_Letter *fl = &FONT2D_letter[iLetter];

	#define MY_SCALE 0.9f

				// Check out the magic number 18. Not my fault - copied from font2d.cpp
				tpCur->CreateStandard ( iX + ( ( fl->width ) * 0.5f ),
										fl->width,
										18,
										fl->u,
										fl->v,
										fl->u + float(fl->width) * (1.0F / 256.0F),
										fl->v + 18.0F * (1.0F / 256.0F)
										);

				tpCur->wcBrightness	.SetIncreasing ( 2.0f, -2.0f );
				tpCur->wcY			.SetConstant ( iY );
				tpCur->wcZ			.SetConstant ( fZ );
				tpCur->fLifetime	= 1.0f;
			}
		}

		iX += GetLetterWidth ( *pcString );
		pcString++;
	}
}



static iDontDrawTooOften = 0;

void ThisMayWellBeTheLastFunctionEverInsertedIntoUrbanChaosAndIReallyMeanItThisTime ( void )
{

	// Darken screen.
	D3DTLVERTEX tlv[4];
	tlv[0].sx = 0.0f;
	tlv[0].sy = 0.0f;
	tlv[0].sz = 0.95f;
	tlv[0].rhw = 0.95f;
	tlv[0].color = 0xc0000000;
	tlv[0].specular = 0;
	tlv[0].tu = 0.0f;
	tlv[0].tv = 0.0f;

	tlv[1] = tlv[2] = tlv[3] = tlv[0];

	tlv[1].sx = 640.0f;
	tlv[2].sx = 640.0f;
	tlv[2].sy = 480.0f;
	tlv[3].sy = 480.0f;

	the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
	the_display.lp_D3D_Device->SetTexture ( 0, NULL );

	HRESULT hres = the_display.lp_D3D_Device->DrawPrimitive ( 
						D3DPT_TRIANGLEFAN,
						D3DVT_TLVERTEX,
						tlv,
						4,
						D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS );

	ASSERT ( SUCCEEDED ( hres ) );

	the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );



	iDontDrawTooOften++;
	if ( ( iDontDrawTooOften & 0xf ) != 0 )
	{
		return;
	}

extern BOOL AreAnyDevicesConnected ( void );
	if ( AreAnyDevicesConnected() )
	{
		// Display "No controller" message.

		char *pcString = XLAT_str ( X_CONTROLLER_REMOVED );


		// Put at bottom left.
		SLONG iXSize, iYSize;
		
		int iX = 32;
		int iY = 480-32-20;

		OhNoThisIsActually ( pcString, iX, iY, 0.97f );

	}
	else
	{
		// The big essay.
		// Centre it on all sides.
		char *pcString[3];
		if ( !IsEnglish )
		{
			pcString[0] = "Une manette vient d'etre";		// NOTE! The E in etre should be ê, but the font doesn't have it, and it's all in caps anyway.
			if ( bWriteVMInsteadOfVMU )
			{
				pcString[1] = "retirée ou une VM est";
			}
			else
			{
				pcString[1] = "retirée ou une VMU est";
			}
			pcString[2] = "en cours de détection";
		}
		else
		{
			pcString[0] = "The controller has been";
			if ( bWriteVMInsteadOfVMU )
			{
				pcString[1] = "removed or a VM";
			}
			else
			{
				pcString[1] = "removed or a VMU";
			}
			pcString[2] = "is being recognised";
		}



		// Put at bottom left.
		int iX = 32;
		int iY = 480-32-25*3;

		OhNoThisIsActually ( pcString[0], iX, iY, 0.97f );
		iY += 40;
		OhNoThisIsActually ( pcString[1], iX, iY, 0.97f );
		iY += 40;
		OhNoThisIsActually ( pcString[2], iX, iY, 0.97f );
		iY += 40;

	}

}





void DreamCastCredits ( void )
{


	// Load the credits text.
	MFFileHandle handle;
	if ( 0 == ENV_get_value_number ( "lang_num", 0, "" ) )
	{
		handle = FileOpen ( "Text\\Credits_eng.txt" );
	}
	else
	{
		handle = FileOpen ( "Text\\Credits_fr.txt" );
	}
	ASSERT ( handle != FILE_OPEN_ERROR );
	SLONG dwCreditsSize = FileSize ( handle );
	ASSERT ( dwCreditsSize > 0 );
	dwCreditsSize += 32;
	dwCreditsSize *= sizeof ( char );
	char *pcCreditsText = (char *)MemAlloc ( dwCreditsSize );
	FileRead ( handle, pcCreditsText, dwCreditsSize );
	FileClose ( handle );




	// Create the memory chunks required.
	ASSERT ( tpParticles == NULL );
	ASSERT ( tlvertParticles == NULL );
	ASSERT ( wParticleIndices == NULL );
	tpParticles			= (TomPart *)		MemAlloc ( MAX_PARTICLES * sizeof ( *tpParticles ) );
	tlvertParticles		= (D3DTLVERTEX *)	MemAlloc ( 4 * MAX_PARTICLES * sizeof ( *tlvertParticles ) );
	wParticleIndices	= (WORD *)			MemAlloc ( 6 * MAX_PARTICLES * sizeof ( *wParticleIndices ) );
	ASSERT ( tpParticles != NULL );
	ASSERT ( tlvertParticles != NULL );
	ASSERT ( wParticleIndices != NULL );

	for ( int i = 0; i < MAX_PARTICLES; i++ )
	{
		tpParticles[i].Kill();
	}

	char *pcCredits = pcCreditsText;


	// Special characters:
	// ('\r' is ignored)
#define SPECIAL_CR					'\n'
#define SPECIAL_ESC					'\\'
#define SPECIAL_ESC_NEW_SECTION		's'
#define SPECIAL_ESC_SECTION_HEAD	'h'
#define SPECIAL_ESC_TAB				't'


	float fCreditsFade = -1.0f;
#define CREDITS_FADE_IN_SPEED 0.2f
#define CREDITS_FADE_OUT_SPEED 1.0f


	// Set my default blending stuff up.
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_SRCALPHA );
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
	the_display.lp_D3D_Device->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );
	the_display.lp_D3D_Device->SetRenderState (D3DRENDERSTATE_FOGENABLE, FALSE );

	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

	the_display.lp_D3D_Device->SetTextureStageState ( 0, D3DTSS_TEXCOORDINDEX, 0 );

	the_display.lp_D3D_Device->SetTextureStageState ( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	the_display.lp_D3D_Device->SetTextureStageState ( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

extern LPDIRECT3DTEXTURE2 TEXTURE_get_handle(SLONG page);
#if 0
	LPDIRECT3DTEXTURE2 ptexFontTex = TEXTURE_get_handle ( TEXTURE_page_lcdfont );
#else
	LPDIRECT3DTEXTURE2 ptexFontTex = TEXTURE_get_handle ( TEXTURE_page_font2d );
#endif
	ASSERT ( ptexFontTex != NULL );


	float fCreditsCountdown = 0.0f;
	float fCreditsPeriod = 2.0f;

	float fGlobalTextScale = 2.0f;




	// Set up the inital WibbleComponent state.
#define NUM_WCS 8
	enum WibbleComponentName
	{
		WC_X = 0,
		WC_Y,
		WC_Z,
		WC_XSIZE,
		WC_YSIZE,
		WC_ROTATION,
		WC_SHADE,
		WC_BRIGHTNESS,

		WC_MAX_VALUE,			// Always last.
	};


	WibbleComponent wcBase[NUM_WCS];
	WibbleComponent wcDelta[NUM_WCS];
	enum WibbleComponentWibbled
	{
		WCW_OFFSET = 0,
		WCW_COS1AMP,
		WCW_COS1OFF,
		WCW_COS1POS,
		WCW_COS1INC,
		WCW_COS2AMP,
		WCW_COS2OFF,
		WCW_COS2POS,
		WCW_COS2INC,
		WCW_RAMPTIME,
		WCW_RAMPTIMEINC,
		WCW_RAMPUPSTART,
		WCW_RAMPUPEND,
		WCW_RAMPMIDVALUE,
		WCW_RAMPDOWNSTART,
		WCW_RAMPDOWNEND,
		WCW_RAMPBASEVALUE,

		WCW_MAX_VALUE,			// Always last.
	} wcwWibbled[NUM_WCS];


	const float fTimeAlive = 20.0f;
	const float fFadeTime = 4.0f;
	const float fYStart = 480.0f;
	const float fYEnd = 0.0f;

	wcBase[WC_X]			.SetConstant ( 320.0f );
	wcBase[WC_Y]			.SetIncreasing ( fYStart, ( fYEnd - fYStart ) / fTimeAlive );
	wcBase[WC_Z]			.SetConstant ( 0.5f );
	wcBase[WC_XSIZE]		.SetConstant ( 1.0f );
	wcBase[WC_YSIZE]		.SetConstant ( 1.0f );
	wcBase[WC_ROTATION]		.SetConstant ( 0.0f );
	wcBase[WC_SHADE]		.SetIncreasing ( 0.0f, 0.2f );
	wcBase[WC_BRIGHTNESS]	.SetRamp ( 0.0f, 2.0f, 0.0f, fFadeTime, fTimeAlive - fFadeTime, fTimeAlive );

	wcDelta[WC_X]			.SetIncreasing ( 320.0f, 1.0f );
	wcDelta[WC_Y]			.SetConstant ( 0.0f );
	wcDelta[WC_Z]			.SetConstant ( 0.5f );
	wcDelta[WC_XSIZE]		.SetConstant ( 0.0f );
	wcDelta[WC_YSIZE]		.SetConstant ( 0.0f );
	wcDelta[WC_ROTATION]	.SetConstant ( 0.0f );
	wcDelta[WC_SHADE]		.SetIncreasing ( 0.0f, 0.001f );
	wcDelta[WC_BRIGHTNESS]	.SetConstant ( 0.0f );

	// Which component of wcBase gets changed by the value of wcDelta.
	wcwWibbled[WC_X]			= WCW_OFFSET;
	wcwWibbled[WC_Y]			= WCW_OFFSET;
	wcwWibbled[WC_Z]			= WCW_OFFSET;
	wcwWibbled[WC_XSIZE]		= WCW_OFFSET;
	wcwWibbled[WC_YSIZE]		= WCW_OFFSET;
	wcwWibbled[WC_ROTATION]		= WCW_OFFSET;
	wcwWibbled[WC_SHADE]		= WCW_OFFSET;
	wcwWibbled[WC_BRIGHTNESS]	= WCW_OFFSET;



	DWORD dwTimeGetTime = timeGetTime();

	// Main loop
	while ( TRUE )
	{

		DWORD dwNewTimeGetTime = timeGetTime();
		DWORD dwTGTDiff = dwNewTimeGetTime - dwTimeGetTime;
		dwTimeGetTime = dwNewTimeGetTime;

		if ( dwTGTDiff > 100 )
		{
			// Limit to 10fps speed.
			dwTGTDiff = 100;
		}

		// Convert to seconds.
		float fTimeStep = (float)dwTGTDiff * 0.001f;


		float fCreditsFadeAmount = 1.0f;
		if ( fCreditsFade < 0.0f )
		{
			// Fade in.
			fCreditsFade += fTimeStep * CREDITS_FADE_IN_SPEED;
			if ( fCreditsFade > 0.0f )
			{
				fCreditsFade = 0.0f;
				fCreditsFadeAmount = 1.0f;
			}
			else
			{
				fCreditsFadeAmount = 1.0f + fCreditsFade;
			}
		}
		else if ( fCreditsFade > 1.0f )
		{
			// Fade out.
			fCreditsFade += fTimeStep * CREDITS_FADE_OUT_SPEED;
			if ( fCreditsFade > 2.0f )
			{
				fCreditsFadeAmount = 0.0f;
			}
			else
			{
				fCreditsFadeAmount = 2.0f - fCreditsFade;
			}
		}

		//TRACE ( "%f ", fCreditsFadeAmount );



		// Prep the screen draw.
		the_display.lp_D3D_Viewport->Clear2 ( 0, NULL, D3DCLEAR_TARGET, 0x000000ff, 0.0f, 0 );



		// Run and draw the particles.
		D3DTLVERTEX *ptlvertCur = tlvertParticles;
		WORD *pwCurInd = wParticleIndices;
		int iVertNum = 0;
		int iNumIndices = 0;

		for ( int i = 0; i < MAX_PARTICLES; i++ )
		{
			if ( tpParticles[i].Run ( fTimeStep ) )
			{
				// Still alive, so try to draw it.
				if ( tpParticles[i].Draw ( ptlvertCur, fCreditsFadeAmount ) )
				{
					// Drew successfully - add it to the index list.
					pwCurInd[0] = iVertNum + 0;
					pwCurInd[1] = iVertNum + 1;
					pwCurInd[2] = iVertNum + 3;
					pwCurInd[3] = iVertNum + 3;
					pwCurInd[4] = iVertNum + 1;
					pwCurInd[5] = iVertNum + 2;

					iVertNum += 4;
					ptlvertCur += 4;
					pwCurInd += 6;
					iNumIndices += 6;
				}
			}
		}

		if ( iNumIndices > 0 )
		{
			the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			the_display.lp_D3D_Device->SetTexture ( 0, ptexFontTex );

			HRESULT hres = the_display.lp_D3D_Device->DrawIndexedPrimitive ( 
								D3DPT_TRIANGLELIST,
								D3DVT_TLVERTEX,
								tlvertParticles,
								iVertNum,
								wParticleIndices,
								iNumIndices,
								D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS );

			ASSERT ( SUCCEEDED ( hres ) );

			the_display.lp_D3D_Device->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		}


		// Draw the background.
		int iColR = (float)( 256.0f * ( 1.0f - 0.5f * fCreditsFadeAmount ) );
		int iColG = iColR;
		int iColB = iColR;
		int iColA = 255;
		if ( iColR < 0 )
		{
			iColR = 0;
		}
		else if ( iColR > 255 )
		{
			iColR = 255;
		}
		if ( iColG < 0 )
		{
			iColG = 0;
		}
		else if ( iColG > 255 )
		{
			iColG = 255;
		}
		if ( iColB < 0 )
		{
			iColB = 0;
		}
		else if ( iColB > 255 )
		{
			iColB = 255;
		}
		if ( iColA < 0 )
		{
			iColA = 0;
		}
		else if ( iColA > 255 )
		{
			iColA = 255;
		}

		DWORD dwColour = ( iColA << 24 ) | ( iColR << 16 ) | ( iColG << 8 ) | ( iColB << 0 );

		// Top left.
		tlvertParticles[0].sx = 0.0f;
		tlvertParticles[0].sy = 0.0f;
		tlvertParticles[0].sz = 0.999f;
		tlvertParticles[0].rhw = 0.001f;
		tlvertParticles[0].color = dwColour;
		tlvertParticles[0].specular = 0;
		tlvertParticles[0].tu = 0.0f;
		tlvertParticles[0].tv = 0.0f;

		// Top right.
		tlvertParticles[1].sx = 640.0f;
		tlvertParticles[1].sy = 0.0f;
		tlvertParticles[1].sz = 0.999f;
		tlvertParticles[1].rhw = 0.001f;
		tlvertParticles[1].color = dwColour;
		tlvertParticles[1].specular = 0;
		tlvertParticles[1].tu = 640.0f / 1024.0f;
		tlvertParticles[1].tv = 0.0f;

		// Bottom right.
		tlvertParticles[2].sx = 640.0f;
		tlvertParticles[2].sy = 480.0f;
		tlvertParticles[2].sz = 0.999f;
		tlvertParticles[2].rhw = 0.001f;
		tlvertParticles[2].color = dwColour;
		tlvertParticles[2].specular = 0;
		tlvertParticles[2].tu = 640.0f / 1024.0f;
		tlvertParticles[2].tv = 480.0f / 512.0f;

		// Bottom left.
		tlvertParticles[3].sx = 0.0f;
		tlvertParticles[3].sy = 480.0f;
		tlvertParticles[3].sz = 0.999f;
		tlvertParticles[3].rhw = 0.001f;
		tlvertParticles[3].color = dwColour;
		tlvertParticles[3].specular = 0;
		tlvertParticles[3].tu = 0.0f;
		tlvertParticles[3].tv = 480.0f / 512.0f;

		wParticleIndices[0] = 0;
		wParticleIndices[1] = 1;
		wParticleIndices[2] = 3;
		wParticleIndices[3] = 3;
		wParticleIndices[4] = 1;
		wParticleIndices[5] = 2;
		
		the_display.lp_D3D_Device->SetTexture ( 0, the_display.lp_DD_Background_use_instead_texture );

		HRESULT hres = the_display.lp_D3D_Device->DrawIndexedPrimitive ( 
							D3DPT_TRIANGLELIST,
							D3DVT_TLVERTEX,
							tlvertParticles,
							4,
							wParticleIndices,
							6,
							D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS );

		// Display any crappy "no controller" message.
extern DIDeviceInfo *primary_device;
		if ( primary_device == NULL )
		{
			ThisMayWellBeTheLastFunctionEverInsertedIntoUrbanChaosAndIReallyMeanItThisTime();
		}


		the_display.lp_DD_FrontSurface->Flip ( NULL, DDFLIP_WAIT );

		// Get input.
		DWORD dwInput = get_hardware_input ( INPUT_TYPE_JOY );

		if ( ( dwInput & INPUT_MASK_START ) && ( fCreditsFade < 1.0001f ) )
		{
			// Start the fadeout.
			fCreditsFade = 1.0001f;
		}

		if ( fCreditsFade > 2.0f )
		{
			// Fully faded - quit.
			break;
		}



		// Handle the credits text.
		fCreditsCountdown -= fTimeStep;
		if ( fCreditsCountdown < 0.0f )
		{
			fCreditsCountdown += fCreditsPeriod;
			if ( fCreditsCountdown < 0.0f )
			{
				// Panic - too fast.
				fCreditsCountdown = 0.0f;
			}

			// Generate next line of characters.

			// Skip leading newlines and whitespace, and track any "line" escape characters.
			bool bHeading = FALSE;
			bool bSectionBreak = FALSE;
			bool bFoundSomething = FALSE;
			bool bRightJustify = FALSE;
			while ( TRUE )
			{
				switch ( *pcCredits )
				{
				case '\\':
					// Escape character.
					{
						pcCredits++;
						switch ( *pcCredits )
						{
						case 's':
							// New section - do a blank line.
							bSectionBreak = TRUE;
							pcCredits++;
							bFoundSomething = TRUE;
							break;
						case 'h':
							// Heading.
							bHeading = TRUE;
							break;
						case 'r':
						case 't':
							// Right-justify
							bRightJustify = TRUE;
							break;
						case 'l':
							// Half-line gap.
							fCreditsCountdown += fCreditsPeriod * 0.5f;
							break;
						default:
							break;
						}
					}
					break;
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					// Whitespace - ignore.
					break;
				case '\0':
					// Hit the end of the file!
#if 0
					// Start again from the beginning.
					pcCredits = pcCreditsText;
#else
					// Er... no. Eidos want us to exit. Whatever...
					pcCredits = pcCreditsText;
					// Start the fadeout.
					fCreditsFade = 1.0001f;
#endif
					bFoundSomething = TRUE;
					break;
				default:
					// Found something to print.
					bFoundSomething = TRUE;
					break;
				}

				if ( bFoundSomething )
				{
					break;
				}
				pcCredits++;
			}

			// First scan the line for spacing info.
			int iTabCount = 0;
			int iTotalWidth = 0;
			char *pcStart = pcCredits;
			bool bFoundTheEndOfTheLine = FALSE;
			while ( !bFoundTheEndOfTheLine )
			{
				switch ( *pcCredits )
				{
				case '\\':
					{
						// Escape sequence
						pcCredits++;
						switch ( *pcCredits )
						{
						case 't':
							// Tab.
							iTabCount++;
							break;
						case 's':
							// Some sort of break.
							bFoundTheEndOfTheLine = TRUE;
							break;
						default:
							// Something else - ignore.
							break;
						}
					}
					break;

				case '\r':
					// Ignore.
					break;

				case '\n':
					// End of line.
					bFoundTheEndOfTheLine = TRUE;
					break;

				default:
					iTotalWidth += GetLetterWidth ( *pcCredits );
					break;
				}

				pcCredits++;
			}


			// OK, so now we know how wide this is, and how many tabs it has.
			// So how wide should it be, and how big is each tab?
			int iTabSize = 3 * GetLetterWidth ( ' ' );
			int iLeftHandEdge = 0;
			if ( bHeading )
			{
				// Should be centered.
				// Any tabs will be the size of three spaces by default.
				iLeftHandEdge = ( 640 - iTotalWidth - iTabSize * iTabCount ) >> 1;
			}
			else if ( bRightJustify )
			{
				iLeftHandEdge = ( 640 - 20 - iTotalWidth - iTabSize * iTabCount );
			}
			else
			{
				// Full-width, with tabs taking equal spacing.
				// If no tabs, left-justified.
				iLeftHandEdge = 20;
				iTabSize = 640 - ( iLeftHandEdge << 1 ) - iTotalWidth;
				if ( iTabCount == 0 )
				{
					// Leave left-justified.
				}
				else
				{
					iTabSize /= iTabCount;
				}
			}


			// Now "draw" it.
			int iCurTPNum = 0;
			TomPart *tpCur = tpParticles;
			int iXPos = iLeftHandEdge;


			while ( pcStart != pcCredits )
			{
				switch ( *pcStart )
				{
				case '\\':
					// Escape sequence;
					{
						pcStart++;
						switch ( *pcStart )
						{
						case 't':
							// Tab.
							iXPos += iTabSize;
							break;
						default:
							// Ignore.
							break;
						}
					}
					break;
				case '\r':
				case '\n':
					// Ignore.
					break;

				case ' ':
					// Space - not a particle.
					iXPos += GetLetterWidth ( *pcStart );
					break;

				default:
					// Normal letter - print it.

					// Find a free letter.
					while ( TRUE )
					{
						if ( !tpCur->IsAlive() )
						{
							// Got one.
							break;
						}
						tpCur++;
						iCurTPNum++;
						if ( iCurTPNum == MAX_PARTICLES )
						{
							// Need more particles!
							ASSERT ( FALSE );
							tpCur = NULL;
							break;
						}
					}

					if ( tpCur != NULL )
					{
						// Create this particle then.

#if 1
// Old style.

#if 0
						CharData *pci = &(FontInfo[*pcStart]);

						// Check out the magic number 18. Not my fault - copied from font2d.cpp
						tpCur->CreateStandard ( iXPos,
												pci->width,
												pci->height,
												pci->x,
												pci->y,
												pci->ox,
												pci->oy
												);
#else
						int iLetter = FONT2D_GetIndex ( *pcStart );
						FONT2D_Letter *fl = &FONT2D_letter[iLetter];

						// Check out the magic number 18. Not my fault - copied from font2d.cpp
						tpCur->CreateStandard ( iXPos + ( ( fl->width * GLOBAL_TEXT_SCALE ) * 0.5f ),
												fl->width * GLOBAL_TEXT_SCALE,
												18 * GLOBAL_TEXT_SCALE,
												fl->u,
												fl->v,
												fl->u + float(fl->width) * (1.0F / 256.0F),
												fl->v + 18.0F * (1.0F / 256.0F)
												);


#endif



#else
// Dodgy new style.
						int iLetter = FONT2D_GetIndex ( *pcStart );
						FONT2D_Letter *fl = &FONT2D_letter[iLetter];


						for ( int iWC = 0; iWC < NUM_WCS; iWC++ )
						{
							float fValue = wcDelta[iWC].EvalAt ( (float)( iXPos - 320 ) );
							switch ( wcwWibbled[iWC] )
							{
							case WCW_OFFSET:		wcBase[iWC].fOffset			= fValue; break;
							case WCW_COS1AMP:		wcBase[iWC].fCos1Amp		= fValue; break;
							case WCW_COS1OFF:		wcBase[iWC].fCos1Off		= fValue; break;
							case WCW_COS1POS:		wcBase[iWC].fCos1Pos		= fValue; break;
							case WCW_COS1INC:		wcBase[iWC].fCos1Inc		= fValue; break;
							case WCW_COS2AMP:		wcBase[iWC].fCos2Amp		= fValue; break;
							case WCW_COS2OFF:		wcBase[iWC].fCos2Off		= fValue; break;
							case WCW_COS2POS:		wcBase[iWC].fCos2Pos		= fValue; break;
							case WCW_COS2INC:		wcBase[iWC].fCos2Inc		= fValue; break;
							case WCW_RAMPTIME:		wcBase[iWC].fRampTime		= fValue; break;
							case WCW_RAMPTIMEINC:	wcBase[iWC].fRampTimeInc	= fValue; break;
							case WCW_RAMPUPSTART:	wcBase[iWC].fRampUpStart	= fValue; break;
							case WCW_RAMPUPEND:		wcBase[iWC].fRampUpEnd		= fValue; break;
							case WCW_RAMPMIDVALUE:	wcBase[iWC].fRampMidValue	= fValue; break;
							case WCW_RAMPDOWNSTART:	wcBase[iWC].fRampDownStart	= fValue; break;
							case WCW_RAMPDOWNEND:	wcBase[iWC].fRampDownEnd	= fValue; break;
							case WCW_RAMPBASEVALUE:	wcBase[iWC].fRampBaseValue	= fValue; break;
							default:
								ASSERT ( FALSE );
								break;
							}

							wcBase[iWC].ChangedSomething();
						}


						tpCur->CreateWibbling (	
												fl->u,
												fl->v,
												fl->u + float(fl->width) * (1.0F / 256.0F),
												fl->v + 18.0F * (1.0F / 256.0F),
												wcBase[WC_X],
												wcBase[WC_Y],
												wcBase[WC_Z],
												wcBase[WC_XSIZE],
												wcBase[WC_YSIZE],
												wcBase[WC_ROTATION],
												wcBase[WC_SHADE],
												wcBase[WC_BRIGHTNESS]
											);


#endif

					}

					iXPos += GetLetterWidth ( *pcStart );

					break;
				}

				pcStart++;
			}
		}
	}


	the_display.lp_D3D_Device->SetTexture ( 0, NULL );


	// Free the memory.
	ASSERT ( tpParticles != NULL );
	ASSERT ( tlvertParticles != NULL );
	ASSERT ( wParticleIndices != NULL );
	MemFree ( tpParticles );
	MemFree ( tlvertParticles );
	MemFree ( wParticleIndices );
	tpParticles = NULL;
	tlvertParticles = NULL;
	wParticleIndices = NULL;

	ASSERT ( pcCreditsText != NULL );
	MemFree ( pcCreditsText );
	pcCreditsText = NULL;


}


#endif
