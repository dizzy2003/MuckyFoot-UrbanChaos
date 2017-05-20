#ifdef THIS_IS_INCLUDED_FROM_SW


{
	SLONG i;
	SLONG x;

	ULONG *dest, addr, *tex;
	ULONG  pixel;
	SLONG  a;
	SLONG  r, rd;
	SLONG  g, gd;
	SLONG  b, bd;
	SLONG  R;
	SLONG  G;
	SLONG  B;
	SLONG  pr;
	SLONG  pg;
	SLONG  pb;
	SLONG  u, ud, tempu;
	SLONG  v, vd, tempv;
	SLONG  U;
	SLONG  V;
	ULONG	wrap, wrap1, wrap2;
	int tempx1,tempx2;

	i = line;


	ULONG *last_dest;
	_int64 mmt1, mmt2, mmt3, mmt4, umask, vmask, wrapmask, uinc, vinc, notumask, notvmask, alpha_test_value, alpha_mask;
	ULONG utemp, vtemp;

	umask = 0x5555ffff5555ffff;
	vmask = 0xaaaaffffaaaaffff;
	notumask = ~0x5555ffff5555ffff;
	notvmask = ~0xaaaaffffaaaaffff;
	alpha_test_value = 0xff000000ff000000;
	alpha_mask = 0x00007fff00000000;

#define SWIZZLE 1


#define ALPHA_BLEND_NOT_DOUBLE_LIGHTING 1

	{
		{
			//r = ss->r;
			//g = ss->g;
			//b = ss->b;
			//u = ss->u;
			//v = ss->v;
			//ud = ss->du;
			//vd = ss->dv;

			tempx1 = ss->x1;
			tempx2 = ss->x2;

			if ( tempx2 > tempx1 )
			{

				dest = SW_buffer + i * SW_buffer_pitch;
				last_dest = dest + tempx2;
				dest += tempx1;
				if ( ( tempx2 & 0x1 ) != 0 ) 
				{
					// Last pixel is odd - go one less so the loop ends properly.
					// Remember that we are stepping two pixels at a time, and
					// ending when we hit this one, then maybe drawing the last pixel.

					// (e.g. x2 is 7, so last pixel to draw is 6, so terminate when we get to 6 - then do one more)
					// (e.g. x2 is 8, so last pixel to draw is 7, so terminate when we get to 8)

					last_dest--;
				}

				a  = (ss->a );
				r  = (ss->r )>>1;
				g  = (ss->g )>>1;
				b  = (ss->b )>>1;
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				rd = (ss->dr)>>1;
				gd = (ss->dg)>>1;
				bd = (ss->db)>>1;
#endif
				// Swizzle interpolators
				u = ss->u;
				v = ss->v;
				ud = ss->du;
				vd = ss->dv;
				tex = (ULONG *)(ss->tga);

				// Remember this is for DWORDS - assembler doesn't automagically scale.
#if SWIZZLE
				wrap = ((ss->tga_size * ss->tga_size) - 1)<<2;
#else
				wrap = ss->tga_size;
#endif

				__asm {

				push esi
				push edi
				push eax
				push ebx
				push ecx
				push edx

#if SWIZZLE
				movd	mm0,[wrap]
				movq	mm1,mm0
				psllq	mm0,32
				por		mm0,mm1
				movq	[wrapmask],mm0
#else
				mov		eax,[wrap]
				mov		ebx,0x200
				mov		ecx,9
wrap_loop:
				dec		ecx
				shr		ebx,1
				cmp		ebx,eax
				jne		wrap_loop

				mov		eax,[v]
				mov		ebx,[vd]
				shl		eax,cl
				shl		ebx,cl
				mov		[v],eax
				mov		[vd],ebx

				mov		eax,[wrap]
				dec		eax
				movd	mm0,eax
				psllq	mm0,16
				movq	mm1,mm0
				psllq	mm0,32
				por		mm0,mm1
				movq	[umask],mm0
				movd	mm1,ecx
				psllq	mm0,mm1
				movq	[vmask],mm0
#endif

				mov		eax,[r]
				mov		ebx,[g]
				mov		ecx,[b]
				and		eax,0x00ffff00
				and		ebx,0x00ffff00
				and		ecx,0x00ffff00
				movd	mm0,ebx
				movd	mm1,ecx
				shr		eax,8
				psllq	mm0,8
				psrlq	mm1,8
				por		mm0,mm1

#if (ALPHA_MODE==ALPHA_BLEND)
				mov		ecx,[a]
//				and		ecx,0x00ff00
#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				shl		ecx,23
#else
				shl		ecx,23-1
#endif
				or		eax,ecx
#else

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				or		eax,0x7fff0000
#else
				or		eax,0x40000000
#endif
#endif
				movd	mm1,eax
				psllq	mm1,32
				por		mm0,mm1
				movq	[mmt1],mm0


#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				mov		eax,[rd]
				mov		ebx,[gd]
				mov		ecx,[bd]
				and		eax,0x00ffff00
				and		ebx,0x00ffff00
				and		ecx,0x00ffff00
				movd	mm0,ebx
				movd	mm1,ecx
				shr		eax,8
				psllq	mm0,8
				psrlq	mm1,8
				por		mm0,mm1
				movd	mm1,eax
				psllq	mm1,32
				por		mm0,mm1
				movq	[mmt2],mm0
#endif

#if SWIZZLE
				mov		eax,[u]
				mov		ebx,[v]
				lea		esi,[swizzle_table]
				mov		ecx,eax
				mov		edx,ebx
				shr		ecx,16
				shr		edx,16
				and		ecx,0xf
				and		edx,0xf
				shr		eax,20
				shr		ebx,20
				and		eax,0xf
				and		ebx,0xf
				mov		ah,[esi+eax]
				mov		bh,[esi+ebx]
				mov		al,[esi+ecx]
				mov		bl,[esi+edx]
				shl		ebx,1
				mov		WORD PTR[u+2],ax
				mov		WORD PTR[v+2],bx

				mov		eax,[ud]
				mov		ebx,[vd]
				mov		ecx,eax
				mov		edx,ebx
				shr		ecx,16
				shr		edx,16
				and		ecx,0xf
				and		edx,0xf
				shr		eax,20
				shr		ebx,20
				and		eax,0xf
				and		ebx,0xf
				mov		ah,[esi+eax]
				mov		bh,[esi+ebx]
				mov		al,[esi+ecx]
				mov		bl,[esi+edx]
				shl		ebx,1
				or		eax,0xaaaa
				or		ebx,0x5555
				mov		WORD PTR[ud+2],ax
				mov		WORD PTR[vd+2],bx
#endif



				movq	mm0, [mmt1]
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				movq	mm2, [mmt2]
#endif
				mov		edi, [dest]
				mov		esi, [tex]


				// Set up the integer texel fetcher
				mov			eax,[u]
				mov			ebx,[v]



				;Is the first pixel odd?
				mov		ecx,[tempx1]
				test	ecx,0x1
				jnz		first_pixel_odd



#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				// Calculate r1g1b1, and double up the increments
				movq	mm1,mm0
				paddsw	mm1,mm2
				psllw	mm2,1
#endif




#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff

				mov		edx,eax
				add		eax,DWORD PTR [ud]
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
				or		ecx,edx
				shr		ecx,16-2
				push	ecx

				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
				or		edx,ecx
				shr		edx,16-2
				pop		ecx
#endif
				movd	mm7,[esi+ecx]
				movd	mm6,[esi+edx]


				jmp		setup_main_loop


first_pixel_odd:


#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
#else
				mov		ecx,eax
				mov		edx,ebx
				add		eax,DWORD PTR [ud]
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
				or		ecx,edx
				shr		ecx,16-2
#endif
				movd	mm7,[esi+ecx]




#if ALPHA_MODE==ALPHA_BLEND
		

				punpcklbw mm7,mm7			;unpack
	#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
	#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
	#endif

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				psrlw	mm7,8-1
#else
				psrlw	mm7,8-2
#endif
	#if SWIZZLE
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				mov		edx,ebx
				add		ebx,DWORD PTR [vd]
	#endif
				pmulhw	mm7,mm0				;colour modulate
				movq	mm6,mm7

	#if SWIZZLE
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
	#endif
				psrlq	mm6,8+1				;half required value (creates a sign bit)
				pand	mm6,[alpha_mask]
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		ecx,edx
				shr		ecx,16-2
	#endif
				movq	mm5,mm6
	#if SWIZZLE
				mov		edx,eax
				add		eax,DWORD PTR [ud]
	#else
				push	ecx
				mov		ecx,eax
	#endif

				psrlq	mm5,16
	#if SWIZZLE
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
	#endif

				por		mm5,mm6
	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
	#endif
				psrlq	mm5,16
	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				and		edx,DWORD PTR [vmask]
				or		edx,ecx
	#endif
				por		mm5,mm6				;alpha channel replicated to r,g,b
	#if SWIZZLE
	#else
				pop		ecx
				shr		edx,16-2
	#endif
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format
				// Remember that mm2 is spare - no gouard shading on alpha-blenders.

				movd	mm6,[edi]

				punpcklbw mm6,mm6			;unpack dest
				psrlw	mm6,8
				
				psubsw	mm7,mm6				;src-dest
				pmulhw	mm7,mm5				;alpha*(src-dest)
				
				paddsw	mm7,mm7				;*2
				paddsw	mm7,mm6				;dest+alpha*(src-dest)
				
				packuswb mm7,mm7			;re-pack


				movd	[edi],mm7			;store pixel

				add		edi,4



#else //#if ALPHA_MODE==ALPHA_BLEND

				punpcklbw mm7,mm7			;unpack
	#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				or		ecx,ebx
	#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
	#endif
				psrlw	mm7,8-2
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
	#endif
				pmulhw	mm7,mm0				;colour modulate
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		ecx,edx
				shr		ecx,16-2
				push	ecx
	#endif
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				paddsw	mm0,mm2				;inc rgb
#endif
	#if SWIZZLE
				mov		edx,eax
				add		eax,DWORD PTR [ud]
	#else
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
				mov		edx,ebx
	#endif
				packuswb mm7,mm7			;re-pack
	#if SWIZZLE
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				add		ebx,DWORD PTR [vd]
				and		ecx,DWORD PTR [umask]
	#endif

#if ALPHA_MODE==ALPHA_ADD
				movd	mm6,[edi]
				paddusb	mm7,mm6
#elif ALPHA_MODE==ALPHA_TEST
				movq	mm5,mm7
				movq	mm6,[edi]
				pcmpgtb	mm7,[alpha_test_value]	;SIGNED compare!
				psrad	mm7,24				;propogate compare result down.
				pand	mm6,mm7
				pandn	mm7,mm5				;mask (mm7 = (mm5 AND ~mm7))
				por		mm7,mm6				;...and merge
#endif

	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				and		edx,DWORD PTR [vmask]
				or		edx,ecx
	#endif

				movd	[edi],mm7			;store pixel


// KLUDGE!
//				mov		DWORD PTR [edi],0x7f7f7f7f


	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				pop		ecx
				shr		edx,16-2
	#endif

				add		edi,4


#endif //#else //#if ALPHA_MODE==ALPHA_BLEND




#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				// Calculate r1g1b1, and double up the increments
				movq	mm1,mm0
				paddsw	mm1,mm2
				psllw	mm2,1
#endif





#if SWIZZLE
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				or		ecx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		ecx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		ecx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
//
//				mov		edx,eax
//				add		eax,DWORD PTR [ud]
//				or		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		edx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		edx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
#else
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				mov		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				and		ecx,DWORD PTR [umask]
//				and		edx,DWORD PTR [vmask]
//				or		ecx,edx
//				shr		ecx,16-2
//				push	ecx
//
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				mov		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				and		ecx,DWORD PTR [umask]
//				and		edx,DWORD PTR [vmask]
//				or		edx,ecx
//				pop		ecx
//				shr		edx,16-2
#endif
				
				movd	mm7,[esi+ecx]
				movd	mm6,[esi+edx]


setup_main_loop:

				cmp		edi,[last_dest]
				je		main_loop_done





hor_loop:
				// Map:
				// MM0: (all 8.8): xx,r0,g0,b0
				// MM1: (all 8.8): xx,r1,g1,b1		(not for blend+add)
				// MM2: (all 8.8): xx,r,g,b deltas	(not for blend+add)
				// eax: (16.16): u
				// ebx: (16.16): v
				// esi: texture base addr
				// edi: dest addr




#if ALPHA_MODE==ALPHA_BLEND
		
				punpcklbw mm7,mm7			;unpack
	#if SWIZZLE
				mov		ecx,eax
	#else
				mov		ecx,eax
	#endif
				punpcklbw mm6,mm6			;unpack
	#if SWIZZLE
				add		eax,DWORD PTR [ud]
	#else
				mov		edx,ebx
	#endif

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				psrlw	mm7,8-1
	#if SWIZZLE
				or		ecx,ebx
	#else
				and		ecx,DWORD PTR [umask]
	#endif
				psrlw	mm6,8-1
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
	#else
				and		edx,DWORD PTR [vmask]
	#endif
#else
				psrlw	mm7,8-2
	#if SWIZZLE
				or		ecx,ebx
	#else
				and		ecx,DWORD PTR [umask]
	#endif
				psrlw	mm6,8-2
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
	#else
				and		edx,DWORD PTR [vmask]
	#endif
#endif
				pmulhw	mm7,mm0				;colour modulate
	#if SWIZZLE
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
	#endif
				pmulhw	mm6,mm0				;colour modulate
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		ecx,edx
				add		eax,DWORD PTR [ud]
	#endif

				// Extract and replicate the alpha parts
				// Texel 0
				movq	mm2,mm7
	#if SWIZZLE
				mov		edx,eax
	#else
				shr		ecx,16-2
				add		ebx,DWORD PTR [vd]
	#endif
				psrlq	mm2,8+1				;half required value (creates a sign bit)
	#if SWIZZLE
				add		eax,DWORD PTR [ud]
	#else
				push	ecx
				mov		edx,ebx
	#endif
				pand	mm2,[alpha_mask]
	#if SWIZZLE
				or		edx,ebx
	#else
				mov		ecx,eax
				and		edx,DWORD PTR [vmask]
	#endif
				movq	mm5,mm2
	#if SWIZZLE
				add		ebx,DWORD PTR [vd]
	#else
				and		ecx,DWORD PTR [umask]
				add		eax,DWORD PTR [ud]
	#endif
				psrlq	mm5,16
	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				or		edx,ecx
				add		ebx,DWORD PTR [vd]
	#endif
				por		mm5,mm2
				psrlq	mm5,16
	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				shr		edx,16-2
				pop		ecx
	#endif
				por		mm5,mm2				;alpha channel replicated to r,g,b

				// Texel 0
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format
				
				movq	mm2,[edi]
				movq	mm1,mm2
				punpcklbw mm2,mm2			;unpack dest
				punpckhbw mm1,mm1			;unpack dest

				psrlw	mm2,8
				
				psubsw	mm7,mm2				;src-dest
				pmulhw	mm7,mm5				;alpha*(src-dest)



				paddsw	mm7,mm7				;*2
				paddsw	mm7,mm2				;dest0+alpha0*(src0-dest0)

				// Texel 1
				movq	mm2,mm6
				psrlq	mm2,8+1				;half required value (creates a sign bit)
				pand	mm2,[alpha_mask]
				movq	mm5,mm2
				psrlq	mm5,16
				por		mm5,mm2
				psrlq	mm5,16
				por		mm5,mm2				;alpha channel replicated to r,g,b

				// Texel 1
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format

				// Already unpacked above.
				psrlw	mm1,8
				
				psubsw	mm6,mm1				;src-dest
				pmulhw	mm6,mm5				;alpha*(src-dest)


				paddsw	mm6,mm6				;*2
				paddsw	mm6,mm1				;dest1+alpha1*(src1-dest1)

				// Pack.
				packuswb mm7,mm6			;re-pack

#else //#if ALPHA_MODE==ALPHA_BLEND

				punpcklbw mm7,mm7			;texel0
	#if SWIZZLE
				mov		ecx,eax
				add		eax,DWORD PTR [ud]
	#else
				mov		ecx,eax
				mov		edx,ebx
	#endif
				punpcklbw mm6,mm6			;texel1
	#if SWIZZLE
				or		ecx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				and		ecx,DWORD PTR [umask]
				and		edx,DWORD PTR [vmask]
	#endif
				psrlw	mm7,8-2
	#if SWIZZLE
				shr		ecx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				or		ecx,edx
				add		eax,DWORD PTR [ud]
	#endif
				psrlw	mm6,8-2
	#if SWIZZLE
				and		ecx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				shr		ecx,16-2
				add		ebx,DWORD PTR [vd]
	#endif
				pmulhw	mm7,mm0
	#if SWIZZLE
				mov		edx,eax
				add		eax,DWORD PTR [ud]
	#else
				push	ecx
				mov		edx,ebx
	#endif
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				pmulhw	mm6,mm1
#else
				pmulhw	mm6,mm0
#endif
	#if SWIZZLE
				or		edx,ebx
				add		ebx,DWORD PTR [vd]
	#else
				mov		ecx,eax
				and		edx,DWORD PTR [vmask]
	#endif

				packuswb mm7,mm6
	#if SWIZZLE
				shr		edx,16-2		;DWORD offset needed
				and		eax,0x5555ffff
	#else
				and		ecx,DWORD PTR [umask]
				add		eax,DWORD PTR [ud]
	#endif

#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				paddsw	mm0,mm2
#endif
#if ALPHA_MODE==ALPHA_ADD
				paddusb	mm7,[edi]
#elif ALPHA_MODE==ALPHA_BLEND
#elif ALPHA_MODE==ALPHA_TEST
				movq	mm5,mm7
				movq	mm6,[edi]
				pcmpgtb	mm7,[alpha_test_value]	;SIGNED compare!
				psrad	mm7,24				;propogate compare result down.
				pand	mm6,mm7
				pandn	mm7,mm5				;mask (mm7 = (mm5 AND ~mm7))
				por		mm7,mm6				;...and merge
#endif
	#if SWIZZLE
				and		edx,DWORD PTR [wrapmask]
				and		ebx,0xaaaaffff
	#else
				or		edx,ecx
				add		ebx,DWORD PTR [vd]
	#endif
#if (ALPHA_MODE!=ALPHA_ADD) && (ALPHA_MODE!=ALPHA_BLEND)
				paddsw	mm1,mm2
#endif


	#if SWIZZLE
	#else
				shr		edx,16-2
				pop		ecx
	#endif



#endif //#else //#if ALPHA_MODE==ALPHA_BLEND

				movq	[edi],mm7

#if SWIZZLE
//				mov		ecx,eax
//				add		eax,DWORD PTR [ud]
//				or		ecx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		ecx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		ecx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
//
//				mov		edx,eax
//				add		eax,DWORD PTR [ud]
//				or		edx,ebx
//				add		ebx,DWORD PTR [vd]
//				shr		edx,16-2		;DWORD offset needed
//				and		eax,0x5555ffff
//				and		edx,DWORD PTR [wrapmask]
//				and		ebx,0xaaaaffff
#else
//				mov		ecx,eax
//				mov		edx,ebx
//				and		ecx,DWORD PTR [umask]
//				and		edx,DWORD PTR [vmask]
//				or		ecx,edx
//				add		eax,DWORD PTR [ud]
//				shr		ecx,16-2
//				add		ebx,DWORD PTR [vd]
//				push	ecx
//
//				mov		edx,ebx
//				mov		ecx,eax
//				and		edx,DWORD PTR [vmask]
//				and		ecx,DWORD PTR [umask]
//				add		eax,DWORD PTR [ud]
//				or		edx,ecx
//				add		ebx,DWORD PTR [vd]
//				shr		edx,16-2
//				pop		ecx
#endif
				movd	mm7,[esi+ecx]
				movd	mm6,[esi+edx]

				add edi,8

				cmp edi,[last_dest]
				jne	hor_loop

main_loop_done:

				// Do we need to do the last pixel?
				;Is the last pixel odd?
				mov		ecx,[tempx2]
				test	ecx,0x1
				jz		finished
				

				// Do the last pixel

#if ALPHA_MODE==ALPHA_BLEND
		
				punpcklbw mm7,mm7			;unpack

#if ALPHA_BLEND_NOT_DOUBLE_LIGHTING
				psrlw	mm7,8-1
#else
				psrlw	mm7,8-2
#endif
				pmulhw	mm7,mm0				;colour modulate

				movq	mm6,mm7

				psrlq	mm6,8+1				;half required value (creates a sign bit)
				pand	mm6,[alpha_mask]
				movq	mm5,mm6
				psrlq	mm5,16
				por		mm5,mm6
				psrlq	mm5,16
				por		mm5,mm6				;alpha channel replicated to r,g,b
				// Now we need to do  alpha*src + (1-alpha)*dest
				// = dest + alpha*(src-dest)
				// which is quicker (only one multiply)
				// Precision is not a problem here - 8.8 format
				// Remember that mm2 is spare - no gouard shading on alpha-blenders.
				
				movd	mm6,[edi]
				//mov		eax,0x10806050
				//movd	mm6,eax

				punpcklbw mm6,mm6			;unpack dest
				psrlw	mm6,8
				
				psubsw	mm7,mm6				;src-dest
				pmulhw	mm7,mm5				;alpha*(src-dest)
				paddsw	mm7,mm7				;*2
				paddsw	mm7,mm6				;dest+alpha*(src-dest)
				
				packuswb mm7,mm7			;re-pack


				//paddd	mm3,[uinc]			;inc u
				//paddd	mm4,[vinc]			;inc v
				//pand	mm3,[umask]			;fix swizzle
				//pand	mm4,[vmask]			;fix swizzle


#else //#if ALPHA_MODE==ALPHA_BLEND

				punpcklbw mm7,mm7
				psrlw	mm7,8-2
				pmulhw	mm7,mm0
				packuswb mm7,mm7
#if ALPHA_MODE==ALPHA_ADD
				movd	mm6,[edi]
				paddusb	mm7,mm6
#elif ALPHA_MODE==ALPHA_BLEND
#elif ALPHA_MODE==ALPHA_TEST
				movq	mm5,mm7
				movq	mm6,[edi]
				pcmpgtb	mm7,[alpha_test_value]	;SIGNED compare!
				psrad	mm7,24				;propogate compare result down.
				pand	mm6,mm7
				pandn	mm7,mm5				;mask (mm7 = (mm5 AND ~mm7))
				por		mm7,mm6				;...and merge
#endif

#endif //#else //#if ALPHA_MODE==ALPHA_BLEND

				movd	[edi],mm7

finished:

				pop edx
				pop ecx
				pop ebx
				pop eax
				pop edi
				pop esi

				// Clean up the MMX state
				emms

				}
			}
		}
	}
}
#endif
