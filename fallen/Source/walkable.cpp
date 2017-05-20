#include "game.h"
#include "pap.h"
#include "walkable.h"
#include	"memory.h"
#include	"mav.h"

extern	void	highlight_rface(SLONG rface);
//
// code to do with walkable faces
//

//
// this whole module has been changed to treat walkable faces as 10% (more in terms of area) bigger for collision.
//


SLONG	clock(const SLONG dx,const SLONG dz,const SLONG dx1,const SLONG dz1)
{
	if((dx*dz1-dz*dx1)<=0)
		return(0);
	else
		return(1);
}

SLONG	point_in_quad(SLONG px,SLONG pz,SLONG x,SLONG y,SLONG z,SWORD face)
{
	SLONG	c0;
	SWORD	vx[4],vz[4];
	SLONG	mx=0,mz=0;

	ASSERT(face>=0);

	
	for(c0=0;c0<4;c0++)
	{
		vx[c0]=x+prim_points[prim_faces4[face].Points[c0]].X;
		vz[c0]=z+prim_points[prim_faces4[face].Points[c0]].Z;

		mx+=vx[c0];
		mz+=vz[c0];
	}

	
//	mx=0;
	mx>>=2;
//	mz/=mx;
	mz>>=2;
	for(c0=0;c0<4;c0++)
	{
		vx[c0]=( ( (vx[c0]-mx)*268)>>8 )+mx;
		vz[c0]=( ( (vz[c0]-mz)*268)>>8 )+mz;
	}


	if(clock(vx[1]-vx[0],vz[1]-vz[0],px-vx[0],pz-vz[0]))//x2-x1,z2-z1,px-x1,pz-z1))
	{
		if(clock(vx[3]-vx[1],vz[3]-vz[1],px-vx[1],pz-vz[1]))
		{
			if(clock(vx[2]-vx[3],vz[2]-vz[3],px-vx[3],pz-vz[3]))
			{
				if(clock(vx[0]-vx[2],vz[0]-vz[2],px-vx[2],pz-vz[2]))
				{
					return(1);
				}

			}

		}

	}
	return(0);
}

//
// returns true if on face
//   *height always trys to have the height

