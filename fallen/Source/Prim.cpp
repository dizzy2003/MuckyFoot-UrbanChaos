
//#include	"Editor.hpp"

#include	"game.h"
//#include	"engine.h"
//#include	"trig.h"
//#include	"math.h"
//#include	"prim.h"
//#include	"prim_draw.h"

#include	"fmatrix.h"
#include	"pap.h"
#include	"walkable.h"
#include	"supermap.h"
#include	"memory.h"
#ifndef PSX
#include	"matrix.h"
#else
#include	"fallen/psxeng/headers/matrix.h"
#endif


#define PRIM_MIN_BBOX 0x58

//
// Extra info for each prim that isn't saved...
//

PrimInfo *prim_info;//[256];//MAX_PRIM_OBJECTS];

#if !defined(PSX) && !defined(TARGET_DC)
struct	SVector			global_res[15560]; //max points per object?
SLONG	global_flags[15560];
UWORD	global_bright[15560];
#endif

extern	struct KeyFrameChunk 	*test_chunk;



#define	USED_POINT	(1<<0)
#define	USED_FACE3	(1<<1)
#define	USED_FACE4	(1<<2)

#ifndef PSX
CBYTE	prim_names[MAX_PRIM_OBJECTS][32];
#endif

#ifndef PSX
void	delete_prim_points_block(SLONG start,SLONG count)
{
	SLONG	c0;

	for(c0=start+count;c0<next_prim_point;c0++)
	{
		prim_points[c0-count]=prim_points[c0];
	}
	next_prim_point-=count;
}

void	delete_prim_faces3_block(SLONG start,SLONG count)
{
	SLONG	c0;

	for(c0=start+count;c0<next_prim_face3;c0++)
	{
		prim_faces3[c0-count]=prim_faces3[c0];
	}
	next_prim_face3-=count;
}

void	delete_prim_faces4_block(SLONG start,SLONG count)
{
	SLONG	c0;

	for(c0=start+count;c0<next_prim_face4;c0++)
	{
		prim_faces4[c0-count]=prim_faces4[c0];
	}
	next_prim_face4-=count;
}

//
// adjusts all the point references in the face structures
// because we have deleted a block of points
//
void	fix_faces_for_del_points(SLONG start,SLONG count)
{
	SLONG	c0,c1;
	for(c0=1;c0<next_prim_face3;c0++)
	{
		for(c1=0;c1<3;c1++)
		{
			if(prim_faces3[c0].Points[c1]>start)
			{
				prim_faces3[c0].Points[c1]-=count;
			}
		}
	}
	for(c0=1;c0<next_prim_face4;c0++)
	{
		for(c1=0;c1<4;c1++)
		{
			if(prim_faces4[c0].Points[c1]>start)
			{
				prim_faces4[c0].Points[c1]-=count;
			}
		}
	}
}

void	fix_objects_for_del_points(SLONG start,SLONG count)
{
	SLONG	c0,c1;
	for(c0=1;c0<next_prim_object;c0++)
	{
		if(prim_objects[c0].StartPoint>start)
		{
			prim_objects[c0].StartPoint-=count;
			prim_objects[c0].EndPoint-=count;
		}
	}
}

void	fix_objects_for_del_faces3(SLONG start,SLONG count)
{
	SLONG	c0,c1;
	for(c0=1;c0<next_prim_object;c0++)
	{
		if(prim_objects[c0].StartFace3>start)
		{
			prim_objects[c0].StartFace3-=count;
			prim_objects[c0].EndFace3-=count;
		}
	}
}

void	fix_objects_for_del_faces4(SLONG start,SLONG count)
{
	SLONG	c0,c1;
	for(c0=1;c0<next_prim_object;c0++)
	{
		if(prim_objects[c0].StartFace4>start)
		{
			prim_objects[c0].StartFace4-=count;
			prim_objects[c0].EndFace4-=count;
		}
	}
}

void	compress_prims(void)
{
#if !defined(PSX) && !defined(TARGET_DC)
	SLONG	c0,c1;
	SLONG	sp,ep,sf,ef;
	UBYTE	*pf;
	SLONG	count;

	struct	PrimObject	*p_obj;

	pf=(UBYTE*)&global_bright[0];

	memset(pf,0,15560*2);

	for(c0=1;c0<next_prim_object;c0++)
	{
		p_obj=&prim_objects[c0];

		for(c1=p_obj->StartPoint;c1<p_obj->EndPoint;c1++)
		{
			pf[c1]|=USED_POINT;
		}
		for(c1=p_obj->StartFace3;c1<p_obj->EndFace3;c1++)
		{
			pf[c1]|=USED_FACE3;
		}
		for(c1=p_obj->StartFace4;c1<p_obj->EndFace4;c1++)
		{
			pf[c1]|=USED_FACE4;
		}
	}

	for(c0=next_prim_point-1;c0>0;c0--)
	{
		if((pf[c0]&USED_POINT)==0)
		{
			count=1;
			for(c1=c0-1;c1>0;c1--)
			{
				if((pf[c1]&USED_POINT)==0)
				{
					count++;
				}
				else
					break;

			}
			c0=c1;
			delete_prim_points_block(c1+1,count);
			fix_faces_for_del_points(c1+1,count);
			fix_objects_for_del_points(c1+1,count);
		}
	}

 	for(c0=next_prim_face3-1;c0>0;c0--)
	{
		if((pf[c0]&USED_FACE3)==0)
		{
			count=1;
			for(c1=c0-1;c1>0;c1--)
			{
				if((pf[c1]&USED_FACE3)==0)
				{
					count++;
				}
				else
					break;

			}
			c0=c1;
			delete_prim_faces3_block(c1+1,count);
			fix_objects_for_del_faces3(c1+1,count);
		}
	}

 	for(c0=next_prim_face4-1;c0>0;c0--)
	{
		if((pf[c0]&USED_FACE4)==0)
		{
			count=1;
			for(c1=c0-1;c1>0;c1--)
			{
				if((pf[c1]&USED_FACE4)==0)
				{
					count++;
				}
				else
					break;

			}
			c0=c1;
			delete_prim_faces4_block(c1+1,count);
			fix_objects_for_del_faces4(c1+1,count);
		}
	}
#endif
}



void	clear_prims(void)
{
	SLONG c0;
	//
	// Mark all the prim objects as unloaded.
	//

	memset((UBYTE*)&prim_objects[0],       0, sizeof(PrimObject)      * MAX_PRIM_OBJECTS);
	memset((UBYTE*)&prim_multi_objects[0], 0, sizeof(PrimMultiObject) * MAX_PRIM_MOBJECTS);

	for(c0=0;c0<MAX_ANIM_CHUNKS;c0++)
		anim_chunk[c0].MultiObject[0]=0;

	next_prim_point        = 1;
	next_prim_face4        = 1;
	next_prim_face3        = 1;
	next_prim_object       = 266;	// Leave room for the prims.
	next_prim_multi_object = 1;
}

// Smooth lighting on a prim
SLONG	sum_shared_brightness_prim(SWORD shared_point,struct PrimObject *p_obj)
{
	SLONG	c0,point;
	SLONG	face;
	SLONG	bright=0;
	SLONG count=0;

	for(face=p_obj->StartFace3;face<p_obj->EndFace3;face++)
		for(point=0;point<3;point++)
		{
			if(prim_faces3[face].Points[point]==shared_point)
			{
				bright+=prim_faces3[face].Bright[point];
				count++;
			}
		}

	for(face=p_obj->StartFace4;face<p_obj->EndFace4;face++)
		for(point=0;point<4;point++)
		{
			if(prim_faces4[face].Points[point]==shared_point)
			{
				bright+=prim_faces4[face].Bright[point];
				count++;
			}
		}

	if(count)
		return(bright/count);
	else
		return(0);
}

void	set_shared_brightness_prim(SWORD shared_point,SWORD bright,struct PrimObject *p_obj)
{
	SLONG	c0,point;
	SLONG	face;

	for(face=p_obj->StartFace3;face<p_obj->EndFace3;face++)
		for(point=0;point<3;point++)
		{
			if(prim_faces3[face].Points[point]==shared_point)
			{
				prim_faces3[face].Bright[point]=bright;
			}
		}

	for(face=p_obj->StartFace4;face<p_obj->EndFace4;face++)
		for(point=0;point<4;point++)
		{
			if(prim_faces4[face].Points[point]==shared_point)
			{
				prim_faces4[face].Bright[point]=bright;
			}
		}
}




void	smooth_a_prim(SLONG prim)
{
	SLONG	face;
	SLONG	bright;
	struct	PrimObject	*p_obj;
	SLONG	point;

	p_obj    =	&prim_objects[prim];

	for(face=p_obj->StartFace4;face<p_obj->EndFace4;face++)
	{
		prim_faces4[face].FaceFlags|=FACE_FLAG_SMOOTH;
		for(point=0;point<4;point++)
		{
			bright=sum_shared_brightness_prim(prim_faces4[face].Points[point],p_obj);
			set_shared_brightness_prim(prim_faces4[face].Points[point],bright,p_obj);
		}

	}

	for(face=p_obj->StartFace3;face<p_obj->EndFace3;face++)
	{
		prim_faces3[face].FaceFlags|=FACE_FLAG_SMOOTH;
		for(point=0;point<3;point++)
		{
			bright=sum_shared_brightness_prim(prim_faces3[face].Points[point],p_obj);
			set_shared_brightness_prim(prim_faces3[face].Points[point],bright,p_obj);

		}

	}

}

