/*
** MCARD.h
**
** Memory card function defines
*/

#ifndef _MCARD_H
#define _MCARD_H

#define MCARD_ACCEPT_OFFSET		0

#define MCARD_OKAY				0
#define MCARD_NO_CARD			1
#define MCARD_INVALID			2
#define MCARD_NEW_CARD			3
#define MCARD_NOT_FORMATTED		4
#define MCARD_NOT_FOUND			5
#define MCARD_EXISTS			6
#define MCARD_FULL				7
#define MCARD_ERROR				8

#define MCARD_READ_OFFSET		0x10

#define MCARD_READ_OKAY			0x10
#define MCARD_READ_NO_CARD		0x11
#define MCARD_READ_INVALID		0x12
#define MCARD_READ_NEW_CARD		0x13
#define MCARD_READ_NOT_FOUND	0x15
#define MCARD_READ_ERROR		0x18

#define MCARD_WRITE_OFFSET		0x20

#define MCARD_WRITE_OKAY		0x20
#define MCARD_WRITE_NO_CARD		0x21
#define MCARD_WRITE_INVALID		0x22
#define MCARD_WRITE_NEW_CARD	0x23
#define MCARD_WRITE_NOT_FOUND	0x25
#define MCARD_WRITE_ERROR		0x28

#define MCARD_FIND_OFFSET		0x30

#define MCARD_FIND_OKAY			0x30
#define MCARD_FIND_NO_CARD		0x31
#define MCARD_FIND_INVALID		0x32
#define MCARD_FIND_NO_FORMAT	0x34
#define MCARD_FIND_NOT_FOUND	0x35
#define MCARD_FIND_EXISTS		0x36

#define MCARD_NEW_OFFSET		0x40

#define MCARD_NEW_OKAY			0x40
#define MCARD_NEW_NO_CARD		0x41
#define MCARD_NEW_INVALID		0x42
#define MCARD_NEW_NO_FORMAT		0x44
#define MCARD_NEW_EXISTS		0x46
#define MCARD_NEW_FULL			0x47

#define MCARD_FORMAT_OFFSET		0x50

#define MCARD_FORMAT_OKAY		0x50
#define MCARD_FORMAT_NO_CARD	0x51
#define MCARD_FORMAT_INVALID	0x52
#define MCARD_FORMAT_NEW_CARD	0x53

#define MCARD_SPACE_OKAY		0x60
#define MCARD_SPACE_FULL		0x67

#define MCARD_NOT_SCANNED		0x80
#define MCARD_SCANNING			0x81
#define MCARD_READING			0x82
#define MCARD_WRITING			0x83
#define MCARD_INIT				0x84
#define MCARD_NOT_AVAILABLE		0x8f

extern void MCARD_Init();
extern void MCARD_Final();
extern SLONG MCARD_Scan();
extern SLONG MCARD_Status();
extern SLONG MCARD_Format();
extern SLONG MCARD_ReadFile(char *fname,UBYTE *addr,SLONG len);
extern SLONG MCARD_WriteFile(char *fname,UBYTE *addr,SLONG len);
extern SLONG MCARD_FindFile(char *fname);
extern SLONG MCARD_CreateFile(char *fname,SLONG blocks);
extern SLONG MCARD_FindSpace(SLONG blocks);

#define MCARD_STATUS(x) ((x)&0x8f)

#endif