SLONG	gh_vx[4],gh_vy[4],gh_vz[4];//out of stack space (on PSX) so words
SLONG	get_height_on_face_quad64_at(SLONG rx, SLONG rz, SWORD face,SLONG *height)
{
//	SLONG 	ux,uy,uz,vx,vy,vz,wx,wy,wz;
	struct	PrimFace4 *this_face4;
	SLONG	ax,ay,az,bx,by,bz;

	SLONG	top, bot;
	SLONG	alpha, beta;
	SLONG	x,y,z;

	SLONG   mx=0,my=0,mz=0; 
	UWORD	c0;
	SLONG	on_face=1;
	ASSERT(face>=0);

	this_face4=&prim_faces4[face];

	for(c0=0;c0<4;c0++)
	{
		gh_vx[c0] = prim_points[this_face4->Points[c0]].X;
		gh_vy[c0] = prim_points[this_face4->Points[c0]].Y;
		gh_vz[c0] = prim_points[this_face4->Points[c0]].Z;

		mx+=gh_vx[c0];
//		my+=gh_vy[c0];
		mz+=gh_vz[c0];
		
	}

	if(gh_vy[0]==gh_vy[1] && gh_vy[1]==gh_vy[2] && gh_vy[2]==gh_vy[3])
	{
		*height=gh_vy[0];
		return(1);
	}

	mx>>=2;
//	my>>=2;
	mz>>=2;

	for(c0=0;c0<4;c0++)
	{
		gh_vx[c0]=( ( (gh_vx[c0]-mx)*268)>>8 )+mx;
//		gh_vy[c0]=( ( (gh_vy[c0]-my)*268)>>8 )+my;
		gh_vz[c0]=( ( (gh_vz[c0]-mz)*268)>>8 )+mz;
	}


/*
	ux=	obj_x+prim_points[this_face4->Points[0]].X;
	uy=	obj_y+prim_points[this_face4->Points[0]].Y;
	uz=	obj_z+prim_points[this_face4->Points[0]].Z;

	vx=	obj_x+prim_points[this_face4->Points[1]].X;
	vy=	obj_y+prim_points[this_face4->Points[1]].Y;
	vz=	obj_z+prim_points[this_face4->Points[1]].Z;
 
	wx=	obj_x+prim_points[this_face4->Points[2]].X;
	wy=	obj_y+prim_points[this_face4->Points[2]].Y;
	wz=	obj_z+prim_points[this_face4->Points[2]].Z;

	if(uy==vy && vy==wy)
		return(uy);
*/

	
	ax = (gh_vx[1] - gh_vx[0]) << 8;
	ay = (gh_vy[1] - gh_vy[0]) << 8;
	az = (gh_vz[1] - gh_vz[0]) << 8;
				  
	bx = (gh_vx[2] - gh_vx[0]) << 8;
	by = (gh_vy[2] - gh_vy[0]) << 8;
	bz = (gh_vz[2] - gh_vz[0]) << 8;

	x  = (rx<<8) - (gh_vx[0] << 8);
	z  = (rz<<8) - (gh_vz[0] << 8);

	//printf("face =%d a=(%d,%d,%d) b =(%d,%d,%d) xz=(%d,%d)\n",face,ax,ay,az,bx,by,bz,x,z);

	// Work out alpha and beta such that x = alpha*ax + beta*bx and y = alhpa*ay + beta*by
	
	// First alpha...

	top   = MUL64(x,  bz) - MUL64(z,  bx);
	bot   = MUL64(bz, ax) - MUL64(bx, az);

	if (bot == 0) {bot = 0x8000;}

	alpha = DIV64(top, bot);

	// Now beta...

	top   = MUL64(z,  ax) - MUL64(x,  az);
	if(bot<3)
	{
		*height=gh_vy[0];

		return(1);
	}
	beta  = DIV64(top, bot);


	if(alpha<0)
	{
		alpha=0;
		on_face=0;
	}
	if(beta<0)
	{
		beta=0;
		on_face=0;
	}
	if(alpha>0x10000)
	{
		alpha=0x10000;
		on_face=0;
	}
	if(beta>0x10000)
	{
		beta=0x10000;
		on_face=0;
	}

/*
	if (alpha < 0 || alpha > 0x10000 || beta < 0 || beta > 0x10000) 
	{
		LogText(" get height on QUAD NOT %d alpha %x beta %x \n",face,alpha,beta);
		return 1000000;
	}
*/
//	else
	if (alpha+beta>0x10000)
	{
		// 0   1	      3	   2
		//
		// 2			  1	   0
		//       3
		//
		// other triangular half of quad
		//

		ax = (gh_vx[2] - gh_vx[3]) << 8;
		ay = (gh_vy[2] - gh_vy[3]) << 8;
		az = (gh_vz[2] - gh_vz[3]) << 8;
					  
		bx = (gh_vx[1] - gh_vx[3]) << 8;
		by = (gh_vy[1] - gh_vy[3]) << 8;
		bz = (gh_vz[1] - gh_vz[3]) << 8;

		x  = (rx<<8) - (gh_vx[3] << 8);
		z  = (rz<<8) - (gh_vz[3] << 8);

		//printf("face =%d a=(%d,%d,%d) b =(%d,%d,%d) xz=(%d,%d)\n",face,ax,ay,az,bx,by,bz,x,z);

		// Work out alpha and beta such that x = alpha*ax + beta*bx and y = alhpa*ay + beta*by
		
		// First alpha...

		top   = MUL64(x,  bz) - MUL64(z,  bx);
		bot   = MUL64(bz, ax) - MUL64(bx, az);

		if (bot == 0) {bot = 0x8000;}

		alpha = DIV64(top, bot);

		// Now beta...

		top   = MUL64(z,  ax) - MUL64(x,  az);
		if(bot<3)
		{
			*height=gh_vy[3];
			return(on_face);
		}
		beta  = DIV64(top, bot);
/*
		if (alpha < 0 || alpha > 0x10000 || beta < 0 || beta > 0x10000) 
		{

		}
		else
*/
		if(alpha<0)
		{
			alpha=0;
			on_face=0;
		}
		if(beta<0)
		{
			beta=0;
			on_face=0;
		}
		if(alpha>0x10000)
		{
			alpha=0x10000;
			on_face=0;
		}
		if(beta>0x10000)
		{
			beta=0x10000;
			on_face=0;
		}
		if (alpha+beta>0x10000)
		{
			// This is benign - happens very very occasionally - don't worry about it.
			ASSERT(0);
			*height=gh_vy[1];
			return(0);
		}
		else
		{
			y     = gh_vy[3] << 8;
			y    += MUL64(alpha, ay);
			y    += MUL64(beta,  by);
			*height= y >> 8;
			return(on_face);
		}


	}
	else
	{
//		LogText(" get height on face=%d alpha %x beta %x  uy %d vy %d wy %d\n",y>>8,face,alpha,beta),uy,vy,wy;
		y     = gh_vy[0] << 8;
		y    += MUL64(alpha, ay);
		y    += MUL64(beta,  by);
		*height= y >> 8;
		return(on_face);
	}
}