/*
 ** copy prim to end
 *
 *  FILENAME: C:\darkcity\editor\source\Prim.cpp
 *
 *  PARAMETERS:
 *		prim ... Prim Number to copy to end of data
 *      direct   Flag for weather to apply user changes to data e.g scale rotate
 *
 */

SLONG	copy_prim_to_end(UWORD prim,UWORD direct,SWORD thing)
{
	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;

	p_obj    =&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	for(c0=ep-1;c0>=sp;c0--)
	{
		prim_points[end_prim_point]=prim_points[c0];
		//if(direct)
		//	apply_user_rotates(&prim_points[end_prim_point]);
		end_prim_point--;
	}
	for(c0=p_obj->EndFace4-1;c0>=p_obj->StartFace4;c0--)
	{
		if(c0==1075)
		{
//			LogText("copy 1074 to %d",end_prim_face4);
		}
		prim_faces4[end_prim_face4]=prim_faces4[c0];
		prim_faces4[end_prim_face4].Points[0]+=-sp+end_prim_point+1;
		prim_faces4[end_prim_face4].Points[1]+=-sp+end_prim_point+1;
		prim_faces4[end_prim_face4].Points[2]+=-sp+end_prim_point+1;
		prim_faces4[end_prim_face4].Points[3]+=-sp+end_prim_point+1;
		prim_faces4[end_prim_face4].ThingIndex=thing;

		prim_faces4[end_prim_face4].Bright[0]=0;
		prim_faces4[end_prim_face4].Bright[1]=0;
		prim_faces4[end_prim_face4].Bright[2]=0;
		prim_faces4[end_prim_face4].Bright[3]=0;

		end_prim_face4--;

	}
	for(c0=p_obj->EndFace3-1;c0>=p_obj->StartFace3;c0--)
	{
		prim_faces3[end_prim_face3]=prim_faces3[c0];
		prim_faces3[end_prim_face3].Points[0]+=-sp+end_prim_point+1;
		prim_faces3[end_prim_face3].Points[1]+=-sp+end_prim_point+1;
		prim_faces3[end_prim_face3].Points[2]+=-sp+end_prim_point+1;
		prim_faces3[end_prim_face3].ThingIndex=thing;

		prim_faces3[end_prim_face3].Bright[0]=0;
		prim_faces3[end_prim_face3].Bright[1]=0;
		prim_faces3[end_prim_face3].Bright[2]=0;


		end_prim_face3--;
	}
	prim_objects[end_prim_object].StartPoint=end_prim_point+1;
	prim_objects[end_prim_object].EndPoint=end_prim_point+1 +p_obj->EndPoint-p_obj->StartPoint;

	prim_objects[end_prim_object].StartFace3=end_prim_face3+1;
	prim_objects[end_prim_object].EndFace3=end_prim_face3+1 +p_obj->EndFace3-p_obj->StartFace3;

	prim_objects[end_prim_object].StartFace4=end_prim_face4+1;
	prim_objects[end_prim_object].EndFace4=end_prim_face4+1 +p_obj->EndFace4-p_obj->StartFace4;

	--end_prim_object;
	return(end_prim_object+1);
}

void	delete_prim_points(SLONG start,SLONG end)
{
	SLONG	c0,offset;
	offset=end-start;

	for(c0=end;c0<next_prim_point;c0++)
	{
		prim_points[c0-offset]=prim_points[c0];
	}

	for(c0=1;c0<next_prim_face3;c0++)
	{
		if(prim_faces3[c0].Points[0]>=start)
			prim_faces3[c0].Points[0]-=offset;
		if(prim_faces3[c0].Points[1]>=start)
			prim_faces3[c0].Points[1]-=offset;
		if(prim_faces3[c0].Points[2]>=start)
			prim_faces3[c0].Points[2]-=offset;

	}
	for(c0=1;c0<next_prim_face4;c0++)
	{
		if(prim_faces4[c0].Points[0]>=start)
			prim_faces4[c0].Points[0]-=offset;
		if(prim_faces4[c0].Points[1]>=start)
			prim_faces4[c0].Points[1]-=offset;
		if(prim_faces4[c0].Points[2]>=start)
			prim_faces4[c0].Points[2]-=offset;
		if(prim_faces4[c0].Points[3]>=start)
			prim_faces4[c0].Points[3]-=offset;
	}
}
void	delete_prim_faces3(SLONG start,SLONG end)
{
	SLONG	c0,offset;
	offset=end-start;

	for(c0=end;c0<next_prim_point;c0++)
	{
		prim_faces3[c0-offset]=prim_faces3[c0];
	}

	for(c0=1;c0<next_prim_object;c0++)
	{
		if(prim_objects[c0].StartFace3>=start)
			prim_objects[c0].StartFace3-=offset;
		if(prim_objects[c0].EndFace3>=start)
			prim_objects[c0].EndFace3-=offset;
	}
}

void	delete_prim_faces4(SLONG start,SLONG end)
{
	SLONG	c0,offset;
	offset=end-start;

	for(c0=end;c0<next_prim_point;c0++)
	{
		prim_faces3[c0-offset]=prim_faces3[c0];
	}

	for(c0=1;c0<next_prim_object;c0++)
	{
		if(prim_objects[c0].StartFace4>=start)
			prim_objects[c0].StartFace4-=offset;
		if(prim_objects[c0].EndFace4>=start)
			prim_objects[c0].EndFace4-=offset;
	}
}

void	delete_prim_objects(SLONG start,SLONG end)
{
	SLONG	c0,offset;
	offset=end-start;

	for(c0=end;c0<next_prim_point;c0++)
	{
		prim_objects[c0-offset]=prim_objects[c0];
	}
}

void	delete_last_prim(void)
{
	next_prim_point=prim_objects[next_prim_object-1].StartPoint;
	next_prim_face3=prim_objects[next_prim_object-1].StartFace3;
	next_prim_face4=prim_objects[next_prim_object-1].StartFace4;
	next_prim_object--;
}


void	delete_a_prim(UWORD prim)
{
	SLONG	c0;

	MSG_add("ERROR: c0 is undefined! Should it be prim?");

	c0 = prim;

	delete_prim_points(prim_objects[c0].StartPoint,prim_objects[c0].EndPoint);
	delete_prim_faces3(prim_objects[c0].StartFace3,prim_objects[c0].EndFace3);
	delete_prim_faces4(prim_objects[c0].StartFace4,prim_objects[c0].EndFace4);
	delete_prim_objects(prim,prim+1);
}

#endif


//dist= max(x,y,z) + other>>2 +other>>2
// 13% max error
//#define	QDIST3(x,y,z)	(x>y ? (x>z ? x+(y>>2)+(z>>2) : z+(x>>2)+(y>>2)) : (y>z ? (y+(x>>2)+(z>>2) : z+(x>>2)+(y>>2) ))


//void	apply_light_to_map(SLONG x,SLONG y,SLONG z,SWORD bright)
/*
UWORD	calc_lights(SLONG x,SLONG y,SLONG z,struct SVector *p_vect)
{
	SLONG	dx,dy,dz,dist;
	SLONG	lx,ly,lz;
	ULONG	c0;
	SLONG	total=0;

	lx=p_vect->X+x;
	ly=p_vect->Y+y;
	lz=p_vect->Z+z;

	for(c0=1;c0<next_d_light;c0++)
	{

		dx=abs(lx-d_lights[c0].X);
		dy=abs(ly-d_lights[c0].Y);
		dz=abs(lz-d_lights[c0].Z);


//		dist=QDIST3(dx,dy,dz);
		dist=dx*dx+dy*dy+dz*dz;

		if(dist==0)
			dist=1;
		if(dist<(256<<11))
			total+=(d_lights[c0].Intensity<<11)/dist;
	}
	return(total);
}
*/

#ifndef	PSX
void	calc_normal(SWORD	face,struct SVector *p_normal)
{
	SLONG	vx,vy,vz,wx,wy,wz;
	struct	PrimFace3 *this_face3;
	struct	PrimFace4 *this_face4;
	SLONG	nx,ny,nz;
	SLONG	length;
	struct	PrimPoint	*p_op0,*p_op1,*p_op2;
	/*
	if(face==-9823)
	{
		length=1;
	}
	*/

	if(face<0) //tri's
	{
		this_face3=&prim_faces3[-face];
		p_op0=&prim_points[this_face3->Points[0]];
		p_op1=&prim_points[this_face3->Points[1]];
		p_op2=&prim_points[this_face3->Points[2]];
	}
	else
	{
		this_face4=&prim_faces4[face];
		p_op0=&prim_points[this_face4->Points[0]];
		p_op1=&prim_points[this_face4->Points[1]];
		p_op2=&prim_points[this_face4->Points[3]];
	}

	vx= -p_op0->X + p_op1->X;
	vy= -p_op0->Y + p_op1->Y;   //vector from point 0 to point 1
	vz= -p_op0->Z + p_op1->Z;

	wx=p_op2->X - p_op1->X;   //vector from point 1 to point 2
	wy=p_op2->Y - p_op1->Y;
	wz=p_op2->Z - p_op1->Z;

	if((vx==0&&vy==0&&vz==0)||(wx==0&&wy==0&&wz==0))
	{
		p_normal->X=0;
		p_normal->Y=255;  //store result
		p_normal->Z=0;
		return;
	}

	length=vx*vx+vy*vy+vz*vz;
	length=Root(length);
	if(length==0)
		length=1;     //bodge around divide by zero
	vx=(vx<<8)/length;
	vy=(vy<<8)/length;  //normalise vect V ( make length equal 1<<8)
	vz=(vz<<8)/length;

	length=Root(wx*wx+wy*wy+wz*wz);
	if(length==0)
		length=1;  //bodge around divide by zero

	wx=(wx<<8)/length;
	wy=(wy<<8)/length; //same to vect W
	wz=(wz<<8)/length;

	nx=((vy)*(wz))-(vz*wy)>>8;
	ny=((vz)*(wx))-(vx*wz)>>8;  //perform cross product on vect V & W
	nz=((vx)*(wy))-(vy*wx)>>8;

	length=Root((nx*nx+ny*ny+nz*nz));
	if(length==0)
		length=1;
	nx=(nx<<8)/length;
	ny=(ny<<8)/length;   //normalise result  //pos opt this out
	nz=(nz<<8)/length;
	if(nx==0&&ny==0&&nz==0)
		ny=255;
	p_normal->X=-nx;
	p_normal->Y=-ny;  //store result
	p_normal->Z=-nz;

}

