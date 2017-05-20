#include	<MFHeader.h>

#include	"I86.h"
#include	"string.h"
#include	"stdio.h"
#include	"conio.h"
#include	"dos.h"


volatile UBYTE	AltFlag,
				ControlFlag,
				ShiftFlag;
volatile UBYTE	Keys[256],
				LastKey;

//*************************************KEYBOARD STUFF ****************
static	SWORD	keyboard_patched=0;
static	UBYTE				oinkey;
static	void __interrupt far (*old_int)(void);
static	void __interrupt far Keyboard_Int(void);

BOOL	SetupKeyboard(void)
{
	SWORD	c0;
	if(keyboard_patched)
		return(0);

	keyboard_patched=1;

	for (c0 = 0; c0 < 128; c0++)
	{
		Keys[c0] = 0;
	}
	old_int = _dos_getvect(0x9);
	_dos_setvect(0x9, Keyboard_Int);
	
	return (1);
}

void	ResetKeyboard(void)
{
	if(!keyboard_patched)
		return;

	_dos_setvect(0x9, old_int);
	
	return;
}

void	__interrupt far Keyboard_Int(void)
{
	SWORD	val,val2;
	LastKey = (UBYTE)inp(0x060);
	if (oinkey == 0xE0)
	{
  //		lbExtendedKeyPress = 1;
		if ((LastKey == 0x2A) || (LastKey == 0xaa))
		{
			oinkey = LastKey;
			LastKey = 128;
		}
		else
		{
			oinkey = LastKey;
			val = (UWORD)(LastKey | 0x0080);	// Set bit eight. 
			if (LastKey > 0x07f)
			{
				Keys[val] = 0;		// This is an extended key. To distinguish it form the other key with this code we
			}
			else				  	// change the 'key on' array position set. We can't use positions below 128 (used 
			{
				Keys[val] = 1;		// or reserved). The val is put in the region 128 to 255 inclusive.
			}
		}
	}	
	else
	{
//		lbExtendedKeyPress = 0;
		oinkey = LastKey;
		val = (UWORD)(LastKey & 0x007f);		// Clear bit eight.
		if (LastKey > 0x07f)
		{
			Keys[val] = 0;
		}
		else
		{
			Keys[val] = 1;
		}
	}
	val = (SWORD)inp(0x061);
	val2 = val;
	val |= 0x080;
	outp(0x061, val);
	outp(0x061, val2);
/*
	if ((LastKey < 128) && !lbIInkey)
	{
		lbIInkey=LastKey;
	}
*/	
	outp(0x20,0x20);
}