//
// returns 0 or 1 (on face false/true) new_y is alt on face
//
/*
SLONG	calc_height_on_face(SLONG x,SLONG z,SLONG face,SLONG *new_y)
{

	if (face > 0)
	{
			return(get_height_on_face_quad64_at(x,z,face,new_y));
	}
	ASSERT(0);
//	return(1000000);
//	return(-100);
}
*/


SLONG	is_thing_on_this_quad(SLONG x,SLONG z,SLONG face)
{
	if(face<0)
	{
		struct	RoofFace4 *rf;
//		highlight_rface(-face);

		if(IS_ROOF_HIDDEN_FACE(face))
		{
			if((x>>8)==ROOF_HIDDEN_X(face) && (z>>8)==ROOF_HIDDEN_Z(face))
				return(1);
			else
				return(0);

		}
		rf=&roof_faces4[-face];
		x>>=8;
		z>>=8;
		if(x==(rf->RX&127) && z==(rf->RZ&127))
			return(1);
		else
			return(0);
		
	}
	else
	{

		if(point_in_quad(x,z,0,0,0,face))
		{
			return(1);
		}
		else
		{
			return(0);
		}
	}
}

SLONG calc_height_on_rface(SLONG x, SLONG z,SWORD	face,SLONG *ret_y)
{
	SLONG h0;
	SLONG h1;
	SLONG h2;
	SLONG h3;

	SLONG xfrac;
	SLONG zfrac;

	SLONG answer;

	struct	RoofFace4 *rf;

	ASSERT(face>0);

	if(IS_ROOF_HIDDEN_FACE(-face))
	{
		if(ROOF_HIDDEN_X(-face)!=(x>>8) || ROOF_HIDDEN_Z(-face)!=(z>>8))
			return(0);

		if(PAP_hi[x>>8][z>>8].Flags&PAP_FLAG_ROOF_EXISTS)
		{
			*ret_y = MAVHEIGHT(x>>8,z>>8)<<6;

			return(ROOF_HIDDEN_GET_FACE(x>>8,z>>8));
		}
		else
			return(0);

	}

//	highlight_rface(face);

	rf=&roof_faces4[face];

	if(x>>8!=(rf->RX&127) ||z>>8!=(rf->RZ&127))
		return(0);


	h0 = rf->Y;
	if(!(rf->RZ&128))
	{
		*ret_y=h0;
		return(face);
	}

	h1 = h0+(rf->DY[2]<<ROOF_SHIFT);
	h2 = h0+(rf->DY[0]<<ROOF_SHIFT);
	h3 = h0+(rf->DY[1]<<ROOF_SHIFT);

	{
		if(rf->RX&1<<7)
		{
			xfrac = x & 0xff;
			zfrac = z & 0xff;

			if (xfrac + (256-zfrac) < 0x100)
			{
				answer  =  h1;
				answer += (h3 - h1) * xfrac >> 8;
				answer += (h0 - h1) * (256-zfrac) >> 8;
			}
			else
			{
				answer  =  h2;
				answer += (h0 - h2) * (0x100 - xfrac) >> 8;
				answer += (h3 - h2) * (zfrac) >> 8;
			}
		}
		else
		{
			xfrac = x & 0xff;
			zfrac = z & 0xff;

			if (xfrac + zfrac < 0x100)
			{
				answer  =  h0;
				answer += (h2 - h0) * xfrac >> 8;
				answer += (h1 - h0) * zfrac >> 8;
			}
			else
			{
				answer  =  h3;
				answer += (h1 - h3) * (0x100 - xfrac) >> 8;
				answer += (h2 - h3) * (0x100 - zfrac) >> 8;
			}
		}
	}

	*ret_y=answer;
	return(face);
}