void	quick_normal(SWORD	face,SLONG *nx,SLONG *ny,SLONG *nz)
{
	SLONG	vx,vy,vz,wx,wy,wz;
	struct	PrimFace3 *this_face3;
	struct	PrimFace4 *this_face4;
	SLONG	length;
	struct	PrimPoint	*p_op0,*p_op1,*p_op2;
	if(face==-9823)
	{
		length=1;
	}

	if(face<0) //tri's
	{
		this_face3=&prim_faces3[-face];
		p_op0=&prim_points[this_face3->Points[0]];
		p_op1=&prim_points[this_face3->Points[1]];
		p_op2=&prim_points[this_face3->Points[2]];
	}
	else
	{
		this_face4=&prim_faces4[face];
		p_op0=&prim_points[this_face4->Points[0]];
		p_op1=&prim_points[this_face4->Points[1]];
		p_op2=&prim_points[this_face4->Points[3]];
	}

	vx= -p_op0->X + p_op1->X;
	vy= -p_op0->Y + p_op1->Y;   //vector from point 0 to point 1
	vz= -p_op0->Z + p_op1->Z;

	wx=p_op2->X - p_op1->X;   //vector from point 1 to point 2
	wy=p_op2->Y - p_op1->Y;
	wz=p_op2->Z - p_op1->Z;


	*nx=((vy)*(wz))-(vz*wy);
	*ny=((vz)*(wx))-(vx*wz);  //perform cross product on vect V & W
	*nz=((vx)*(wy))-(vy*wx);

}

#define shadow_calc	0
#define	in_shadow(x,y,z,i,j,k)	0
#define	in_shadowo(x,y,z,i,j,k)	0

UWORD	apply_ambient_light_to_object(UWORD object,SLONG lnx,SLONG lny,SLONG lnz,UWORD intense)
{
//	SLONG	length,vx,vy,vz,wx,wy,wz;
	struct	MyMapElement*	me;
	struct	MyObject *mo;
	SLONG	count,offset=0,fred=0;
	struct	PrimFace3 *this_face;
	SLONG	nx,ny,nz;
	SLONG	tmp_shade;
	UWORD	no_faces;
	UWORD	start_face,current_face;
	SLONG	ratio,light,repeat=0;
	SLONG	object_y;
	UWORD	next=0;
	UWORD	no_faces4,start_face4;
	SLONG	ox,oy,oz;
//	,px,py,pz;
	struct	PrimFace4 *this_face4;
//	return(0);

	start_face4=prim_objects[object].StartFace4;
	no_faces4=prim_objects[object].EndFace4-prim_objects[object].StartFace4;

	start_face=prim_objects[object].StartFace3;
	no_faces=prim_objects[object].EndFace3-prim_objects[object].StartFace3;

	ox=0;
	oy=0;
	oz=0;

	for(current_face=0;current_face<no_faces;current_face++)
	{
		struct	SVector	normal;
		this_face=&prim_faces3[start_face+current_face];
		calc_normal(-(start_face+current_face),&normal);
		nx=normal.X;
		ny=normal.Y;
		nz=normal.Z;

		light=(nx*lnx+ny*lny+nz*lnz)>>8;
		light=(light*intense)>>8;
		if(light<intense>>3)
			light=intense>>3;

		if(light>=0)
		{
/*
			if(shadow_calc&1)
			{
				px=ox+prim_points[this_face->Points[0]].X;
				py=oy+prim_points[this_face->Points[0]].Y;
				pz=oz+prim_points[this_face->Points[0]].Z;
#ifdef	EDITOR
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face->Shade0=light>>1;
				else
					this_face->Shade0=light;
#endif

				px=ox+prim_points[this_face->Points[1]].X;
				py=oy+prim_points[this_face->Points[1]].Y;
				pz=oz+prim_points[this_face->Points[1]].Z;
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face->Shade1=light>>1;
				else
					this_face->Shade1=light;

				px=ox+prim_points[this_face->Points[2]].X;
				py=oy+prim_points[this_face->Points[2]].Y;
				pz=oz+prim_points[this_face->Points[2]].Z;
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face->Shade2=light>>1;
				else
					this_face->Shade2=light;
			}
			else
*/
			{
				this_face->Bright[0]=light;
				this_face->Bright[1]=light;
				this_face->Bright[2]=light;
			}
		}

//early_out:;
	}

	for(current_face=0;current_face<no_faces4;current_face++)
	{
		struct	SVector	normal;

		this_face4=&prim_faces4[start_face4+current_face];

		calc_normal((start_face4+current_face),&normal);
		nx=normal.X;
		ny=normal.Y;
		nz=normal.Z;
		//nx,ny,nz is unit vector of normal out of poly

		light=(nx*lnx+ny*lny+nz*lnz)>>8;
		light=(light*intense)>>8;
		if(light<intense>>3)
			light=intense>>3;

		if(light>=0)
		{
/*
			if(shadow_calc&1)
			{
				px=ox+prim_points[this_face4->Points[0]].X;
				py=oy+prim_points[this_face4->Points[0]].Y;
				pz=oz+prim_points[this_face4->Points[0]].Z;
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face4->Shade0=light>>1;
				else
					this_face4->Shade0=light;

				px=ox+prim_points[this_face4->Points[1]].X;
				py=oy+prim_points[this_face4->Points[1]].Y;
				pz=oz+prim_points[this_face4->Points[1]].Z;
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face4->Shade1=light>>1;
				else
					this_face4->Shade1=light;

				px=ox+prim_points[this_face4->Points[2]].X;
				py=oy+prim_points[this_face4->Points[2]].Y;
				pz=oz+prim_points[this_face4->Points[2]].Z;
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face4->Shade2=light>>1;
				else
					this_face4->Shade2=light;

				px=ox+prim_points[this_face4->Points[3]].X;
				py=oy+prim_points[this_face4->Points[3]].Y;
				pz=oz+prim_points[this_face4->Points[3]].Z;
				if(in_shadowo(px,py,pz,px-(lnx<<4),py-(lny<<4),pz-(lnz<<4)))
					this_face4->Shade3=light>>1;
				else
					this_face4->Shade3=light;


			}
			else
*/
			{
				this_face4->Bright[0]=light;
				this_face4->Bright[1]=light;
				this_face4->Bright[2]=light;
				this_face4->Bright[3]=light;
			}
		}
//early_out4:;
	}

	return	next;
	//	printf( " REPEAT %d , out of %d \n",repeat,no_faces);
}

