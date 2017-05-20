// Bucket.h
// Guy Simmons, 24th October 1997.

#ifndef	BUCKET_H
#define	BUCKET_H

//---------------------------------------------------------------

#define	MAX_LISTS			24
#define	MAX_BUCKETS			1024
#define	BUCKET_POOL_SIZE	(512*1024)

#define	TEXTURE_LIST_1		0
#define	TEXTURE_LIST_2		1
#define	TEXTURE_LIST_3		2
#define	TEXTURE_LIST_4		3
#define	TEXTURE_LIST_5		4
#define	TEXTURE_LIST_6		5

#define LAST_NORMAL_PAGE	(MAX_LISTS-8)

#define SKY_LIST_2			(MAX_LISTS-7)
#define SKY_LIST_1			(MAX_LISTS-6)
#define	MASK_LIST_1			(MAX_LISTS-5)
#define	COLOUR_LIST_1		(MAX_LISTS-4)
#define WATER_LIST_1		(MAX_LISTS-3)
#define	ALPHA_LIST_1		(MAX_LISTS-2)
#define FOG_LIST_1			(MAX_LISTS-1)

#define	BUCKET_NONE			0
#define	BUCKET_QUAD			1
#define	BUCKET_TRI			2
#define	BUCKET_LINE			3
#define	BUCKET_POINT		4

#define	GET_BUCKET(s)		(s*)e_buckets;e_buckets+=sizeof(s)
#define	BUCKETS_FULL		(e_buckets>=e_end_buckets)

struct	BucketHead
{
	BucketHead			*NextBucket;
	ULONG				BucketType;
};

struct	BucketQuad
{
	BucketHead			BucketHeader;
	UWORD				P[4];
};

struct	BucketTri
{
	BucketHead			BucketHeader;
	UWORD				P[4];
};

struct	BucketLine
{
	BucketHead			BucketHeader;
	UWORD				P[2];
};

struct	BucketPoint
{
	BucketHead			BucketHeader;
	UWORD				P[2];
};

extern UBYTE			e_bucket_pool[BUCKET_POOL_SIZE],
						*e_buckets,
						*e_end_buckets;
extern BucketHead		*bucket_lists[MAX_LISTS][MAX_BUCKETS+1];

//---------------------------------------------------------------

inline void	reset_buckets(void)
{
	e_buckets		=	e_bucket_pool;
	e_end_buckets	=	&e_bucket_pool[BUCKET_POOL_SIZE];
}

//---------------------------------------------------------------

inline	SLONG	check_vertex(D3DTLVERTEX	*v)
{
	if(v->sz<0.0 || v->sz>1.0 || v->rhw<0.0 || v->rhw>1.0)
		return(1);
	else
		return(0);

}
extern	D3DTLVERTEX			vertex_pool[];

inline void	add_bucket(BucketHead *header,UBYTE bucket_type,SLONG list_index,SLONG z)
{
	SLONG	c0;
	BucketQuad	*the_quad;
	BucketTri	*the_tri;
	BucketLine	*the_line;
	D3DTLVERTEX		*vertex;

/*
	SLONG		index;


	if(z>0)
		index	=	MAX_BUCKETS/z;
	else if(z==0)
		index	=	MAX_BUCKETS;
	else
		index	=	0;

	header->BucketType				=	bucket_type;
	header->NextBucket				=	bucket_lists[list_index][index];
	bucket_lists[list_index][index]	=	header;
*/
	header->BucketType						=	bucket_type;
	header->NextBucket						=	bucket_lists[list_index][z];
	bucket_lists[list_index][z]				=	header;
/*
	switch(bucket_type)
	{
		case	BUCKET_QUAD:
				the_quad=(struct BucketQuad*)header;
				for(c0=0;c0<4;c0++)
				{
					SLONG	p0;
					vertex=&vertex_pool[the_quad->P[c0]];

					if(check_vertex(vertex))
					{
						ASSERT(0);
					}
				}

			break;
		case	BUCKET_TRI:
				the_tri=(struct BucketTri*)header;
				for(c0=0;c0<3;c0++)
				{
					SLONG	p0;
					vertex=&vertex_pool[the_tri->P[c0]];

					if(check_vertex(vertex))
					{
						ASSERT(0);
					}
				}
			break;
		case	BUCKET_LINE:
				the_line	=	(BucketLine*)header;
				for(c0=0;c0<2;c0++)
				{
					SLONG	p0;
					vertex=&vertex_pool[the_line->P[c0]];

					if(check_vertex(vertex))
					{
						ASSERT(0);
					}
				}
			break;

	}			
*/

}

//---------------------------------------------------------------

void	init_buckets(void);

//---------------------------------------------------------------

#endif



