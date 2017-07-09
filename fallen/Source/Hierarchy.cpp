#include	"game.h"
#include	"Hierarchy.h"
#include	"fallen/headers/FMatrix.h"

// functions and data for animating people

//**************************************************************************************************
//JCL - Body Part parent names
CBYTE	*body_part_parent[][2]=
{
	{"pelvis"		, ""			},
	{"lfemur"		, "pelvis"		},
	{"ltibia"		, "lfemur"		},
	{"lfoot"		, "ltibia"		},
	{"torso"		, "pelvis"		},
	{"rhumorus"		, "torso"		},
	{"rradius"		, "rhumorus"	},
	{"rhand"		, "rradius"		},
	{"lhumorus"		, "torso"		},
	{"lradius"		, "lhumorus"	},
	{"lhand"		, "lradius"		},
	{"skull"		, "torso"		},
	{"rfemur"		, "pelvis"		},
	{"rtibia"		, "rfemur"		},
	{"rfoot"		, "rtibia"		}
};

//**************************************************************************************************
//! JCL - don't think this can be a permanent thing....
SLONG	body_part_parent_numbers[]=
{
	-1,
	0,
	1,
	2,
	0,
	4,
	5,
	6,
	4,
	8,
	9,
	4,
	0,
	12,
	13
};

//**************************************************************************************************
//! JCL - nor this!!
SLONG	body_part_children[][5] =
{
	{1, 4, 12, -1, 0},
	{2, -1, 0, 0, 0},
	{3, -1, 0, 0, 0},
	{-1, 0, 0, 0, 0},
	{5, 8, 11, -1, 0},
	{6, -1, 0, 0, 0},
	{7, -1, 0, 0, 0},
	{-1, 0, 0, 0, 0},
	{9, -1, 0, 0, 0},
	{10, -1, 0, 0, 0},
	{-1, 0, 0, 0, 0},
	{-1, 0, 0, 0, 0},
	{13, -1, 0, 0, 0},
	{14, -1, 0, 0, 0},
	{-1, 0, 0, 0, 0}
};

//**************************************************************************************************
inline	void	uncompress_matrix(CMatrix33 *cm, Matrix33 *m)
{
	SLONG v;

	v=((cm->M[0]&CMAT0_MASK)<<2)>>22;
	m->M[0][0]=(v<<6);

	v=((cm->M[0]&CMAT1_MASK)<<12)>>22;
	m->M[0][1]=(v<<6);

	v=((cm->M[0]&CMAT2_MASK)<<22)>>22;
	m->M[0][2]=(v<<6);

	v=((cm->M[1]&CMAT0_MASK)<<2)>>22;
	m->M[1][0]=(v<<6);

	v=((cm->M[1]&CMAT1_MASK)<<12)>>22;
	m->M[1][1]=(v<<6);

	v=((cm->M[1]&CMAT2_MASK)<<22)>>22;
	m->M[1][2]=(v<<6);

	v=((cm->M[2]&CMAT0_MASK)<<2)>>22;
	m->M[2][0]=(v<<6);

	v=((cm->M[2]&CMAT1_MASK)<<12)>>22;
	m->M[2][1]=(v<<6);

	v=((cm->M[2]&CMAT2_MASK)<<22)>>22;
	m->M[2][2]=(v<<6);
}

//**************************************************************************************************
// utility function to calculate the position offset of a body part given various bits of information
// about its parent...

void	HIERARCHY_Get_Body_Part_Offset( Matrix31 *dest_position, Matrix31 *base_position,
									   CMatrix33 *parent_base_matrix, Matrix31 *parent_base_position,
									    Matrix33 *parent_curr_matrix, Matrix31 *parent_curr_position)
{
	// build the partial matrix
	struct Matrix33 pmatq, pmati, cmati;

	// uncompress the base matrix.
	uncompress_matrix(parent_base_matrix, &pmati);
	pmatq = *parent_curr_matrix;

	// transpose the base matrix
	SWAP(pmati.M[0][1], pmati.M[1][0]);
	SWAP(pmati.M[0][2], pmati.M[2][0]);
	SWAP(pmati.M[2][1], pmati.M[1][2]);

	struct Matrix31	i, o;
	o.M[0] = (base_position->M[0] - parent_base_position->M[0]) * 256;   // increase accuracy
	o.M[1] = (base_position->M[1] - parent_base_position->M[1]) * 256;
	o.M[2] = (base_position->M[2] - parent_base_position->M[2]) * 256;

	// transpose by the inverse of the original parent matrix
	matrix_transformZMY(&i     , &pmati, &o);

	// and transpose back by the new quaternion interpolated matrix.
	matrix_transformZMY(dest_position, &pmatq, &i);

	// and add back to original position.
	dest_position->M[0] += parent_curr_position->M[0];
	dest_position->M[1] += parent_curr_position->M[1];
	dest_position->M[2] += parent_curr_position->M[2];

}

