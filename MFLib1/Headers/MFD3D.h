// MFD3D.h
// Guy Simmons, 13th May 1997.

#ifndef	MFD3D_H
#define	MFD3D_H

#define	D3D_HARDWARE		(1<<0)
#define	D3D_GOURAUD			(1<<1)
#define	D3D_ZBUFFER			(1<<2)
#define	D3D_RAMP			(1<<3)

extern BOOL					HasHardware;
extern IDirect3DDevice2		*lp_D3D_Device;
extern LPDIRECT3D2			lp_D3D_2;

//----------------------------------------------------------------------

void	SetupD3D2(void);
void	ResetD3D2(void);
BOOL	ChooseD3DDevice(ULONG flags);

//----------------------------------------------------------------------



#endif
