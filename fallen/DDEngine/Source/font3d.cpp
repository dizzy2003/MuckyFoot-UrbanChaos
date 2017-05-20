
// THIS IS PANTS!
#if 0

/************************************************************
 *
 *   font3d.cpp
 *   3D text writer
 *
 */

#include "font3d.h"
#include <string>
#include <cmath>
#include <DDLib.h>
#include "matrix.h"
#include "StdKeybd.h"
#include "poly.h"

const char fontindex[] = "ABCDEFGHIJKLMNOPQRSTUVWYXZ0123456789";

//
// Some crappy vector stuff
//

float Magnitude(FontVec &vec) {
  return sqrt((vec.x*vec.x)+(vec.y*vec.y)+(vec.z*vec.z));
}

void Normalize(FontVec &Result) {
   float holder;

   holder=Magnitude(Result);
   if (holder) {
     Result.x/=holder;
     Result.y/=holder;
     Result.z/=holder;
   }

}

FontVec Cross(FontVec &Temp1, FontVec &Temp2) {
	FontVec Result;

    Result.x=(Temp1.y*Temp2.z)-(Temp1.z*Temp2.y);  
    Result.y=(Temp1.z*Temp2.x)-(Temp1.x*Temp2.z);
    Result.z=(Temp1.x*Temp2.y)-(Temp1.y*Temp2.x);

	return Result;
}

FontVec CalcSurfaceNormal(FontVec &V0, FontVec &V1, FontVec &V2) {
    FontVec Result,Temp1, Temp2;

	Temp1.x=V1.x-V0.x; Temp1.y=V1.y-V0.y; Temp1.z=V1.z-V0.z;
	Temp2.x=V1.x-V2.x; Temp2.y=V1.y-V2.y; Temp2.z=V1.z-V2.z;

	Result=Cross(Temp1,Temp2);
    Normalize(Result);
	return Result;
}

void MixNorms(FontVec &norm, FontVec &vec) {

//	if (vec.nx==vec.ny==vec.nz==0) { Strange!

	if (vec.nx == 0 && vec.ny == 0 && vec.nz == 0) {
		vec.nx=norm.x; vec.ny=norm.y; vec.nz=norm.z;
	} else {
		vec.nx+=norm.x; vec.ny+=norm.y; vec.nz+=norm.z;
		vec.nx/=2; vec.ny/=2; vec.nz/=2;
	}
}

/*
//
// some crappy matrix stuff
//

void PerspMatrix(float matrix[9]) {

  memset(matrix,0,sizeof(matrix));
  matrix[0]=d;
  matrix[5]=d;
  matrix[10]=1;
  matrix[11]=-d;
  matrix[14]=1;

}
*/

//
// construct, destruct.... FLA would be proud :P
//

Font3D::Font3D(char *path, float scale) {
	UBYTE i;
	CBYTE fn[400], chr[2];

	memset(data,0,sizeof(data));
	nextchar=0;
	fontscale=scale;

	chr[1]=0;

	for (i=0;i<strlen(fontindex);i++) {
		chr[0]=fontindex[i];
		strcpy(fn,path);
		strcat(fn,chr);
		strcat(fn,".sex");
		AddLetter(fn);
	}
}


Font3D::~Font3D() {
	ClearLetters();
}

//
// reset the font data
//

void Font3D::ClearLetters() {
	UBYTE i;

	for (i=0;i<100;i++) {
	  if (data[i].pts)   delete [] data[i].pts;
	  if (data[i].faces) delete [] data[i].faces;
	}
	memset(data,0,sizeof(data));
	nextchar=0;
}

//
// load a new .sex file in as a font letter
//