//
// Finds a face to be stood on (checks height is not out of range)
//

SLONG find_face_for_this_pos(
			SLONG  x,
			SLONG  y,
			SLONG  z,
			SLONG *ret_y,
			SLONG  ignore_building,
			UBYTE	ignore_height_flag)
{
	UBYTE mx;
	UBYTE mz;
	SWORD dy;
	SLONG facey;
	SWORD index;
	SWORD groundy;
	SWORD best_face  = NULL;
	SWORD best_dy    = 0x7fff;
	SWORD best_facey = 0;

	SWORD mx1 = x - 0x200 >> PAP_SHIFT_LO;
	SWORD mz1 = z - 0x200 >> PAP_SHIFT_LO;

	SWORD mx2 = x + 0x200 >> PAP_SHIFT_LO;
	SWORD mz2 = z + 0x200 >> PAP_SHIFT_LO;

	SATURATE(mx1, 0, PAP_SIZE_LO - 1);
	SATURATE(mz1, 0, PAP_SIZE_LO - 1);

	SATURATE(mx2, 0, PAP_SIZE_LO - 1);
	SATURATE(mz2, 0, PAP_SIZE_LO - 1);

	if(PAP_hi[x>>8][z>>8].Flags&PAP_FLAG_ROOF_EXISTS)
	{
		best_face=ROOF_HIDDEN_GET_FACE(x>>8,z>>8);

		if(ignore_height_flag==FIND_ANYFACE)
		{
			*ret_y = MAVHEIGHT(x>>8,z>>8)<<6;
			return(best_face);
		}
		best_facey=MAVHEIGHT(x>>8,z>>8)<<6;
		best_dy = best_facey - y;
		if(best_dy<30 && ignore_height_flag==FIND_FACE_NEAR_BELOW)
		{
			*ret_y = best_facey;

			return best_face;
		}
		if (best_dy < 0xa0)
		{
			best_face  = best_face;
			best_dy    = abs(best_dy);
		}
	}

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		index = PAP_2LO(mx,mz).Walkable;
		
		 while(index)
		 {
//			ASSERT(index >= 0);
//			ASSERT(WITHIN(index, 1, next_prim_face4 - 1));

			if (is_thing_on_this_quad(x,z, index))
			{
				//
				// We've found a face to stand on. But at what height?
				// we are on this face so use clipped alpha and beta in calc height on face
				//

				if(index<0)
				{
					calc_height_on_rface(x,z, -index,&facey);

				}
				else
				{
					calc_height_on_face(x,z, index,&facey);
				}

				dy = facey - y;

				if(ignore_height_flag==FIND_ANYFACE)
				{
					*ret_y = facey;

					return index;
				}

				if(dy<30 && ignore_height_flag==FIND_FACE_NEAR_BELOW)
				{
					*ret_y = facey;

					return index;
				}

				if (dy <= 0xa0)
				{
					//
					// This is a candidate face.
					//

					if (abs(dy) < best_dy)
					{
						best_dy    = abs(dy);
						best_face  = index;
						best_facey = facey;
					}
				}
				else
				{
					//
					// Too much difference in y. Ignore this face.
					//
				}
			}

			if(index<0)
			{
				index = roof_faces4[-index].Next;
			}
			else
			{
				index = prim_faces4[index].WALKABLE;
			}
		 }
	}

	if (best_face == NULL)
	{
		//
		// Could not find a face to stand on. How about the ground?
		//

		if(PAP_2HI(x>>PAP_SHIFT_HI,z>>PAP_SHIFT_HI).Flags & PAP_FLAG_HIDDEN)
		{
			return(0);
		}

		groundy = PAP_calc_height_at(x,z); //+5
		*ret_y  = groundy;

		dy = y - groundy;

		if (abs(dy) < 0x50)
		{
			return GRAB_FLOOR; // step onto floor
		}
	}
	else
	{
		*ret_y = best_facey;

		return best_face;
	}

	return NULL;

}

