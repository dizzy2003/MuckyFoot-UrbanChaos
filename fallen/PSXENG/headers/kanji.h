#include "psxeng.h"

#ifndef _KANJI_H
#define _KANJI_H
#ifdef VERSION_KANJI

extern void Kanji_Init(UWORD x,UWORD y);

extern void Kanji_string(SLONG x,SLONG y,UWORD *str,SLONG col,SLONG scale);

extern void Kanji_Debug();

#endif
#endif