void Font3D::AddLetter(char *fn) {
	FILE     *handle;
	float     x,y,z;
	SLONG	  d;
	SLONG     match;
	FontData *currchar;
	FontVec  *vec  = 0;
	FontFace *face = 0;
	SLONG	  m, p1, p2, p3, t1, t2 ,t3;
	SLONG	  edge_a, edge_b, edge_c;
	CBYTE	  line[6000];
	float     minx,miny,minz,maxx,maxy,maxz,ctrx,ctry,ctrz;

	minx=miny=minz= 999999;
	maxx=maxy=maxz=-999999;

	handle = MF_Fopen(fn, "rb");

	if (handle == NULL) return; // Could not open file

	if (nextchar==100)  return; // no more room, erk

	currchar=&data[nextchar];
	nextchar++;

	while(fgets(line, 6000, handle)) {
		// Steal info from comments :P

		match = sscanf(line, "#     Num points    : %d",&d);
		if (match==1) {
		  currchar->numpts=d;
		  currchar->pts = vec = new FontVec[d];
		}

		match = sscanf(line, "#     Num faces     : %d",&d);
		if (match==1) {
		  currchar->numfaces=d;
		  currchar->faces = face = new FontFace[d];
		}

		// Ignore all other comment lines

		if (line[0] == '#') continue;

		// Add new vertices to pool

		match = sscanf(line, "Vertex: (%f,%f,%f)", &x, &y, &z);

		if ((match==3)&&vec) {
		  vec->x=x; vec->y=y; vec->z=z;
		  vec->nx=vec->ny=vec->nz=0;
		  if (x>maxx) maxx=x;
		  if (y>maxy) maxy=y;
		  if (z>maxz) maxz=z;
		  if (x<minx) minx=x;
		  if (y<miny) miny=y;
		  if (z<minz) minz=z;
		  vec++;
		}

		// Add new faces to pool
		match = sscanf(line, "Face: Material %d xyz (%d,%d,%d) uv (%d,%d,%d) edge (%d,%d,%d)", &m, &p1, &p2, &p3, &t1, &t2, &t3, &edge_a, &edge_b, &edge_c);

		if ((match == 7)||(match == 10)) {
		  face->a=&currchar->pts[p3];
		  face->b=&currchar->pts[p2];
		  face->c=&currchar->pts[p1];
		  face->norm=CalcSurfaceNormal(*face->a,*face->b,*face->c);
		  MixNorms(face->norm,*face->a);
		  MixNorms(face->norm,*face->b);
		  MixNorms(face->norm,*face->c);
		  face++;
		}


	}

	ctrx=(maxx+minx)/2;	ctry=(maxy+miny)/2;	ctrz=(maxz+minz)/2;

	for (d=0,vec=currchar->pts;d<currchar->numpts;d++,vec++) {
		vec->x-=ctrx; vec->y-=ctry; vec->z-=ctrz;
	}

	currchar->width=maxx-minx;


	MF_Fclose(handle);

}

//
// Get the width of a character
//

ULONG Font3D::LetterWidth(CBYTE chr) {
	SLONG ndx;
	FontData *currchar;

	if (chr==' ') return 10;
    ndx=strchr(fontindex,chr)-fontindex;
	if ((ndx<0)||(ndx>=strlen(fontindex))) return 0; // out of bounds
	return data[ndx].width*fontscale;
}

//
// Draw a character
//