SLONG find_height_for_this_pos(	SLONG  x,SLONG  z,	SLONG *ret_face)
{
	UBYTE mx;
	UBYTE mz;
	SLONG dy;
	SLONG facey;
	SWORD index;
	SLONG groundy;

	SWORD mx1 = x - 0x200 >> PAP_SHIFT_LO;
	SWORD mz1 = z - 0x200 >> PAP_SHIFT_LO;

	SWORD mx2 = x + 0x200 >> PAP_SHIFT_LO;
	SWORD mz2 = z + 0x200 >> PAP_SHIFT_LO;

	SATURATE(mx1, 0, PAP_SIZE_LO - 1);
	SATURATE(mz1, 0, PAP_SIZE_LO - 1);

	SATURATE(mx2, 0, PAP_SIZE_LO - 1);
	SATURATE(mz2, 0, PAP_SIZE_LO - 1);

	if(PAP_hi[x>>8][z>>8].Flags&PAP_FLAG_ROOF_EXISTS)
	{
		*ret_face=ROOF_HIDDEN_GET_FACE(x>>8,z>>8);
		return(MAVHEIGHT(x>>8,z>>8)<<6);
	}

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		index = PAP_2LO(mx,mz).Walkable;
		
		 while(index)
		 {
//			ASSERT(index >= 0);
//			ASSERT(WITHIN(index, 1, next_prim_face4 - 1));

			if (is_thing_on_this_quad(x,z, index))
			{
				//
				// We've found a face to stand on. But at what height?
				//

				//
				// use result no matter what as we are on the face
				//
				if(index<0)
				{
					calc_height_on_rface(x,z, -index,&facey);

				}
				else
				{
					calc_height_on_face(x,z, index,&facey);
				}
				{

						*ret_face = index;

						return facey;
				}
			}

			if(index<0)
			{
				index = roof_faces4[-index].Next;
			}
			else
			{
				index = prim_faces4[index].WALKABLE;
			}
		 }
	}

	//
	// How about the ground?
	//

	if(PAP_2HI(x>>PAP_SHIFT_HI,z>>PAP_SHIFT_HI).Flags & PAP_FLAG_HIDDEN)
		return(0);

	groundy = PAP_calc_height_at(x,z); //+5

	*ret_face = 0;
	return groundy; // step onto floor

}

