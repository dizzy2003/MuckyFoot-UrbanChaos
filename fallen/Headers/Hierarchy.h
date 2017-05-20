#ifndef HIERARCHY_H
#define HIERARCHY_H

// JCL - functions and data managing the hierarchy of body parts in people

extern	CBYTE	*body_part_parent[][2];
extern	SLONG	body_part_parent_numbers[];
extern  SLONG	body_part_children[][5];

void	HIERARCHY_Get_Body_Part_Offset(Matrix31 *dest_position, Matrix31 *base_position,
									   CMatrix33 *parent_base_matrix, Matrix31 *parent_base_position,
									    Matrix33 *parent_curr_matrix, Matrix31 *parent_curr_position);

#endif