#endif
#ifndef	PSX
void calc_prim_info()
{
	SLONG i;
	SLONG j;

	SLONG dist;
	SLONG below;
	SLONG num_points;

	PrimObject *obj;
	PrimInfo   *inf;
	PrimPoint  *pt;

	prim_objects[29].coltype=PRIM_COLLIDE_NONE;

	for (i = 1; i < 256; i++)
	{
		obj = &prim_objects[i];
		inf = &prim_info   [i];

		if (obj->StartPoint == NULL)
		{
			//
			// This object has not been loaded.
			//

			continue;
		}

		inf->minx   = +INFINITY;
		inf->miny   = +INFINITY;
		inf->minz   = +INFINITY;
		inf->maxx   = -INFINITY;
		inf->maxy   = -INFINITY;
		inf->maxz   = -INFINITY;
		inf->radius = -INFINITY;

		//
		// Workout the y of the bounding rectangle first.
		//

		for (j = obj->StartPoint; j < obj->EndPoint; j++)
		{
			pt = &prim_points[j];

			if (pt->Y < inf->miny) {inf->miny = pt->Y;}
			if (pt->Y > inf->maxy) {inf->maxy = pt->Y;}
		}

		//
		// When working out the bounding box of a prim, only
		// count points at the bottom of the prim.
		//

		below = inf->miny + (inf->maxy - inf->miny >> 3);

		for (j = obj->StartPoint; j < obj->EndPoint; j++)
		{
			pt = &prim_points[j];

			if (i == 41)
			{
				//
				// Special case for the 'step' prim!
				//

				//if (pt->Y >= inf->miny + 0x10)
				{
					if (pt->X < inf->minx) {inf->minx = pt->X;}
					if (pt->Z < inf->minz) {inf->minz = pt->Z;}

					if (pt->X > inf->maxx) {inf->maxx = pt->X;}
					if (pt->Z > inf->maxz) {inf->maxz = pt->Z;}
				}
			}
			else
			{
				if (pt->Y < below)
				{
					if (pt->X < inf->minx) {inf->minx = pt->X;}
					if (pt->Z < inf->minz) {inf->minz = pt->Z;}

					if (pt->X > inf->maxx) {inf->maxx = pt->X;}
					if (pt->Z > inf->maxz) {inf->maxz = pt->Z;}
				}
			}

			dist = pt->X * pt->X + pt->Y * pt->Y + pt->Z * pt->Z;
			dist = Root(dist);

			if (dist > inf->radius) {inf->radius = dist;}
		}

		if (i == PRIM_OBJ_WILDCATVAN_BODY)
		{
			if (inf->minx > -64)
			{
				//
				// Emergency fix for the wild cat van!
				//

				for (j = obj->StartPoint; j < obj->EndPoint; j++)
				{
					pt = &prim_points[j];

					pt->X -= 128;
					pt->Z += 256;
				}

				inf->minx -= 128;
				inf->minz += 256;

				inf->maxx -= 128;
				inf->maxz += 256;
			}
		}

		//
		// The minimum dimension for a bounding box.
		//


		//
		// Make sure none of the bounding boxes of the objects are
		// too small.
		//

		dist = inf->maxx - inf->minx;

		if (dist < PRIM_MIN_BBOX)
		{
			inf->maxx += PRIM_MIN_BBOX - dist >> 1;
			inf->minx -= PRIM_MIN_BBOX - dist >> 1;
		}

		dist = inf->maxz - inf->minz;

		if (dist < PRIM_MIN_BBOX)
		{
			inf->maxz += PRIM_MIN_BBOX - dist >> 1;
			inf->minz -= PRIM_MIN_BBOX - dist >> 1;
		}

		if (i == 181 || i == 182)
		{
			//
			// This is the gate at the entrance to the estate.
			// Make it bigger so you can't run through it!
			//

			inf->minx -= 0x40;
			inf->minz -= 0x40;

			inf->maxx += 0x40;
			inf->maxz += 0x40;
		}

		//
		// Make the centre of gravity the origin.
		//

		inf->cogx = 0;
		inf->cogy = 0;
		inf->cogz = 0;

		//
		// Are any of the faces environment mapped?
		//

		obj->flag &= ~PRIM_FLAG_ENVMAPPED;

		for (j = obj->StartFace3; j < obj->EndFace3; j++)
		{
			if (prim_faces3[j].FaceFlags & FACE_FLAG_ENVMAP)
			{
				obj->flag |= PRIM_FLAG_ENVMAPPED;

				goto set_the_envmapped_flag;
			}
		}

		for (j = obj->StartFace4; j < obj->EndFace4; j++)
		{
			if (prim_faces4[j].FaceFlags & FACE_FLAG_ENVMAP)
			{
				obj->flag |= PRIM_FLAG_ENVMAPPED;

				goto set_the_envmapped_flag;
			}
		}

	  set_the_envmapped_flag:;

		//
		// Shall we include this prim in LOS calculations?
		//

		obj->damage &= ~PRIM_DAMAGE_NOLOS;

		if (obj->coltype == PRIM_COLLIDE_BOX)
		{
			if (inf->maxy >= inf->miny + 0xc0)
			{
				//
				// Tall enough ...
				//

				if (inf->maxx >= inf->minx + 0x80 &&
					inf->maxz >= inf->minz + 0x80)
				{
					//
					// ... and wide enough to hide behind.
					//

					obj->damage |= PRIM_DAMAGE_NOLOS;
				}
			}
		}
	}

	// calculate vehicle prim info for crumple zones
	VEH_init_vehinfo();
}
#endif

//
// The normals.
//

#ifndef	PSX
#define MAX_POINTS_PER_PRIM 5000
#else
#define MAX_POINTS_PER_PRIM 1
#endif

UBYTE each_point[MAX_POINTS_PER_PRIM];
#ifndef	PSX
// one day

void calc_prim_normals(void)
{
	SLONG   i;
	SLONG   j;
	SLONG   k;
	SLONG   dx;
	SLONG   dy;
	SLONG   dz;
	SLONG   dist;
	SLONG   num_points;
	SVector fnormal;
	SLONG   p_index;

	PrimObject *p_obj;
	PrimFace3  *p_f3;
	PrimFace4  *p_f4;
	PrimPoint  *p_pt;

	for (i = 1; i < next_prim_object; i++)
	{
		p_obj = &prim_objects[i];

		if (p_obj->StartPoint == NULL)
		{
			//
			// This object has not been loaded.
			//

			continue;
		}

		num_points = p_obj->EndPoint - p_obj->StartPoint;

		ASSERT(num_points <= MAX_POINTS_PER_PRIM);
		ASSERT(num_points >=0);

		//
		// Mark all the points as having zero faces using them.
		//

		memset(each_point, 0, sizeof(UBYTE) * num_points);

		//
		// Work out the normal for each point by going through
		// all the faces.
		//

		ASSERT(( -p_obj->StartFace3 +p_obj->EndFace3)<2000);
		for (j = p_obj->StartFace3; j < p_obj->EndFace3; j++)
		{
			p_f3 = &prim_faces3[j];

			//
			// What is the normal of this face?
			//

			calc_normal(-j, &fnormal);

			//
			// Use this normal to work out the normal of each point that
			// makes up the face.
			//

			for (k = 0; k < 3; k++)
			{
				p_index = p_f3->Points[k] - p_obj->StartPoint;

				ASSERT(WITHIN(p_index, 0, MAX_POINTS_PER_PRIM - 1));
				ASSERT(p_f3->Points[k]<MAX_PRIM_POINTS);

				if (each_point[p_index] == 0)
				{
					//
					// This is the only face that we know uses the point,
					// so make the normal of the point equal to the normal
					// of the face.
					//

					prim_normal[p_f3->Points[k]] = fnormal;

					each_point[p_index] = 1;
				}
				else
				{
					//
					// Average this faces' normal with the current normal.
					//

					prim_normal[p_f3->Points[k]].X *= each_point[p_index];
					prim_normal[p_f3->Points[k]].Y *= each_point[p_index];
					prim_normal[p_f3->Points[k]].Z *= each_point[p_index];

					prim_normal[p_f3->Points[k]].X += fnormal.X;
					prim_normal[p_f3->Points[k]].Y += fnormal.Y;
					prim_normal[p_f3->Points[k]].Z += fnormal.Z;

					each_point[p_index] += 1;

					prim_normal[p_f3->Points[k]].X /= each_point[p_index];
					prim_normal[p_f3->Points[k]].Y /= each_point[p_index];
					prim_normal[p_f3->Points[k]].Z /= each_point[p_index];

				}
			}
		}

		ASSERT(( -p_obj->StartFace4 +p_obj->EndFace4)<2000);
		for (j = p_obj->StartFace4; j < p_obj->EndFace4; j++)
		{
			p_f4 = &prim_faces4[j];

			//
			// What is the normal of this face?
			//

			calc_normal(j, &fnormal);

			//
			// Use this normal to work out the normal of each point that
			// makes up the face.
			//

			for (k = 0; k < 4; k++)
			{
				p_index = p_f4->Points[k] - p_obj->StartPoint;

				ASSERT(WITHIN(p_index, 0, MAX_POINTS_PER_PRIM - 1));
				ASSERT(p_f4->Points[k]<MAX_PRIM_POINTS);
				if (each_point[p_index] == 0)
				{
					//
					// This is the only face that we know uses the point,
					// so make the normal of the point equal to the normal
					// of the face.
					//

					prim_normal[p_f4->Points[k]] = fnormal;

					each_point[p_index] = 1;
				}
				else
				{
					//
					// Average this faces' normal with the current normal.
					//

					prim_normal[p_f4->Points[k]].X *= each_point[p_index];
					prim_normal[p_f4->Points[k]].Y *= each_point[p_index];
					prim_normal[p_f4->Points[k]].Z *= each_point[p_index];

					prim_normal[p_f4->Points[k]].X += fnormal.X;
					prim_normal[p_f4->Points[k]].Y += fnormal.Y;
					prim_normal[p_f4->Points[k]].Z += fnormal.Z;

					each_point[p_index] += 1;

					prim_normal[p_f4->Points[k]].X /= each_point[p_index];
					prim_normal[p_f4->Points[k]].Y /= each_point[p_index];
					prim_normal[p_f4->Points[k]].Z /= each_point[p_index];
				}
			}
		}


		//
		// Normalise the length of each normal to be 256.
		//

		SLONG old_nx;
		SLONG old_ny;
		SLONG old_nz;

		for (j = p_obj->StartPoint; j < p_obj->EndPoint; j++)
		{
			ASSERT(j<MAX_PRIM_POINTS);
			old_nx = prim_normal[j].X;
			old_ny = prim_normal[j].Y;
			old_nz = prim_normal[j].Z;

			dx = abs(prim_normal[j].X);
			dy = abs(prim_normal[j].Y);
			dz = abs(prim_normal[j].Z);

			dist  = dx*dx + dy*dy + dz*dz;
			dist  = Root(dist);
			dist += 2;

			prim_normal[j].X <<= 8;
			prim_normal[j].Y <<= 8;
			prim_normal[j].Z <<= 8;

			prim_normal[j].X /= -dist;
			prim_normal[j].Y /= -dist;
			prim_normal[j].Z /= -dist;

			if ((prim_normal[j].X * prim_normal[j].X +
				 prim_normal[j].Y * prim_normal[j].Y +
				 prim_normal[j].Z * prim_normal[j].Z) > (65536+400))
			{
				ASSERT(0);
			}
		}
	}
}
#endif

