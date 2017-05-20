// KFDef2.c
// Guy Simmons, 20th September 1997.


//---------------------------------------------------------------

ControlDef	kframe_ctrls_def[]	=	
{
	{	BUTTON,			0,	"Load Keyframe Chunk",	2,	2,	0,						10			},
	{	H_SLIDER,		0,	"",						2,	(KEY_FRAME_IMAGE_SIZE<<1)-18,	KEY_FRAME_COUNT*KEY_FRAME_IMAGE_SIZE,	0			},

	{	0	}
};

//---------------------------------------------------------------