void Font3D::DrawLetter(CBYTE chr, ULONG x, ULONG y, ULONG rgb, float yaw, float roll, float pitch, float scale) {
	SLONG ndx;
	FontData *currchar;
	FontVec  *pt;
	FontFace *face;
	float matrix[9];
	float px,py,pz, az;
    POLY_Point pp[3], *tri[3];

	if (scale<=0) return; // too small

	scale*=fontscale;

    ndx=strchr(fontindex,chr)-fontindex;
	if ((ndx<0)||(ndx>=strlen(fontindex))) return; // out of bounds
	currchar=&data[ndx];

	float POLY_screen_mul_x, POLY_screen_mul_y;

	POLY_screen_mul_x  = 640 * 0.5F / POLY_ZCLIP_PLANE;
	POLY_screen_mul_y  = 480 * 0.5F / POLY_ZCLIP_PLANE;

	tri[2] =	&pp[0];
	tri[1] =	&pp[1];
	tri[0] =	&pp[2];

	MATRIX_calc(matrix, yaw, pitch, roll);

	for (ndx=0,pt=currchar->pts;ndx<currchar->numpts;ndx++,pt++) {
		pt->tx=pt->x; pt->ty=pt->y; pt->tz=pt->z;
		MATRIX_MUL(matrix, pt->tx,pt->ty,pt->tz);

		pt->tnx=pt->nx;  pt->tny=pt->ny;  pt->tnz=pt->nz;
		MATRIX_MUL(matrix, pt->tnx, pt->tny, pt->tnz);
		pt->tnx*=0.15; pt->tny*=0.15; pt->tnz*=0.15;


		// some scale factor or other...
		pt->tx*=scale; pt->ty*=scale; pt->tz*=scale;


		az=((pt->tz*0.25)+75)*0.01;

	  	pt->tz=150-pt->tz;
		pt->tz=1.0/pt->tz;

		pt->tx*=az;
		pt->ty*=az;

		pt->tx+=x; pt->ty+=y;

	}
	
	for (ndx=0,face=currchar->faces;ndx<currchar->numfaces;ndx++,face++) {

	// cheesy debug kack:
/*		face->a->tx=face->c->tx=0;
		face->b->tx=640;
		face->a->ty=face->b->ty=200;
		face->c->ty=480;
*/

	//face->a->tz=face->b->tz=face->c->tz=0.5f;


	// Draw the face...

#define SPEC 0xff000000
//#define SPEC 0xff4f0000

////////////////////////////////////

		pp[0].X		=	face->a->tx;
		pp[0].Y		=	face->a->ty;
		pp[0].Z		=	face->a->tz;
		pp[0].u		=	(face->a->tx/640)+face->a->tnx;
		pp[0].v		=	(face->a->ty/640)+face->a->tny+0.2;
		pp[0].colour	=	rgb;//0xffffff;
		pp[0].specular=	SPEC;

		pp[1].X		=	face->b->tx;
		pp[1].Y		=	face->b->ty;
		pp[1].Z		=	face->b->tz;
		pp[1].u		=	(face->b->tx/640)+face->b->tnx;
		pp[1].v		=	(face->b->ty/640)+face->b->tny+0.2;
		pp[1].colour	=	rgb;//0xffffff;
		pp[1].specular	=	SPEC;

		pp[2].X		=	face->c->tx;
		pp[2].Y		=	face->c->ty;
		pp[2].Z		=	face->c->tz;
		pp[2].u		=	(face->c->tx/640)+face->c->tnx;
		pp[2].v		=	(face->c->ty/640)+face->c->tny+0.2;
		pp[2].colour	=	rgb;//0xffffff;
		pp[2].specular	=	SPEC;

		POLY_add_triangle(tri,POLY_PAGE_MENUTEXT,TRUE,TRUE);


/*
		// some fucked up second pass shit

		pp[0].X		=	face->a->tx;
		pp[0].Y		=	face->a->ty;
		pp[0].Z		=	face->a->tz;
		pp[0].u		=	(face->a->tx/640)+face->a->tnx;
		pp[0].v		=	(face->a->ty/960)+face->a->tny+0.5;
		pp[0].colour	=	rgb;//0xffffff;
		pp[0].specular=	SPEC;

		pp[1].X		=	face->b->tx;
		pp[1].Y		=	face->b->ty;
		pp[1].Z		=	face->b->tz;
		pp[1].u		=	(face->b->tx/640)+face->b->tnx;
		pp[1].v		=	(face->b->ty/960)+face->b->tny+0.5;
		pp[1].colour	=	rgb;//0xffffff;
		pp[1].specular	=	SPEC;

		pp[2].X		=	face->c->tx;
		pp[2].Y		=	face->c->ty;
		pp[2].Z		=	face->c->tz;
		pp[2].u		=	(face->c->tx/640)+face->c->tnx;
		pp[2].v		=	(face->c->ty/960)+face->c->tny+0.5;
		pp[2].colour	=	rgb;//0xffffff;
		pp[2].specular	=	SPEC;

		POLY_add_triangle(tri,POLY_PAGE_MENUPASS,FALSE,TRUE);
*/


	// Draw the drop shadow...

		pp[0].X		=	face->a->tx+10;
		pp[0].Y		=	face->a->ty+10;
		pp[0].Z		=	0.0001;
		pp[0].u		=	0.2;
		pp[0].v		=	0.2;
		pp[0].colour	=	0;
		pp[0].specular=	0xff000000;

		pp[1].X		=	face->b->tx+10;
		pp[1].Y		=	face->b->ty+10;
		pp[1].Z		=	0.0001;
		pp[1].u		=	0.8;
		pp[1].v		=	0.2;
		pp[1].colour	=	0;
		pp[1].specular	=	0xff000000;

		pp[2].X		=	face->c->tx+10;
		pp[2].Y		=	face->c->ty+10;
		pp[2].Z		=	0.0001;
		pp[2].u		=	0.2;
		pp[2].v		=	0.8;
		pp[2].colour	=	0;
		pp[2].specular	=	0xff000000;

		POLY_add_triangle(tri,POLY_PAGE_MENUTEXT,TRUE,TRUE);



	}
	
}

void Font3D::DrawString(CBYTE *str, ULONG ctrx, ULONG y, ULONG rgb, float scale, CBYTE wibble, UWORD zoom) {
	static float rotate=0;
	float thisone;
	float sep, x, f, zoomctr;
	UWORD i;
	CBYTE *c;

	//sep=11.5*scale;

	//sep = 9.0F * scale;

	// calculate proportional width
	sep=0;
	for(i=0,c=str;i<strlen(str);i++,c++) sep+=scale*LetterWidth(*c);

	x=ctrx-(sep*0.5);

	zoomctr=strlen(str);

//    x=ctrx-(strlen(str)*sep*0.5)+(sep*0.5);
	rotate++;
	if (wibble==2) rotate=90;
	if (rotate==500) rotate=0;
    for (i=0,c=str;i<strlen(str);i++,c++) {
		x+=LetterWidth(*c)*scale*0.5;
		thisone=rotate-(i*8);
		if (wibble&&(thisone>90)&&(thisone<180)) 
			thisone=90+((thisone-180)*4);
		else
			thisone=90;
		thisone*=PI/180;
		if (!zoom) {
		  DrawLetter(*c,x,y,rgb,0,0,thisone,scale);
		} else {
		  f=1+abs(zoomctr-(i*2));
		  f/=zoomctr;
		  f*=zoom;
		  DrawLetter(*c,x,y,rgb,0,0,thisone,scale-f);
		}
		x+=LetterWidth(*c)*scale*0.5;
	}
}


#endif