PrimInfo *get_prim_info(SLONG prim)
{
	ASSERT(WITHIN(prim, 1, 255));

	return &prim_info[prim];
}


#define	SLIDE_EDGE_HEIGHT	0x90
#ifndef	PSX
void calc_slide_edges_roof()
{
	SLONG	c0;
	SLONG	c1;
	struct	RoofFace4	*rf1,*rf2;
	SLONG	dx,dz;

	for(c0=1;c0<next_roof_face4;c0++)
	{
		roof_faces4[c0].DrawFlags|=0x78;
	}

	rf1=&roof_faces4[1];

	for(c0=1;c0<next_roof_face4;c0++)
	{
		SLONG	index;
		SLONG	x,z;
		//rf1->DrawFlags|=0x78;

		x=(rf1->RX&127)<<8;
		z=(rf1->RZ&127)<<8;

		//
		// check for fences above
		//
		if (does_fence_lie_along_line(x, z, x+256, z))
		{
			rf1->DrawFlags &= ~(RFACE_FLAG_SLIDE_EDGE_0);
		}
		if (does_fence_lie_along_line(x+256, z, x+256, z+256))
		{
			rf1->DrawFlags &= ~(RFACE_FLAG_SLIDE_EDGE_1);
		}
		if (does_fence_lie_along_line(x, z+256, x+256, z+256))
		{
			rf1->DrawFlags &= ~(RFACE_FLAG_SLIDE_EDGE_2);
		}
		if (does_fence_lie_along_line(x, z, x, z+256))
		{
			rf1->DrawFlags &= ~(RFACE_FLAG_SLIDE_EDGE_3);
		}

		//
		// check with floor alt
		//
		{
			SLONG	pap[4],roof[4];
			SLONG	d1,d2,c1;
			SLONG	mx,mz;
			mx=(rf1->RX&127)<<8;
			mz=(rf1->RZ&127)<<8;

			pap[0]=PAP_calc_height_at(mx,mz);
			pap[1]=PAP_calc_height_at(mx+256,mz);
			pap[2]=PAP_calc_height_at(mx+256,mz+256);
			pap[3]=PAP_calc_height_at(mx,mz+256);

			roof[0]=rf1->Y;
			roof[1]=rf1->Y+(rf1->DY[0]<<ROOF_SHIFT);
			roof[2]=rf1->Y+(rf1->DY[1]<<ROOF_SHIFT);
			roof[3]=rf1->Y+(rf1->DY[2]<<ROOF_SHIFT);

			for(c1=0;c1<4;c1++)
			{
				d1=roof[c1]-pap[c1];
				d2=roof[(c1+1)&3]-pap[(c1+1)&3];

				if(d1<SLIDE_EDGE_HEIGHT && d2<=SLIDE_EDGE_HEIGHT)
				{
					rf1->DrawFlags&=~(RFACE_FLAG_SLIDE_EDGE<<c1);
				}
			}
		}

		//   Y		 Y+dy[0]
		//
		//	 Y+dy[2] Y+dy[1]



		for(dx=-1;dx<=1;dx++)
		for(dz=-1;dz<=1;dz++)

		{
			SLONG	mx,mz;
			mx=((rf1->RX&127)>>2)+dx;
			mz=((rf1->RZ&127)>>2)+dz;

			if(mx>=0 && mz>=0 && mx<PAP_SIZE_LO && mz<PAP_SIZE_LO)
			{
				index = PAP_2LO(mx,mz).Walkable; //MAP[MAP_INDEX(mx,mz)].Walkable;
				while(index)
				{
					if(index<0)
					{
						SLONG	d1,d2;

						rf2=&roof_faces4[-index];
//						if(rf1!=rf2)
						{
							if((rf1->RX&127)==(rf2->RX&127))
							{
								if((rf1->RZ&127)==(rf2->RZ&127)+1)
								{
									SLONG	d1,d2;
									// rf2
									// rf1
									d1=(rf1->Y)-(rf2->Y+(rf2->DY[2]<<ROOF_SHIFT));
									d2=(rf1->Y+(rf1->DY[0]<<ROOF_SHIFT))-(rf2->Y+(rf2->DY[1]<<ROOF_SHIFT));
	//								d1=(rf1->Y+(rf1->DY[2]<<ROOF_SHIFT))-(rf2->Y);
	//								d2=(rf1->Y+(rf1->DY[1]<<ROOF_SHIFT))-(rf2->Y+(rf2->DY[0]<<ROOF_SHIFT));

									if(d1>=0 && d1<SLIDE_EDGE_HEIGHT && d2>=0&&d2<=SLIDE_EDGE_HEIGHT)
										rf1->DrawFlags&=~RFACE_FLAG_SLIDE_EDGE_0;
								}
								if((rf1->RZ&127)==(rf2->RZ&127)-1)
								{
									// rf1
									// rf2
									//
	//								d1=(rf1->Y)-(rf2->Y+(rf2->DY[2]<<ROOF_SHIFT));
	//								d2=(rf1->Y+(rf1->DY[0]<<ROOF_SHIFT))-(rf2->Y+(rf2->DY[1]<<ROOF_SHIFT));
									d1=(rf1->Y+(rf1->DY[2]<<ROOF_SHIFT))-(rf2->Y);
									d2=(rf1->Y+(rf1->DY[1]<<ROOF_SHIFT))-(rf2->Y+(rf2->DY[0]<<ROOF_SHIFT));

									if(d1>=0 && d1<SLIDE_EDGE_HEIGHT && d2>=0&&d2<=SLIDE_EDGE_HEIGHT)
										rf1->DrawFlags&=~RFACE_FLAG_SLIDE_EDGE_2;
								}
							}
							if((rf1->RZ&127)==(rf2->RZ&127))
							{
								if((rf1->RX&127)==(rf2->RX&127)+1)
								{
									//  rf2 rf1
									//
									d1=(rf1->Y)-(rf2->Y+(rf2->DY[0]<<ROOF_SHIFT));
									d2=(rf1->Y+(rf1->DY[2]<<ROOF_SHIFT))-(rf2->Y+(rf2->DY[1]<<ROOF_SHIFT));
									if(d1>=0 && d1<SLIDE_EDGE_HEIGHT && d2>=0&&d2<=SLIDE_EDGE_HEIGHT)
										rf1->DrawFlags&=~RFACE_FLAG_SLIDE_EDGE_3;
								}
								if((rf1->RX&127)==(rf2->RX&127)-1)
								{
									//  rf1 rf2
									//
									d1=(rf1->Y+(rf1->DY[0]<<ROOF_SHIFT))-(rf2->Y);
									d2=(rf1->Y+(rf1->DY[1]<<ROOF_SHIFT))-(rf2->Y+(rf2->DY[2]<<ROOF_SHIFT));
									if(d1>=0 && d1<SLIDE_EDGE_HEIGHT && d2>=0&&d2<=SLIDE_EDGE_HEIGHT)
										rf1->DrawFlags&=~RFACE_FLAG_SLIDE_EDGE_1;
								}
							}
						}
						index=rf2->Next;
					}
					else
					{
						index=prim_faces4[index].WALKABLE;
					}
				}
			}
		}
		rf1++;

	}
}