SLONG	RFACE_on_slope(SLONG face,SLONG x,SLONG z,SLONG *angle)
{
	SLONG h0;
	SLONG h1;
	SLONG h2;
	SLONG h3;

	SLONG xfrac;
	SLONG zfrac;


	struct	RoofFace4 *rf;

	ASSERT(face>0);


	if(IS_ROOF_HIDDEN_FACE(-face))
		return(0);

	rf=&roof_faces4[face];
	if(!(rf->RZ&128))
	{
		*angle=0;
		return(0);
	}

	if(x>>8!=(rf->RX&127) ||z>>8!=(rf->RZ&127))
		return(0);



	h0 = rf->Y;
	h1 = h0+(rf->DY[2]<<ROOF_SHIFT);
	h2 = h0+(rf->DY[0]<<ROOF_SHIFT);
	h3 = h0+(rf->DY[1]<<ROOF_SHIFT);


	if (h0 == h1 && h1 == h2 && h2 == h3)
	{
		//
		// No need to do any interpolation.
		//
		return(0);

	}
	else
	{

		//  h0   h2
		//
		//	h1   h3

		
		h0 <<= PAP_ALT_SHIFT;
		h1 <<= PAP_ALT_SHIFT;
		h2 <<= PAP_ALT_SHIFT;
		h3 <<= PAP_ALT_SHIFT;

		xfrac = x & 0xff;
		zfrac = z & 0xff;

		if(rf->RX&(1<<7))
		{
			if (xfrac + (256-zfrac) < 0x100)
			{
				SLONG	vx,vy,vz;
				SLONG	wx,wy,wz;
				SLONG	rx,ry,rz;
				SLONG	len;

				vx=256;
				vy=h3-h1;
				vz=0;

				wx=0;
				wy=h0-h1;
				wz=256;

				rx=(vy*wz); //-vz*wy;
				ry=65536; //vz*wx-vx*wz; dont care about this
				rz=(vx*wy); //-vy*wx;

				if(rx==0 && rz==0)
					return(0);

		   		*angle   = (Arctan(rx,rz))&2047;

				rx=abs(rx);
				rz=abs(rz);
				len=QDIST3(rx,ry,rz);

				ry=(ry<<8)/(len);

				return(abs(256-(len>>8)));

			}
			else
			{
				SLONG	vx,vy,vz;
				SLONG	wx,wy,wz;
				SLONG	rx,ry,rz;
				SLONG	len;

				vx=-256;
				vy=h0-h2;
				vz=0;

				wx=0;
				wy=h3-h2;
				wz=256;

				rx=(vy*wz); //-vz*wy;
				ry=65536; //vz*wx-vx*wz; dont care about this
				rz=(vx*wy); //-vy*wx;

				if(rx==0 && rz==0)
					return(0);

		   		*angle   = (Arctan(-rx,rz))&2047 ;

				rx=abs(rx);
				rz=abs(rz);
				len=QDIST3(rx,ry,rz);

				ry=(ry<<8)/(len);

				return(abs(256-(len>>8)));
			}
		}
		else
		{

			if (xfrac + zfrac < 0x100)
			{
				SLONG	vx,vy,vz;
				SLONG	wx,wy,wz;
				SLONG	rx,ry,rz;
				SLONG	len;

				vx=256;
				vy=h2-h0;
				vz=0;

				wx=0;
				wy=h1-h0;
				wz=-256;

				rx=(vy*wz); //-vz*wy;
				ry=65536; //vz*wx-vx*wz; dont care about this
				rz=(vx*wy); //-vy*wx;

				if(rx==0 && rz==0)
					return(0);

		   		*angle   = (Arctan(-rx,-rz))&2047;

				rx=abs(rx);
				rz=abs(rz);
				len=QDIST3(rx,ry,rz);

				ry=(ry<<8)/(len);

				return(abs(256-(len>>8)));

			}
			else
			{
				SLONG	vx,vy,vz;
				SLONG	wx,wy,wz;
				SLONG	rx,ry,rz;
				SLONG	len;

				vx=-256;
				vy=h1-h3;
				vz=0;

				wx=0;
				wy=h2-h3;
				wz=-256;

				rx=(vy*wz); //-vz*wy;
				ry=65536; //vz*wx-vx*wz; dont care about this
				rz=(vx*wy); //-vy*wx;

				if(rx==0 && rz==0)
					return(0);

		   		*angle   = (Arctan(rx,-rz))&2047 ;

				rx=abs(rx);
				rz=abs(rz);
				len=QDIST3(rx,ry,rz);

				ry=(ry<<8)/(len);

				return(abs(256-(len>>8)));
			}
		}
	}
}

#ifndef	PSX
void WALKABLE_remove_rface(UBYTE map_x, UBYTE map_z)
{
	SWORD  next;
	SWORD *prev;

	PAP_Lo    *pl;
	RoofFace4 *rf;
	
	pl = &PAP_2LO(map_x >> 2, map_z >> 2);

	if(PAP_hi[map_x][map_z].Flags&PAP_FLAG_ROOF_EXISTS)
	{
		PAP_hi[map_x][map_z].Flags&=~PAP_FLAG_ROOF_EXISTS;
		return;
	}
	


	prev = &pl->Walkable;
	next =  pl->Walkable;

	while(next)
	{
		if (next < 0)
		{
			rf = &roof_faces4[-next];

			if ((rf->RX&127) == map_x &&
				(rf->RZ&127) == map_z)
			{
				//
				// We have found the face to get rid of.  Take it out of the linked list
				// for this square.
				//

			   *prev = rf->Next;

				//
				// Instead of deleting this face proper, we mark it as no-draw for now.
				//

				rf->DrawFlags |= RFACE_FLAG_NODRAW;

				return;
			}

			prev = &rf->Next;
			next =  rf->Next;
		}
		else
		{
			//
			// This is a normal walkable face.
			//

			prev = &prim_faces4[next].WALKABLE;
			next =  prim_faces4[next].WALKABLE;
		}
	}

	//
	// Make this mapsquare be HIDDEN.
	//

	PAP_2HI(map_x,map_z).Flags |= PAP_FLAG_HIDDEN;
}
#endif