void calc_slide_edges()
{
	SLONG i;
	SLONG j;
	SLONG p;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG len;

	SLONG bx;
	SLONG by;
	SLONG bz;

	SLONG px;
	SLONG pz;

	SLONG x1;
	SLONG z1;
	SLONG x2;
	SLONG z2;

	SLONG mx;
	SLONG mz;

	SLONG ei;
	SLONG ej;

	SLONG ip1;
	SLONG ip2;

	SLONG jp1;
	SLONG jp2;

	SLONG ip1x;
	SLONG ip1y;
	SLONG ip1z;

	SLONG ip2x;
	SLONG ip2y;
	SLONG ip2z;

	SLONG jp1x;
	SLONG jp1y;
	SLONG jp1z;

	SLONG jp2x;
	SLONG jp2y;
	SLONG jp2z;

	SLONG near_height;
	SLONG pos_face;

	PrimFace4 *f;
	PrimFace4 *g;

	SLONG index;

	//
	// Set all the slide edge bits.
	//

	calc_slide_edges_roof();


	for (i = 1; i < next_prim_face4; i++)
	{
		f = &prim_faces4[i];

		if (f->FaceFlags & FACE_FLAG_WALKABLE)
		{
			f->FaceFlags |=
				FACE_FLAG_SLIDE_EDGE_0 |
				FACE_FLAG_SLIDE_EDGE_1 |
				FACE_FLAG_SLIDE_EDGE_2 |
				FACE_FLAG_SLIDE_EDGE_3;
		}
	}

	//
	// Compare each walkable face against all the others! If
	// two walkable faces share an edge then clear the slide
	// bits along their edges.
	//

	UBYTE point_order[4] = {0, 1, 3, 2};

	for (i = 1; i < next_prim_face4; i++)
	{
		f = &prim_faces4[i];

		if (f->FaceFlags & FACE_FLAG_WALKABLE)
		{
			if (f->FaceFlags & FACE_FLAG_WMOVE)
			{
				//
				// Don't check moveable walkable faces.
				//
			}
			else
			{
				//
				// Find the bounding box in which to search for neighbouring faces.
				//

				x1 = +INFINITY;
				z1 = +INFINITY;

				x2 = -INFINITY;
				z2 = -INFINITY;

				for (p = 0; p < 4; p++)
				{
					px = prim_points[f->Points[p]].X;
					pz = prim_points[f->Points[p]].Z;

					if (px < x1) {x1 = px;}
					if (px > x2) {x2 = px;}

					if (pz < z1) {z1 = pz;}
					if (pz > z2) {z2 = pz;}
				}

				#define JUST_IN_CASE 8

				x1 -= JUST_IN_CASE;
				z1 -= JUST_IN_CASE;

				x2 += JUST_IN_CASE;
				z2 += JUST_IN_CASE;

				x1 >>= PAP_SHIFT_LO;
				z1 >>= PAP_SHIFT_LO;
				x2 >>= PAP_SHIFT_LO;
				z2 >>= PAP_SHIFT_LO;

				SATURATE(x1, 0, PAP_SIZE_LO-1);
				SATURATE(x2, 0, PAP_SIZE_LO-1);
				SATURATE(z1, 0, PAP_SIZE_LO-1);
				SATURATE(z2, 0, PAP_SIZE_LO-1);

				for (mx = x1; mx <= x2; mx++)
				for (mz = z1; mz <= z2; mz++)
				{
					//
					// Go through the walkable faces above this mapsquare.
					//

					index = PAP_2LO(mx,mz).Walkable; //MAP[MAP_INDEX(mx,mz)].Walkable;

					while(index)
					{
						j     = index;

						if(j<0)
						{
							index = roof_faces4[-j].Next;
						}
						else
						{
							index = prim_faces4[j].WALKABLE;
						}

						if (j <= 0 || j == i)
						{
							continue;
						}

						g = &prim_faces4[j];

						//
						// Faces created in building city have the FaceFlags walkable bit set
						// and faces flagged in the editor have their DrawFlags set!
						//

						ASSERT(g->FaceFlags & FACE_FLAG_WALKABLE);

						g->FaceFlags |= FACE_FLAG_WALKABLE;
						g->DrawFlags |= POLY_FLAG_WALKABLE;

						for (ei = 0; ei < 4; ei++)
						{
							ip1 = f->Points[point_order[ei]];
							ip2 = f->Points[point_order[(ei + 1) & 0x3]];

							//
							// The world-space position of these points.
							//

							ip1x = prim_points[ip1].X;
							ip1y = prim_points[ip1].Y;
							ip1z = prim_points[ip1].Z;

							ip2x = prim_points[ip2].X;
							ip2y = prim_points[ip2].Y;
							ip2z = prim_points[ip2].Z;

							for (ej = 0; ej < 4; ej++)
							{
								jp1 = g->Points[point_order[ej]];
								jp2 = g->Points[point_order[(ej + 1) & 0x3]];

								//
								// The world-space position of these points.
								//

								jp1x = prim_points[jp1].X;
								jp1y = prim_points[jp1].Y;
								jp1z = prim_points[jp1].Z;

								jp2x = prim_points[jp2].X;
								jp2y = prim_points[jp2].Y;
								jp2z = prim_points[jp2].Z;

								//
								// If the edges of each face share the same primpoints or
								// the same world space positions, then clear the edge flags.
								//

								if ((ip1 == jp2 &&
									 ip2 == jp1) ||
									(ip1x == jp2x &&
									 ip1y == jp2y &&
									 ip1z == jp2z &&
									 ip2x == jp1x &&
									 ip2y == jp1y &&
									 ip2z == jp1z))
								{
									//
									// Clear the edge flags on the two faces.
									//

									f->FaceFlags &= ~(FACE_FLAG_SLIDE_EDGE << ei);
									g->FaceFlags &= ~(FACE_FLAG_SLIDE_EDGE << ej);
								}
							}
						}
					}
				}
			}
		}
	}

	//
	// We only 'grab' those edges marked as SLIDE_EDGES. The problem is that
	// we might not want to grab some edges.  In particular we don't grab edges
	// of walkable faces that are too close the the ground and edges that lie
	// along the bottom of walkable faces.
	//

	for (i = 1; i < next_prim_face4; i++)
	{
		f = &prim_faces4[i];

		if (f->FaceFlags & FACE_FLAG_WALKABLE)
		{
			if (f->FaceFlags & FACE_FLAG_WMOVE)
			{
				//
				// Don't check moveable walkable faces.
				//
			}
			else
			{
				for (ei = 0; ei < 4; ei++)
				{
					if (f->FaceFlags & (FACE_FLAG_SLIDE_EDGE << ei))
					{
						ip1 = f->Points[point_order[(ei + 0) & 0x3]];
						ip2 = f->Points[point_order[(ei + 1) & 0x3]];

						//
						// The world-space position of these points.
						//

						ip1x = prim_points[ip1].X;
						ip1y = prim_points[ip1].Y;
						ip1z = prim_points[ip1].Z;

						ip2x = prim_points[ip2].X;
						ip2y = prim_points[ip2].Y;
						ip2z = prim_points[ip2].Z;

						//
						// Look for a position beyond this edge.
						//

						bx = ip1x + ip2x >> 1;
						by = ip1y + ip2y >> 1;
						bz = ip1z + ip2z >> 1;

						dx = ip2x - ip1x;
						dy = ip2y - ip1y;
						dz = ip2z - ip1z;

						len = QDIST2(abs(dx),abs(dz)) + 1;

						dx = dx * 35 / len;
						dz = dz * 35 / len;

						bx += dz;
						bz -= dx;

						//
						// Is there a face or someground here?
						//

						if (!WITHIN(bx, 0, (PAP_SIZE_HI << 8) - 1) ||
							!WITHIN(bz, 0, (PAP_SIZE_HI << 8) - 1))
						{
							//
							// Something dodgey is going on...
							//

							near_height = 0;
						}
						else
						{
							near_height = find_height_for_this_pos(bx, bz, &pos_face);
						}

						if (near_height > by - SLIDE_EDGE_HEIGHT)
						{
							//
							// The ground or another face is too close to this edge.
							//

							f->FaceFlags &= ~(FACE_FLAG_SLIDE_EDGE << ei);
						}
					}
				}
			}
		}
	}

	//
	// Some edges lie underneath fences- so we don't want to be able to grab them.
	//


	for (i = 1; i < next_prim_face4; i++)
	{
		f = &prim_faces4[i];

		if (f->FaceFlags & FACE_FLAG_WALKABLE)
		{
			if (f->FaceFlags & FACE_FLAG_WMOVE)
			{
				//
				// Don't check moveable walkable faces.
				//
			}
			else
			{
				for (ei = 0; ei < 4; ei++)
				{
					if (f->FaceFlags & (FACE_FLAG_SLIDE_EDGE << ei))
					{
						ip1 = f->Points[point_order[(ei + 0) & 0x3]];
						ip2 = f->Points[point_order[(ei + 1) & 0x3]];

						//
						// The world-space position of these points.
						//

						ip1x = prim_points[ip1].X;
						ip1y = prim_points[ip1].Y;
						ip1z = prim_points[ip1].Z;

						ip2x = prim_points[ip2].X;
						ip2y = prim_points[ip2].Y;
						ip2z = prim_points[ip2].Z;

						if (does_fence_lie_along_line(ip1x, ip1z, ip2x, ip2z))
						{
							f->FaceFlags &= ~(FACE_FLAG_SLIDE_EDGE << ei);
						}
					}
				}
			}
		}
	}
}

void get_rotated_point_world_pos(
		SLONG  point,				// -1 => A random point.
		SLONG  prim,
		SLONG  prim_x,
		SLONG  prim_y,
		SLONG  prim_z,
		SLONG  prim_yaw,
		SLONG  prim_pitch,
		SLONG  prim_roll,
		SLONG *px,
		SLONG *py,
		SLONG *pz)
{
	SLONG matrix[9];

	ASSERT(WITHIN(prim, 1, next_prim_object - 1));

	PrimObject *po = &prim_objects[prim];

	SLONG num_points = po->EndPoint - po->StartPoint;

	//
	// The rotation matrix of the prim.
	//

	FMATRIX_calc(
		matrix,
		prim_yaw,
		prim_pitch,
		prim_roll);

	if (point == -1)
	{
		point  = Random() % num_points;
		point += po->StartPoint;
	}
	else
	{
		point += po->StartPoint;
	}

	SLONG x = prim_points[point].X;
	SLONG y = prim_points[point].Y;
	SLONG z = prim_points[point].Z;

	FMATRIX_MUL_BY_TRANSPOSE(
		matrix,
		x,
		y,
		z);

	x += prim_x;
	y += prim_y;
	z += prim_z;

   *px = x;
    *py = y;
   *pz = z;
}
#endif


SLONG slide_along_prim(
		SLONG  prim,
		SLONG  prim_x,
		SLONG  prim_y,
		SLONG  prim_z,
		SLONG  prim_yaw,
		SLONG  x1, SLONG  y1, SLONG  z1,
		SLONG *x2, SLONG *y2, SLONG *z2,
		SLONG  radius,
		SLONG  shrink,
		SLONG  dont_slide)	// TRUE => Don't move if the vector collides with the prim.
{
	SLONG old_x2 = *x2;
	SLONG old_y2 = *y2;
	SLONG old_z2 = *z2;

	SWORD y_bot;
	SWORD y_top;

	PrimInfo *pi;

	x1 >>= 8;
	y1 >>= 8;
	z1 >>= 8;


   *x2 >>= 8;
   *y2 >>= 8;
   *z2 >>= 8;

	if (shrink)
	{
		//
		// Reduce the radius.
		//

		radius -= 0x30;
	}

	//
	// The prim info...
	//

	pi = get_prim_info(prim);

	//
	// Is the 'y' range ok?
	//

	#define FURN_UNDERNEATH_BOT 96
	#define FURN_UNDERNEATH_TOP 0

	y_bot = prim_y + pi->miny - FURN_UNDERNEATH_BOT;
	y_top = prim_y + pi->maxy - FURN_UNDERNEATH_TOP;

	if (shrink)
	{
		y_top = y_bot + (y_top - y_bot >> 2);
	}

	if (WITHIN(y1, y_bot, y_top))
	{
		if (slide_around_box(
					prim_x,
					prim_z,
					pi->minx,
					pi->minz,
					pi->maxx,
					pi->maxz,
					prim_yaw,
					radius,
					x1,
					z1,
				    x2,
				    z2))
		{
			*x2 <<= 8;
			*y2 <<= 8;
			*z2 <<= 8;

			return TRUE;
		}
	}

   *x2 = old_x2;
   *y2 = old_y2;
   *z2 = old_z2;

	return FALSE;
}


UBYTE prim_get_collision_model(SLONG prim)
{
	ASSERT(WITHIN(prim, 0, 255));

	return prim_objects[prim].coltype;
}

UBYTE prim_get_shadow_type(SLONG prim)
{
	ASSERT(WITHIN(prim, 0, 255));

	return prim_objects[prim].shadowtype;
}


#ifndef PSX
void	fn_anim_prim_normal(Thing *p_thing)
{
	Switch		*the_switch;
	SLONG	     tween_step;
	DrawTween	*draw_info;

	draw_info=p_thing->Draw.Tweened;
	tween_step=draw_info->CurrentFrame->TweenStep<<1;

	tween_step = (tween_step*TICK_RATIO)>>TICK_SHIFT;
	if(tween_step==0)
		tween_step=1;
	draw_info->AnimTween += tween_step; //256/(draw_info->CurrentFrame->TweenStep+1);

	//
	// This prim is animating, so animate it!
	//

	if(draw_info->AnimTween>256)
	{
		draw_info->AnimTween-=256;

		SLONG advance_keyframe(DrawTween *draw_info);

		if (advance_keyframe(draw_info))
		{
			//
			// Anim over.
			//

			switch(get_anim_prim_type(p_thing->Index))
			{
				case ANIM_PRIM_TYPE_NORMAL:
					break;

				case ANIM_PRIM_TYPE_DOOR:

					if (p_thing->Draw.Tweened->CurrentAnim == 2)
					{
						//
						// Door just opened.
						//

						p_thing->Flags |=  FLAGS_SWITCHED_ON;
					}
					else
					{
						//
						// Door just shut.
						//

						p_thing->Flags &= ~FLAGS_SWITCHED_ON;
					}

					p_thing->StateFn = NULL;

					break;

				case ANIM_PRIM_TYPE_SWITCH:

					p_thing->Flags ^= FLAGS_SWITCHED_ON;

					//
					// Mark anim as finished- don't call this function anymore!
					//

					p_thing->StateFn = NULL;

					break;

				default:
					ASSERT(0);
					break;
			}
		}
	}
}

void	create_anim_prim(SLONG x,SLONG y,SLONG z,SLONG prim, SLONG yaw)
{
 	SLONG	new_thing;
	Thing	*t_thing;
 	new_thing	=	alloc_primary_thing(CLASS_ANIM_PRIM);
	if(new_thing)
	{
		t_thing					=	TO_THING(new_thing);
		t_thing->Class			=	CLASS_ANIM_PRIM;
		t_thing->WorldPos.X		=	x<<8;
		t_thing->WorldPos.Y		=	y<<8;
		t_thing->WorldPos.Z		=	z<<8;
		t_thing->StateFn		=	NULL;
		t_thing->Draw.Tweened	=	alloc_draw_tween(DT_ROT_MULTI);
		t_thing->DrawType		=	DT_ANIM_PRIM;
		t_thing->Index          =	prim;
		t_thing->Flags			=	0;

		t_thing->Draw.Tweened->Angle			=	yaw;
		t_thing->Draw.Tweened->Roll				=	0;
		t_thing->Draw.Tweened->Tilt				=	0;
		t_thing->Draw.Tweened->AnimTween		=	0;
		t_thing->Draw.Tweened->TweenStage		=	0;
		t_thing->Draw.Tweened->NextFrame		=	NULL;
		t_thing->Draw.Tweened->QueuedFrame		=	NULL;
		t_thing->Draw.Tweened->TheChunk			=	&anim_chunk[prim];
		t_thing->Draw.Tweened->CurrentFrame		=	anim_chunk[prim].AnimList[1];
		t_thing->Draw.Tweened->CurrentAnim		=	1;

		if(t_thing->Draw.Tweened->CurrentFrame)
		{
			t_thing->Draw.Tweened->NextFrame	=	t_thing->Draw.Tweened->CurrentFrame->NextFrame;
		}

		t_thing->Draw.Tweened->FrameIndex		=	0;
		t_thing->Draw.Tweened->Flags			=	0;

		add_thing_to_map(t_thing);

		switch(get_anim_prim_type(t_thing->Index))
		{
			case ANIM_PRIM_TYPE_NORMAL:

				//
				// Start off doing the first animation.
				//

				t_thing->StateFn=fn_anim_prim_normal;

				break;

			case ANIM_PRIM_TYPE_DOOR:
				t_thing->Flags |= FLAGS_SWITCHED_ON;
				break;
			case ANIM_PRIM_TYPE_SWITCH:
				break;

			default:
				ASSERT(0);
				break;
		}
	}
}

void	set_anim_prim_anim(SLONG anim_prim, SLONG anim)
{
	Thing *t_thing = TO_THING(anim_prim);

	ASSERT(WITHIN(anim, 1, anim_chunk[t_thing->Index].MaxAnimFrames - 1));

	t_thing->Draw.Tweened->AnimTween	 = 0;
	t_thing->Draw.Tweened->NextFrame	 = NULL;
	t_thing->Draw.Tweened->QueuedFrame	 = NULL;
	t_thing->Draw.Tweened->CurrentFrame	 = anim_chunk[t_thing->Index].AnimList[anim];
	t_thing->Draw.Tweened->FrameIndex	 = 0;
	t_thing->Draw.Tweened->CurrentAnim   = anim;

	if (t_thing->Draw.Tweened->CurrentFrame)
	{
		t_thing->Draw.Tweened->NextFrame = t_thing->Draw.Tweened->CurrentFrame->NextFrame;
	}

	t_thing->StateFn=fn_anim_prim_normal;
}



SLONG get_anim_prim_type(SLONG anim_prim)
{
	switch(anim_prim)
	{
		case 4:
			return ANIM_PRIM_TYPE_SWITCH;
		case 3:
		case 5:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
			return ANIM_PRIM_TYPE_DOOR;

		default:
			return ANIM_PRIM_TYPE_NORMAL;
	}
}
#endif


#ifndef	PSX
#define MAX_FIND_ANIM_PRIMS 8

THING_INDEX found_aprim[MAX_FIND_ANIM_PRIMS];

SLONG find_anim_prim(
		SLONG x,
		SLONG y,
		SLONG z,
		SLONG range,
		ULONG type_bit_field)
{
	SLONG i;

	SLONG dx;
	SLONG dy;
	SLONG dz;
	SLONG dist;

	THING_INDEX best_thing = NULL;
	SLONG       best_dist  = range + 1;

	//
	// Find all anim prims in range.
	//

	SLONG       num;

	num = THING_find_sphere(
			x, y, z,
			range,
			found_aprim,
			MAX_FIND_ANIM_PRIMS,
			(1 << CLASS_ANIM_PRIM));

	//
	// Find the nearest anim-prim.
	//

	for (i = 0; i < num; i++)
	{
		//
		// Only consider anim prims of the correct type.
		//

		if (type_bit_field & (1 << get_anim_prim_type(TO_THING(found_aprim[i])->Index)))
		{
			dx = (TO_THING(found_aprim[i])->WorldPos.X >> 8) - x;
			dy = (TO_THING(found_aprim[i])->WorldPos.Y >> 8) - y;
			dz = (TO_THING(found_aprim[i])->WorldPos.Z >> 8) - z;

			dist = abs(dx) + abs(dy) + abs(dz);

			if (dist < best_dist)
			{
				best_dist  = dist;
				best_thing = found_aprim[i];
			}
		}
	}

	return best_thing;
}

void toggle_anim_prim_switch_state(SLONG anim_prim_thing_index)
{
	Thing *t_thing = TO_THING(anim_prim_thing_index);

	//
	// Do nothing if the prim is animating.
	//

	if (t_thing->StateFn)
	{
		return;
	}

	if (t_thing->Flags & FLAGS_SWITCHED_ON)
	{
		set_anim_prim_anim(anim_prim_thing_index, 2);
	}
	else
	{
		set_anim_prim_anim(anim_prim_thing_index, 1);
	}
}
#endif
//
// Expands the given bounding box using the given animated-prim position.
//

#ifndef	PSX
void expand_anim_prim_bbox(
		SLONG prim,
		GameKeyFrameElement *anim_info,

		SLONG *min_x,
		SLONG *min_y,
		SLONG *min_z,

		SLONG *max_x,
		SLONG *max_y,
		SLONG *max_z)
{
	SLONG i;

	float px;
	float py;
	float pz;

	SLONG ix;
	SLONG iy;
	SLONG iz;

	Matrix33 mat;
	PrimObject *po;

	//
	// Build the rotation matrix.
	//

	CMatrix33	m1;
	GetCMatrix(anim_info, &m1);
	build_tween_matrix(&mat, &m1, &m1, 0);

	//
	// Do this in floats for now!
	//

	float fmatrix[9];
	float ox = float(anim_info->OffsetX);
	float oy = float(anim_info->OffsetY);
	float oz = float(anim_info->OffsetZ);

	fmatrix[0] = float(mat.M[0][0]) * (1.0F / 32768.0F);
	fmatrix[1] = float(mat.M[0][1]) * (1.0F / 32768.0F);
	fmatrix[2] = float(mat.M[0][2]) * (1.0F / 32768.0F);
	fmatrix[3] = float(mat.M[1][0]) * (1.0F / 32768.0F);
	fmatrix[4] = float(mat.M[1][1]) * (1.0F / 32768.0F);
	fmatrix[5] = float(mat.M[1][2]) * (1.0F / 32768.0F);
	fmatrix[6] = float(mat.M[2][0]) * (1.0F / 32768.0F);
	fmatrix[7] = float(mat.M[2][1]) * (1.0F / 32768.0F);
	fmatrix[8] = float(mat.M[2][2]) * (1.0F / 32768.0F);

	//
	// The prim object.
	//

	ASSERT(WITHIN(prim, 1, next_prim_object - 1));

	po = &prim_objects[prim];

	for (i = po->StartPoint; i < po->EndPoint; i++)
	{
		px = float(prim_points[i].X);
		py = float(prim_points[i].Y);
		pz = float(prim_points[i].Z);

		MATRIX_MUL(
			fmatrix,
			px,
			py,
			pz);

		px += ox;
		py += oy;
		pz += oz;

		ix = SLONG(px);
		iy = SLONG(py);
		iz = SLONG(pz);

		if (ix < *min_x) {*min_x = ix;}
		if (iy < *min_y) {*min_y = iy;}
		if (iz < *min_z) {*min_z = iz;}

		if (ix > *max_x) {*max_x = ix;}
		if (iy > *max_y) {*max_y = iy;}
		if (iz > *max_z) {*max_z = iz;}
	}
}



void find_anim_prim_bboxes()
{
	SLONG i;
	SLONG j;
	SLONG dist;
	SLONG ele_count;
	SLONG start_object;

	GameKeyFrameElement *ele;
	AnimPrimBbox        *pmb;

	for (i = 1; i < MAX_ANIM_CHUNKS; i++)
	{
		//
		// Initialise the bounding box.
		//

		pmb = &anim_prim_bbox[i];

		pmb->minx = +INFINITY;
		pmb->miny = +INFINITY;
		pmb->minz = +INFINITY;
		pmb->maxx = -INFINITY;
		pmb->maxy = -INFINITY;
		pmb->maxz = -INFINITY;

		if (anim_chunk[i].MultiObject[0] == 0)
		{
			//
			// This anim prim has not been loaded.
			//

			continue;
		}

		ASSERT(anim_chunk[i].AnimList[1]);

		ele = anim_chunk[i].AnimList[1]->FirstElement;

		ASSERT(ele != NULL);

		ele_count = anim_chunk[i].ElementCount;
		start_object = prim_multi_objects[anim_chunk[i].MultiObject[0]].StartObject;

		//
		// Expand the bouding box with each prim.
		//

		for (j = 0; j < ele_count; j++)
		{
			expand_anim_prim_bbox(
				start_object + j,
				ele          + j,
			   &pmb->minx,
			   &pmb->miny,
			   &pmb->minz,
			   &pmb->maxx,
			   &pmb->maxy,
			   &pmb->maxz);
		}

		//
		// Make sure the bounding box isn't too small.
		//

		dist = pmb->maxx - pmb->minx;

		if (dist < PRIM_MIN_BBOX)
		{
			pmb->maxx += PRIM_MIN_BBOX - dist >> 1;
			pmb->minx -= PRIM_MIN_BBOX - dist >> 1;
		}

		dist = pmb->maxz - pmb->minz;

		if (dist < PRIM_MIN_BBOX)
		{
			pmb->maxz += PRIM_MIN_BBOX - dist >> 1;
			pmb->minz -= PRIM_MIN_BBOX - dist >> 1;
		}

	}
}
#endif



void mark_prim_objects_as_unloaded()
{
	//
	// Easy!
	//

	memset((UBYTE*)prim_objects, 0, sizeof(PrimObject) * 256);
}


#ifndef	PSX
void	re_center_prim(SLONG prim,SLONG dx,SLONG dy,SLONG dz)
{
 	SLONG	c0;
	struct	PrimObject	*p_obj;
	SLONG	sp,ep;

	p_obj    =&prim_objects[prim];

	sp=p_obj->StartPoint;
	ep=p_obj->EndPoint;

	for(c0=sp;c0<ep;c0++)
	{
		DebugText(" prim %d x %d y %d  z %d \n",prim,prim_points[c0].X,prim_points[c0].Y,prim_points[c0].Z);
		prim_points[c0].X+=dx;
		prim_points[c0].Y+=dy;
		prim_points[c0].Z+=dz;
	}
}
#endif

SLONG does_fence_lie_along_line(SLONG x1, SLONG z1, SLONG x2, SLONG z2)
{
	//
	// Only orthogonal lines supported!
	//

	SLONG dx = abs(x2 - x1);
	SLONG dz = abs(z2 - z1);

	if (dx < dz)
	{
		x1 = x2;
	}
	else
	{
		z1 = z2;
	}

	SLONG mx1 = x1 - 0x80 >> PAP_SHIFT_LO;
	SLONG mz1 = z1 - 0x80 >> PAP_SHIFT_LO;
	SLONG mx2 = x2 + 0x80 >> PAP_SHIFT_LO;
	SLONG mz2 = z2 + 0x80 >> PAP_SHIFT_LO;

	SLONG minx;
	SLONG maxx;
	SLONG minz;
	SLONG maxz;

	SLONG px1;
	SLONG pz1;
	SLONG px2;
	SLONG pz2;

	SLONG mx;
	SLONG mz;

	SLONG f_list;
	SLONG exit;
	SLONG facet;

	DFacet *df;

	SATURATE(mx1, 0, PAP_SIZE_LO - 1);
	SATURATE(mz1, 0, PAP_SIZE_LO - 1);

	SATURATE(mx2, 0, PAP_SIZE_LO - 1);
	SATURATE(mz2, 0, PAP_SIZE_LO - 1);

	for (mx = mx1; mx <= mx2; mx++)
	for (mz = mz1; mz <= mz2; mz++)
	{
		f_list = PAP_2LO(mx,mz).ColVectHead;

		if (f_list)
		{
			exit = FALSE;

			do
			{
				facet = facet_links[f_list++];

				if (facet < 0)
				{
					facet = -facet;
					exit  =  TRUE;
				}

				df = &dfacets[facet];

				if (df->FacetType == STOREY_TYPE_FENCE      ||
					df->FacetType == STOREY_TYPE_FENCE_FLAT ||
					df->FacetType == STOREY_TYPE_FENCE_BRICK||
					(df->FacetFlags& FACET_FLAG_BARB_TOP)

					)
				{
					if (x1 == x2)
					{
						if ((df->x[0] << 8) == x1)
						{
							minz = df->z[0] << 8;
							maxz = df->z[1] << 8;

							if (minz > maxz)
							{
								SWAP(minz, maxz);
							}

							if (WITHIN(z1, minz, maxz) &&
								WITHIN(z2, minz, maxz))
							{
								return TRUE;
							}
						}
					}
					else
					{
						if ((df->z[0] << 8) == z1)
						{
							minx = df->x[0] << 8;
							maxx = df->x[1] << 8;

							if (minx > maxx)
							{
								SWAP(minx, maxx);
							}

							if (WITHIN(x1, minx, maxx) &&
								WITHIN(x2, minx, maxx))
							{
								return TRUE;
							}
						}
					}
				}

			} while(!exit);
		}
	}

	return FALSE;
}



void clear_all_wmove_flags(void)
{
	SLONG i;

	PrimFace3 *f3;
	PrimFace4 *f4;

	for (i = 1; i < next_prim_face3; i++)
	{
		f3 = &prim_faces3[i];

		f3->FaceFlags &= ~FACE_FLAG_WMOVE;
	}

	for (i = 1; i < next_prim_face4; i++)
	{
		f4 = &prim_faces4[i];

		f4->FaceFlags &= ~FACE_FLAG_WMOVE;
	}
}


