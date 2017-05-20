//
// The virtual machine to run our basic programs.
//

#include "always.h"
#include "console.h"
#include "key.h"
#include "mem.h"
#include "ml.h"
#include "sysvar.h"



//
// Instruction memory.
//

SLONG *VM_code;
SLONG  VM_code_max;
SLONG  VM_code_upto;
SLONG *VM_code_pointer;		// The instruction we are executing...



//
// The stack.
//

ML_Data *VM_stack;
SLONG    VM_stack_max;
ML_Data *VM_stack_top;		// The top of the stack
ML_Data *VM_stack_base;		// The current stack frame


//
// The globals.
//

ML_Data *VM_global;
SLONG    VM_global_max;


//
// The static data table.
//

UBYTE *VM_data;
SLONG  VM_data_max;




//
// Extra globals we add onto the end of the program.
//

#define VM_EXTRA_GLOBAL_KEY(key) (VM_global_extra + (key))
#define VM_EXTRA_GLOBAL_INKEY    (VM_global_extra + 256)
#define VM_EXTRA_GLOBAL_NUMBER   257

SLONG VM_global_extra;		// The index of the first extra global.



//
// OS_ticks() the last time we did a flip.
//

SLONG VM_flip_tick;


//
// Checks for sufficient stack space to push on another
// item of data.
//

#define VM_CHECK_STACK_PUSH() {ASSERT(WITHIN(VM_stack_top, VM_stack, VM_stack + VM_stack_max - 1));}

//
// Pops two items off the stack (just decreases the stack pointer!)
//

#define VM_POP_STACK(n) {VM_stack_top -= (n); ASSERT(WITHIN(VM_stack_top, VM_stack, VM_stack + VM_stack_max - 1));}






//
// Frees memory used by the given bit of data.
//

void VM_data_free(ML_Data md)
{
	SLONG i;

	switch(md.type)
	{
		case ML_TYPE_STRVAR:
			MEM_free(md.strvar);
			break;

		case ML_TYPE_STRUCTURE:
			
			//
			// Free all the fields.
			//

			for (i = 0; i < md.structure->num_fields; i++)
			{
				VM_data_free(md.structure->field[i].data);
			}

			//
			// Now free the structure.
			//

			MEM_free(md.structure);

			break;

		case ML_TYPE_ARRAY:
			
			//
			// Free all the elements of the array.
			//

			for (i = 0; i < md.array->length; i++)
			{
				VM_data_free(md.array->data[i]);
			}

			MEM_free(md.array->data);

			//
			// Now free the array.
			//

			MEM_free(md.array);

			break;

		case ML_TYPE_TEXTURE:

			md.lt->ref_count -= 1;

			if (md.lt->ref_count == 0)
			{
				//
				// Free the texture.
				//

				LL_free_texture(md.lt);
			}

			break;


		case ML_TYPE_BUFFER:

			md.lb->ref_count -= 1;

			if (md.lb->ref_count == 0)
			{
				//
				// Free the buffer.
				//

				LL_free_buffer(md.lb);
			}

			break;

		case ML_TYPE_MATRIX:
			MEM_free(md.matrix);
			break;
		
		case ML_TYPE_VECTOR:
			MEM_free(md.vector);
			break;

		default:
			break;
	}
}


//
// Creates a copy of the given bit of data.
//

ML_Data VM_data_copy(ML_Data original)
{
	ML_Data ans;

	switch(original.type)
	{
		//
		// Types that need extra allocation.
		//

		case ML_TYPE_STRVAR:

			{
				SLONG length = MEM_block_size(original.strvar);

				ans.type   = ML_TYPE_STRVAR;
				ans.strvar = (CBYTE *) MEM_alloc(length);

				memcpy(ans.strvar, original.strvar, length);
			}

			return ans;

		case ML_TYPE_STRUCTURE:

			{
				//
				// Create another copy of the structure.
				//

				SLONG length = sizeof(ML_Structure) + original.structure->num_fields * sizeof(ML_Field);

				ans.type      = ML_TYPE_STRUCTURE;
				ans.structure = (ML_Structure *) MEM_alloc(length);

				SLONG i;

				ans.structure->num_fields = original.structure->num_fields;

				for (i = 0; i < ans.structure->num_fields; i++)
				{
					ans.structure->field[i].field_id = original.structure->field[i].field_id;
					ans.structure->field[i].data     = VM_data_copy(original.structure->field[i].data);
				}
			}

			return ans;

		case ML_TYPE_ARRAY:

			{
				//
				// Create another copy of the array.
				//

				SLONG length = sizeof(ML_Array) + original.array->num_dimensions * sizeof(ML_Dimension);

				ans.type  = ML_TYPE_ARRAY;
				ans.array = (ML_Array *) MEM_alloc(length);

				ans.array->data           = (ML_Data *) MEM_alloc(sizeof(ML_Data) * original.array->length);
				ans.array->length         = original.array->length;
				ans.array->num_dimensions = original.array->num_dimensions;

				SLONG i;

				for (i = 0; i < original.array->num_dimensions; i++)
				{
					ans.array->dimension[i] = original.array->dimension[i];
				}

				for (i = 0; i < original.array->length; i++)
				{
					ans.array->data[i] = VM_data_copy(original.array->data[i]);
				}
			}

			return ans;

		case ML_TYPE_TEXTURE:

			//
			// Increase the reference count of the texture.
			//

			original.lt->ref_count += 1;

			return original;

		case ML_TYPE_BUFFER:

			//
			// Increase the reference count of the buffer.
			//

			original.lb->ref_count += 1;

			return original;

		case ML_TYPE_POINTER:

			//
			// Push on the derefenced value.
			//

			return VM_data_copy(*original.data);

		case ML_TYPE_MATRIX:

			ans.type    =  ML_TYPE_MATRIX;
			ans.matrix  =  (ML_Matrix *) MEM_alloc(sizeof(ML_Matrix));
		  *(ans.matrix) = *original.matrix;

			return ans;

		case ML_TYPE_VECTOR:

			ans.type    =  ML_TYPE_VECTOR;
			ans.vector  =  (ML_Vector *) MEM_alloc(sizeof(ML_Vector));
		  *(ans.vector) = *original.vector;

			return ans;

		//
		// Simple types.
		//

		default:
			return original;
	}
}





//
// Convert the given bit of data to a string. If the input is a
// string variable or string constant then the original is returned,
// not a copy.
//

void VM_convert_to_string(ML_Data *original)
{
	ML_Data ans;

	ans.type = ML_TYPE_STRVAR;

	switch(original->type)
	{
		case ML_TYPE_UNDEFINED:
			ans.strvar = (CBYTE *) MEM_alloc(16);	// Enough to hold the string "<UNDEFINED>"
			memcpy(ans.strvar, "<UNDEFINED>", 12);
			break;

		case ML_TYPE_SLUMBER:
			ans.strvar = (CBYTE *) MEM_alloc(16);	// Enough to hold the number -2 ^ 32 and a NULL.
			sprintf(ans.strvar, "%d", original->slumber);
			break;

		case ML_TYPE_FLUMBER:
			ans.strvar = (CBYTE *) MEM_alloc(32);	// Enough to hold the number -2 ^ 32 and a NULL.
			sprintf(ans.strvar, "%f", original->flumber);
			break;

		case ML_TYPE_STRCONST:
		case ML_TYPE_STRVAR:

			//
			// Should we be calling this function?
			//

			return;

		case ML_TYPE_BOOLEAN:

			ans.strvar = (CBYTE *) MEM_alloc(6);	// Enough to hold the string "TRUE" or "FALSE"

			if (original->boolean)
			{
				memcpy(ans.strvar, "TRUE", 5);
			}
			else
			{
				memcpy(ans.strvar, "FALSE", 6);
			}

			break;

		default:
			ASSERT(0);
			break;
	}

   *original = ans;	

	return;
}

//
// Returns the string held by the given string variable.
//

CBYTE *VM_get_string(ML_Data string)
{
	switch(string.type)
	{
		case ML_TYPE_STRCONST:
			ASSERT(WITHIN(string.strconst, 0, VM_data_max - 2));
			return (CBYTE *) (VM_data + string.strconst);

		case ML_TYPE_STRVAR:
			return string.strvar;

		default:
			ASSERT(0);
			return NULL;
	}
}


//
// Makes VM_stack_top[0] and VM_stack_top[1] have the same type.
// VM_stack_top[0] must be a FLUMBER or a SLUMBER.
//

void VM_convert_stack_top_to_same_numerical_type()
{
	switch(VM_stack_top[0].type)
	{
		case ML_TYPE_SLUMBER:

			if (VM_stack_top[1].type == ML_TYPE_FLUMBER)
			{
				//
				// Convert first argument to a float.
				//

				VM_stack_top[0].type    = ML_TYPE_FLUMBER;
				VM_stack_top[0].flumber = float(VM_stack_top[0].slumber);
			}

			break;

		case ML_TYPE_FLUMBER:

			if (VM_stack_top[1].type == ML_TYPE_SLUMBER)
			{
				//
				// Convert second argument to a float.
				//

				VM_stack_top[1].type    = ML_TYPE_FLUMBER;
				VM_stack_top[1].flumber = float(VM_stack_top[1].slumber);
			}

			break;

		default:
			ASSERT(0);
			break;
	}
}



//
// Makes sure the array is big enough to for the indices
// given in the md[] array (an array of SLUMBERs)
//

void VM_grow_array(ML_Array *ma, ML_Data *md)
{
	SLONG i;
	SLONG j;
	SLONG bigger_index;
	SLONG bigger_length;

	#define VM_MAX_DIMENSIONS 64

	ASSERT(WITHIN(ma->num_dimensions, 1, VM_MAX_DIMENSIONS));

	SLONG bigger_size  [VM_MAX_DIMENSIONS];
	SLONG bigger_stride[VM_MAX_DIMENSIONS];
	SLONG index        [VM_MAX_DIMENSIONS];

	//
	// Build the array of the new sizes of each dimension.
	//

	for (i = 0; i < ma->num_dimensions; i++)
	{
		if (md[i].slumber > ma->dimension[i].size)
		{
			//
			// Double the dimension of the array.
			//

			bigger_size[i] = ma->dimension[i].size * 2;

			if (md[i].slumber > bigger_size[i])
			{
				//
				// Doubling wasn't good enough! Make it as big as need be.
				//

				bigger_size[i] = md[i].slumber;
			}
		}
		else
		{
			bigger_size[i] = ma->dimension[i].size;
		}
	}

	//
	// The new stride in each dimension.
	//

	for (i = 0; i < ma->num_dimensions; i++)
	{
		bigger_stride[i] = 1;

		for (j = i + 1; j < ma->num_dimensions; j++)
		{
			bigger_stride[i] *= bigger_size[j];
		}
	}
	
	//
	// How long is the new array?
	//

	bigger_length = bigger_size[0];

	for (i = 1; i < ma->num_dimensions; i++)
	{
		bigger_length *= bigger_size[1];
	}

	//
	// Allocate a new area of memory.
	//

	ML_Data *bigger_data = (ML_Data *) MEM_alloc(sizeof(ML_Data) * bigger_length);

	if (ma->num_dimensions == 1)
	{
		//
		// Easy to copy over data...
		//

		memcpy(bigger_data,  ma->data, sizeof(ML_Data) * ma->length);
		memset(bigger_data + ma->length, 0, sizeof(ML_Data) * (bigger_length - ma->length));
	}
	else
	{
		//
		// Nightmare to copy over data. Initialise new memory to <UNDEFINED>
		//

 		memset(bigger_data, 0, sizeof(ML_Data) * bigger_length);

		//
		// Clear the index array.
		//

		for (i = 0; i < ma->num_dimensions; i++)
		{
			index[i] = 0;
		}

		//
		// Copy over all old data in a horribly inefficient manner.
		//

		for (i = 0; i < ma->length; i++)
		{
			//
			// What's the new index of this element?
			//

			bigger_index = 0;

			for (j = 0; j < ma->num_dimensions; j++)
			{
				bigger_index += bigger_stride[j] * index[j];
			}

			//
			// Copy over this element.
			//

			bigger_data[bigger_index] = ma->data[i];

			//
			// Update our indices...
			//

			for (j = ma->num_dimensions - 1; j >= 1; j--)
			{
				index[j] += 1;

				if (index[j] >= ma->dimension[j].size)
				{
					//
					// Roll-over.
					//

					index[j    ]  = 0;
					index[j - 1] += 1;
				}
			}
		}
	}

	//
	// Free old memory.
	//

	MEM_free(ma->data);

	ma->data   = bigger_data;
	ma->length = bigger_length;
	
	for (i = 0; i < ma->num_dimensions; i++)
	{
		ma->dimension[i].size   = bigger_size[i];
		ma->dimension[i].stride = bigger_stride[i];
	}
}


//
// Complicated instructions that needn't be fast
// get their own function.
//


void VM_do_texture()
{
	VM_POP_STACK(2);

	//
	// We don't support USER textures now, so the
	// first argument must be the filename and the
	// second should be UNDEFINED.
	// 

	if (VM_stack_top[0].type != ML_TYPE_STRCONST &&
		VM_stack_top[0].type != ML_TYPE_STRVAR)
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	if (VM_stack_top[1].type != ML_TYPE_UNDEFINED)
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}
	
	//
	// Create the texture.
	//

	ML_Data texture;

	texture.type = ML_TYPE_TEXTURE;
	texture.lt   = LL_create_texture(VM_get_string(VM_stack_top[0]));

	//
	// Free the arguments.
	//

	VM_data_free(VM_stack_top[0]);
	VM_data_free(VM_stack_top[1]);

	VM_stack_top[0] = texture;

	VM_stack_top += 1;
}

void VM_do_buffer()
{
	SLONG i;
	SLONG j;
	SLONG num_verts;
	SLONG num_indices;
	ULONG found;
	SLONG slumber;
	float flumber;

	ML_Data      *vert_array;
	ML_Structure *vert_struct;
	ML_Field     *field;
	ML_Data      *index_array;
	LL_Tlvert    *tl;
	UWORD        *index;

	VM_POP_STACK(4);

	//
	// The first argument should be an 1d array of structures!
	//

	if (VM_stack_top[0].type != ML_TYPE_ARRAY)
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}
	else
	if (VM_stack_top[0].array->num_dimensions != 1)
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	//
	// The next argument should be the number of vertices.
	//

	if (VM_stack_top[1].type == ML_TYPE_SLUMBER)
	{
		num_verts = VM_stack_top[1].slumber;
	}
	else
	if (VM_stack_top[1].type == ML_TYPE_FLUMBER)
	{
		float integer;
		float fraction;

		fraction = modff(VM_stack_top[1].flumber, &integer);

		if (fraction != 0.0F)
		{
			//
			// ERROR! A fractional number of verts?
			//

			ASSERT(0);
		}

		num_verts = ftol(integer);
	}

	//
	// The next two arguments are optional depending on whether
	// this is a triangle list or an indexed list.
	//

	if (VM_stack_top[3].type == ML_TYPE_UNDEFINED)
	{
		//
		// Ok... just means no indices.
		//

		num_indices = 0;
	}
	else
	if (VM_stack_top[3].type == ML_TYPE_SLUMBER)
	{
		num_indices = VM_stack_top[3].slumber;
	}
	else
	if (VM_stack_top[3].type == ML_TYPE_FLUMBER)
	{
		float integer;
		float fraction;

		fraction = modff(VM_stack_top[1].flumber, &integer);

		if (fraction != 0.0F)
		{
			//
			// ERROR! Oh dear! A fractional number of indices!
			//

			ASSERT(0);
		}

		num_indices = ftol(integer);
	}
	else
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	if (VM_stack_top[2].type == ML_TYPE_ARRAY)
	{
		//
		// Should be an 1d array of numbers.
		//

		if (VM_stack_top[2].array->num_dimensions != 1)
		{
			//
			// ERROR!
			//

			ASSERT(0);
		}

		if (num_indices == 0 || num_indices % 3 != 0)
		{
			//
			// ERROR!
			//

			ASSERT(0);
		}
	}
	else
	if (VM_stack_top[2].type == ML_TYPE_UNDEFINED)
	{
		//
		// Can't have indices without an array.
		//

		if (num_indices != 0)
		{
			//
			// ERROR!
			//

			ASSERT(0);
		}
	}

	//
	// Is our vert array big enough?
	//

	if (num_verts > VM_stack_top[0].array->length)
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	//
	// Create the vert buffer.
	//

	tl = (LL_Tlvert *) MEM_alloc(num_verts * sizeof(LL_Tlvert));

	vert_array = VM_stack_top[0].array->data;

	for (i = 0; i < num_verts; i++)
	{
		if (vert_array[i].type != ML_TYPE_STRUCTURE)
		{
			//
			// ERROR!
			//

			ASSERT(0);
		}

		vert_struct = vert_array[i].structure;

		//
		// Extract the various components of the LL_Tlvert.
		//

		found = 0;

		for (j = 0; j < vert_struct->num_fields; j++)
		{
			field = &vert_struct->field[j];

			if (field->field_id < SYSVAR_FIELD_NUMBER)
			{
				//
				// This is a system field so it could be part of our
				// Tlvert structure. Type check the data if it is.
				//

				switch(field->field_id)
				{
					case SYSVAR_FIELD_X:
					case SYSVAR_FIELD_Y:
					case SYSVAR_FIELD_U:
					case SYSVAR_FIELD_V:
						
						//
						// Must be numbers.
						//

						if (field->data.type == ML_TYPE_SLUMBER)
						{
							flumber = float(field->data.slumber);
						}
						else
						if (field->data.type == ML_TYPE_FLUMBER)
						{
							flumber = field->data.flumber;
						}
						else
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						break;


					case SYSVAR_FIELD_Z:
					case SYSVAR_FIELD_RHW:

						//
						// Must be floating point numbers between 0 and 1.
						//

						if (field->data.type == ML_TYPE_SLUMBER)
						{
							flumber = float(field->data.slumber);
						}
						else
						if (field->data.type == ML_TYPE_FLUMBER)
						{
							flumber = field->data.flumber;
						}
						else
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						if (!WITHIN(flumber, 0.0F, 1.0F))
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						if (field->field_id == SYSVAR_FIELD_RHW)
						{
							//
							// If RHW is too small it fucks up... in general!
							//

							if (flumber < 1.0F / 1024.0F)
							{
								flumber = 1.0F / 1024.0F;
							}
						}

						break;

					case SYSVAR_FIELD_COLOUR:
					case SYSVAR_FIELD_SPECULAR:

						//
						// Must be numbers... or maybe string colours?!
						//

						if (field->data.type == ML_TYPE_SLUMBER)
						{
							slumber = field->data.slumber;
						}
						else
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						break;

					default:

						//
						// Not a field we are interested in.
						//

						continue;
				}

				//
				// This is one of our fields.
				//

				found |= 1 << field->field_id;

				switch(field->field_id)
				{
					case SYSVAR_FIELD_X:        tl[i].x        = flumber; break;
					case SYSVAR_FIELD_Y:		tl[i].y        = flumber; break;
					case SYSVAR_FIELD_Z:		tl[i].z        = flumber; break;
					case SYSVAR_FIELD_RHW:		tl[i].x        = flumber; break;
					case SYSVAR_FIELD_U:		tl[i].u        = flumber; break;
					case SYSVAR_FIELD_V:		tl[i].v        = flumber; break;
					case SYSVAR_FIELD_COLOUR:   tl[i].colour   = slumber; break;
					case SYSVAR_FIELD_SPECULAR: tl[i].specular = slumber; break;

					default:
						ASSERT(0);
						break;
				}
			}
		}

		//
		// Fill in missing fields with default values.
		//

		if (!(found & (1 << SYSVAR_FIELD_Z       ))) {tl[i].z        = 0.0F;       found |= 1 << SYSVAR_FIELD_Z;       }
		if (!(found & (1 << SYSVAR_FIELD_RHW     ))) {tl[i].rhw      = 0.5F;       found |= 1 << SYSVAR_FIELD_RHW;     }
		if (!(found & (1 << SYSVAR_FIELD_U       ))) {tl[i].u        = 0.0F;       found |= 1 << SYSVAR_FIELD_U;       }
		if (!(found & (1 << SYSVAR_FIELD_V       ))) {tl[i].v        = 0.0F;       found |= 1 << SYSVAR_FIELD_V;       }
		if (!(found & (1 << SYSVAR_FIELD_COLOUR  ))) {tl[i].colour   = 0xffffffff; found |= 1 << SYSVAR_FIELD_COLOUR;  }
		if (!(found & (1 << SYSVAR_FIELD_SPECULAR))) {tl[i].specular = 0xff000000; found |= 1 << SYSVAR_FIELD_SPECULAR;}

		//
		// Make sure we have initialised every member.
		//

		const SLONG found_every_tlvert_field =
						(1 << SYSVAR_FIELD_X       ) |
						(1 << SYSVAR_FIELD_Y       ) |
						(1 << SYSVAR_FIELD_Z       ) |
						(1 << SYSVAR_FIELD_RHW     ) |
						(1 << SYSVAR_FIELD_U       ) |
						(1 << SYSVAR_FIELD_V       ) |
						(1 << SYSVAR_FIELD_COLOUR  ) |
						(1 << SYSVAR_FIELD_SPECULAR);

		if (found != found_every_tlvert_field)
		{
			//
			// ERROR!
			//

			ASSERT(0);
		}
	}

	//
	// Now create the index buffer.
	//

	if (num_indices == 0)
	{
		index = NULL;
	}
	else
	{
		//
		// Is our index array big enough?
		//

		if (num_indices > VM_stack_top[2].array->length)
		{
			//
			// ERROR!
			//

			ASSERT(0);
		}

		index       = (UWORD *) MEM_alloc(num_indices * sizeof(UWORD));
		index_array = VM_stack_top[2].array->data;

		for (i = 0; i < num_indices; i++)
		{
			if (index_array[i].type == ML_TYPE_SLUMBER)
			{
				slumber = index_array[i].slumber;
			}
			else
			if (index_array[i].type == ML_TYPE_FLUMBER)
			{
				float integer;
				float fraction;

				fraction = modff(index_array[i].flumber, &integer);

				if (fraction != 0.0F)
				{
					//
					// ERROR! Oh dear! A fractional number of indices!
					//

					ASSERT(0);
				}

				slumber = ftol(integer);
			}
			else
			{
				//
				// ERROR!
				//

				ASSERT(0);
			}

			if (!WITHIN(slumber, 1, num_verts))
			{
				//
				// ERROR!
				//

				ASSERT(0);
			}

			//
			// Convert from 1-based indexing to 0-based indexing.
			// 

			index[i] = slumber - 1;
		}
	}

	//
	// Free the arguments.
	//

	VM_data_free(VM_stack_top[0]);
	VM_data_free(VM_stack_top[1]);
	VM_data_free(VM_stack_top[2]);
	VM_data_free(VM_stack_top[3]);

	VM_stack_top[0].type = ML_TYPE_BUFFER;
	VM_stack_top[0].lb   = LL_create_buffer(
								LL_BUFFER_TYPE_TLV,
								tl,
								num_verts,
								index,
								num_indices);

	VM_stack_top += 1;
}


void VM_do_draw()
{
	VM_POP_STACK(3);

	LL_Buffer  *lb;
	LL_Texture *lt;
	ULONG       rs;

	//
	// We should have a buffer then a texture and a renderstate.
	//

	if (VM_stack_top[0].type == ML_TYPE_BUFFER)
	{
		lb = VM_stack_top[0].lb;
	}
	else
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	if (VM_stack_top[1].type == ML_TYPE_TEXTURE)
	{
		lt = VM_stack_top[1].lt;
	}
	else
	if (VM_stack_top[1].type == ML_TYPE_UNDEFINED)
	{
		//
		// Undefined => draw untextured.
		//

		lt = NULL;
	}
	else
	{
		//
		// ERROR!
		//
		
		ASSERT(0);
	}

	if (VM_stack_top[2].type == ML_TYPE_SLUMBER)
	{
		rs = VM_stack_top[2].slumber;
	}
	else
	if (VM_stack_top[2].type == ML_TYPE_UNDEFINED)
	{
		//
		// Default renderstate.
		//

		rs = LL_RS_NORMAL;
	}
	else
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	//
	// Do the draw!
	//

	LL_draw_buffer(lb,lt,rs);

	//
	// Free the arguments.
	//

	VM_data_free(VM_stack_top[0]);
	VM_data_free(VM_stack_top[1]);
	VM_data_free(VM_stack_top[2]);
}


void VM_do_cls()
{
	ULONG colour;
	float zsort;

	VM_POP_STACK(2);

	//
	// The colour.
	//

	if (VM_stack_top[0].type == ML_TYPE_SLUMBER)
	{
		colour = VM_stack_top[0].slumber;
	}
	else
	if (VM_stack_top[0].type == ML_TYPE_UNDEFINED)
	{
		colour = 0;
	}
	else
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	//
	// The z-value.
	//

	if (VM_stack_top[1].type == ML_TYPE_SLUMBER)
	{
		zsort = float(VM_stack_top[1].slumber);
	}
	else
	if (VM_stack_top[1].type == ML_TYPE_FLUMBER)
	{
		zsort = VM_stack_top[1].flumber;
	}
	else
	if (VM_stack_top[1].type == ML_TYPE_UNDEFINED)
	{	
		zsort = 1.0F;
	}

	if (!WITHIN(zsort, 0.0F, 1.0F))
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	LL_cls(colour, zsort);
}




//
// Executes the program!
//

void VM_execute()
{
	while(1)
	{
		ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));

		switch(*VM_code_pointer++)
		{
			case ML_DO_PUSH_CONSTANT:

				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN(VM_code_pointer + 1, VM_code, VM_code + VM_code_upto - 1));

				VM_stack_top->type  = *VM_code_pointer++;
				VM_stack_top->value = *VM_code_pointer++;

				VM_stack_top += 1;

				break;

			case ML_DO_ADD:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// Must do type conversion...
					//

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_BOOLEAN:

							if (VM_stack_top[1].type == ML_TYPE_STRVAR ||
								VM_stack_top[1].type == ML_TYPE_STRCONST)
							{
								//
								// Convert first argument to a string.
								//

								VM_convert_to_string(&VM_stack_top[0]);
							}
							else
							{
								//
								// ERROR!
								//

								ASSERT(0);
							}

							break;

						case ML_TYPE_SLUMBER:

							if (VM_stack_top[1].type == ML_TYPE_FLUMBER)
							{
								//
								// Convert first argument to a float.
								//

								VM_stack_top[0].type    = ML_TYPE_FLUMBER;
								VM_stack_top[0].flumber = float(VM_stack_top[0].slumber);
							}
							else
							if (VM_stack_top[1].type == ML_TYPE_STRVAR ||
								VM_stack_top[1].type == ML_TYPE_STRCONST)
							{
								//
								// Convert first argument to a string.
								//

								VM_convert_to_string(&VM_stack_top[0]);
							}
							else
							{
								//
								// ERROR!
								//

								ASSERT(0);
							}

							break;

						case ML_TYPE_FLUMBER:

							if (VM_stack_top[1].type == ML_TYPE_SLUMBER)
							{
								//
								// Convert second argument to a float.
								//

								VM_stack_top[1].type    = ML_TYPE_FLUMBER;
								VM_stack_top[1].flumber = float(VM_stack_top[1].slumber);
							}
							else
							if (VM_stack_top[1].type == ML_TYPE_STRVAR ||
								VM_stack_top[1].type == ML_TYPE_STRCONST)
							{
								//
								// Convert first argument to a string.
								//

								VM_convert_to_string(&VM_stack_top[0]);
							}
							else
							{
								//
								// ERROR!
								//

								ASSERT(0);
							}

							break;

						case ML_TYPE_STRVAR:
						case ML_TYPE_STRCONST:

							//
							// Converts to a string representing the data.
							//

							VM_convert_to_string(&VM_stack_top[1]);

							break;

						default:

							//
							// ERROR!
							//

							ASSERT(0);

							break;
					}
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER:
						VM_stack_top[0].slumber = VM_stack_top[0].slumber + VM_stack_top[1].slumber;
						break;

					case ML_TYPE_FLUMBER:
						VM_stack_top[0].flumber = VM_stack_top[0].flumber + VM_stack_top[1].flumber;
						break;

					case ML_TYPE_STRVAR:
					case ML_TYPE_STRCONST:

						{
							ML_Data result;
							SLONG   length;
							CBYTE  *str1;
							CBYTE  *str2;
							

							//
							// How long is the resulting string?
							//

							str1 = VM_get_string(VM_stack_top[0]);
							str2 = VM_get_string(VM_stack_top[1]);

							length = strlen(str1) + strlen(str2) + 1;	// + 1 for the terminating NULL.

							//
							// Build the result.
							//

							result.type   = ML_TYPE_STRVAR;
							result.strvar = (CBYTE *) MEM_alloc(length);

							strcpy(result.strvar, str1);
							strcat(result.strvar, str2);

							//
							// Free memory.
							//

							VM_data_free(VM_stack_top[0]);
							VM_data_free(VM_stack_top[1]);

							//
							// Put new data on the stack.
							//

							VM_stack_top[0] = result;
						}

						break;

					case ML_TYPE_VECTOR:

						VM_stack_top[0].vector->x += VM_stack_top[1].vector->x;
						VM_stack_top[0].vector->y += VM_stack_top[1].vector->y;
						VM_stack_top[0].vector->z += VM_stack_top[1].vector->z;

						VM_data_free(VM_stack_top[1]);

						break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_PRINT:

				VM_POP_STACK(1);

				VM_convert_to_string(&VM_stack_top[0]);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_STRVAR:

						CONSOLE_print(VM_stack_top[0].strvar);

						VM_data_free(VM_stack_top[0]);

						break;

					case ML_TYPE_STRCONST:

						ASSERT(WITHIN(VM_stack_top[0].strconst, 0, VM_data_max - 2));

						CONSOLE_print((CBYTE *) (VM_data + VM_stack_top[0].strconst));

						break;

					default:
						ASSERT(0);
						break;
				}

				break;

			case ML_DO_EXIT:
				
				//
				// Free memory.
				//

				{
					SLONG i;

					for (i = 0; i < VM_global_max; i++)
					{
						VM_data_free(VM_global[i]);
					}

					ASSERT(MEM_total_bytes_allocated() == 0);
				}

				return;

			case ML_DO_GOTO:

				//
				// Valid address?
				//

				ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(VM_code_pointer[0], 0, VM_code_upto - 1));

				VM_code_pointer = &VM_code[VM_code_pointer[0]];

				break;
			
			case ML_DO_UMINUS:
				
				VM_POP_STACK(1);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER:
						VM_stack_top[0].slumber = -VM_stack_top[0].slumber;
						break;

					case ML_TYPE_FLUMBER:
						VM_stack_top[0].flumber = -VM_stack_top[0].flumber;
						break;

					case ML_TYPE_VECTOR:
						VM_stack_top[0].vector->x = -VM_stack_top[0].vector->x;
						VM_stack_top[0].vector->y = -VM_stack_top[0].vector->y;
						VM_stack_top[0].vector->z = -VM_stack_top[0].vector->z;
						break;

					default:

						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_PUSH_GLOBAL_VALUE:
				
				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(VM_code_pointer[0], 0, VM_global_max - 1));

				//
				// Push a copy of the global onto the stack. Any memory allocated
				// by the original will be duplicated.
				//

			   *VM_stack_top++ = VM_data_copy(VM_global[*VM_code_pointer++]);

				break;

			case ML_DO_PUSH_GLOBAL_QUICK:
				
				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(VM_code_pointer[0], 0, VM_global_max - 1));

				//
				// Push a copy of the global onto the stack - don't copy data.
				//

			   *VM_stack_top++ = VM_global[*VM_code_pointer++];

				break;

			case ML_DO_MINUS:
				
				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// Must do type conversion...
					//

					VM_convert_stack_top_to_same_numerical_type();
				}

				ASSERT(VM_stack_top[0].type == VM_stack_top[1].type);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER:
						VM_stack_top[0].slumber = VM_stack_top[0].slumber - VM_stack_top[1].slumber;
						break;

					case ML_TYPE_FLUMBER:
						VM_stack_top[0].flumber = VM_stack_top[0].flumber - VM_stack_top[1].flumber;
						break;

					case ML_TYPE_VECTOR:

						VM_stack_top[0].vector->x -= VM_stack_top[1].vector->x;
						VM_stack_top[0].vector->y -= VM_stack_top[1].vector->y;
						VM_stack_top[0].vector->z -= VM_stack_top[1].vector->z;

						VM_data_free(VM_stack_top[1]);

						break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_TIMES:
				
				VM_POP_STACK(2);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER:

						switch(VM_stack_top[1].type)
						{
							case ML_TYPE_SLUMBER:
								VM_stack_top[0].slumber *= VM_stack_top[1].slumber;
								break;

							case ML_TYPE_FLUMBER:

								//
								// Answer becomes a float.
								//

								VM_stack_top[0].type    = ML_TYPE_FLUMBER;
								VM_stack_top[0].flumber = float(VM_stack_top[0].slumber) * VM_stack_top[1].flumber;

								break;

							case ML_TYPE_VECTOR:

								{	
									float fmul = float(VM_stack_top[0].slumber);

									VM_stack_top[0] = VM_stack_top[1];

									VM_stack_top[0].vector->x *= fmul;
									VM_stack_top[0].vector->y *= fmul;
									VM_stack_top[0].vector->z *= fmul;
								}

								break;

							case ML_TYPE_MATRIX:

								{	
									float fmul = float(VM_stack_top[0].slumber);

									VM_stack_top[0] = VM_stack_top[1];

									VM_stack_top[0].matrix->vector[0].x *= fmul;
									VM_stack_top[0].matrix->vector[0].y *= fmul;
									VM_stack_top[0].matrix->vector[0].z *= fmul;

									VM_stack_top[0].matrix->vector[1].x *= fmul;
									VM_stack_top[0].matrix->vector[1].y *= fmul;
									VM_stack_top[0].matrix->vector[1].z *= fmul;

									VM_stack_top[0].matrix->vector[2].x *= fmul;
									VM_stack_top[0].matrix->vector[2].y *= fmul;
									VM_stack_top[0].matrix->vector[2].z *= fmul;
								}

								break;

							default:

								//
								// ERROR!
								//

								ASSERT(0);

								break;
						}

						break;

					case ML_TYPE_FLUMBER:

						switch(VM_stack_top[1].type)
						{
							case ML_TYPE_SLUMBER:
								VM_stack_top[0].flumber *= (float) VM_stack_top[1].slumber;
								break;

							case ML_TYPE_FLUMBER:
								VM_stack_top[0].flumber *= VM_stack_top[1].flumber;

								break;

							case ML_TYPE_VECTOR:

								{	
									float fmul = VM_stack_top[0].flumber;

									VM_stack_top[0] = VM_stack_top[1];

									VM_stack_top[0].vector->x *= fmul;
									VM_stack_top[0].vector->y *= fmul;
									VM_stack_top[0].vector->z *= fmul;
								}

								break;

							case ML_TYPE_MATRIX:

								{	
									float fmul = VM_stack_top[0].flumber;

									VM_stack_top[0] = VM_stack_top[1];

									VM_stack_top[0].matrix->vector[0].x *= fmul;
									VM_stack_top[0].matrix->vector[0].y *= fmul;
									VM_stack_top[0].matrix->vector[0].z *= fmul;

									VM_stack_top[0].matrix->vector[1].x *= fmul;
									VM_stack_top[0].matrix->vector[1].y *= fmul;
									VM_stack_top[0].matrix->vector[1].z *= fmul;

									VM_stack_top[0].matrix->vector[2].x *= fmul;
									VM_stack_top[0].matrix->vector[2].y *= fmul;
									VM_stack_top[0].matrix->vector[2].z *= fmul;
								}

								break;

							default:
								
								//
								// ERROR!
								//

								ASSERT(0);

								break;
						}

						break;

					case ML_TYPE_MATRIX:

						switch(VM_stack_top[1].type)
						{
							case ML_TYPE_MATRIX:

								//
								// Matrix multiplication.
								//

								{
									ML_Matrix  ans;
									ML_Matrix *a = VM_stack_top[0].matrix;
									ML_Matrix *b = VM_stack_top[1].matrix;

									ans.vector[0].x = a->vector[0].x * b->vector[0].x + a->vector[0].y * b->vector[1].x + a->vector[0].z * b->vector[2].x;
									ans.vector[0].y = a->vector[0].x * b->vector[0].y + a->vector[0].y * b->vector[1].y + a->vector[0].z * b->vector[2].y;
									ans.vector[0].z = a->vector[0].x * b->vector[0].z + a->vector[0].y * b->vector[1].z + a->vector[0].z * b->vector[2].z;

									ans.vector[1].x = a->vector[1].x * b->vector[0].x + a->vector[1].y * b->vector[1].x + a->vector[1].z * b->vector[2].x;
									ans.vector[1].y = a->vector[1].x * b->vector[0].y + a->vector[1].y * b->vector[1].y + a->vector[1].z * b->vector[2].y;
									ans.vector[1].z = a->vector[1].x * b->vector[0].z + a->vector[1].y * b->vector[1].z + a->vector[1].z * b->vector[2].z;

									ans.vector[2].x = a->vector[2].x * b->vector[0].x + a->vector[2].y * b->vector[1].x + a->vector[2].z * b->vector[2].x;
									ans.vector[2].y = a->vector[2].x * b->vector[0].y + a->vector[2].y * b->vector[1].y + a->vector[2].z * b->vector[2].y;
									ans.vector[2].z = a->vector[2].x * b->vector[0].z + a->vector[2].y * b->vector[1].z + a->vector[2].z * b->vector[2].z;

								   *VM_stack_top[0].matrix = ans;

									//
									// Don't need the second matrix any more.
									//

									VM_data_free(VM_stack_top[1]);
								}

								break;
							
							case ML_TYPE_VECTOR:

								//
								// Matrix * vector.
								//

								{
									ML_Vector ans;

									ans.x = VM_stack_top[0].matrix->vector[0].x * VM_stack_top[1].vector->x + VM_stack_top[0].matrix->vector[0].y * VM_stack_top[1].vector->y + VM_stack_top[0].matrix->vector[0].z * VM_stack_top[1].vector->z;
									ans.y = VM_stack_top[0].matrix->vector[1].x * VM_stack_top[1].vector->x + VM_stack_top[0].matrix->vector[1].y * VM_stack_top[1].vector->y + VM_stack_top[0].matrix->vector[1].z * VM_stack_top[1].vector->z;
									ans.z = VM_stack_top[0].matrix->vector[2].x * VM_stack_top[1].vector->x + VM_stack_top[0].matrix->vector[2].y * VM_stack_top[1].vector->y + VM_stack_top[0].matrix->vector[2].z * VM_stack_top[1].vector->z;

									VM_data_free(VM_stack_top[0]);

									VM_stack_top[0]           = VM_stack_top[1];
									VM_stack_top[0].vector->x = ans.x;
									VM_stack_top[0].vector->y = ans.y;
									VM_stack_top[0].vector->z = ans.z;
								}

								break;

							default:

								//
								// ERROR!
								//

								ASSERT(0);

								break;
						}

						break;

					case ML_TYPE_VECTOR:

						switch(VM_stack_top[1].type)
						{
							case ML_TYPE_SLUMBER:

								{
									float fmul = float(VM_stack_top[1].slumber);

									VM_stack_top[0].vector->x *= fmul;
									VM_stack_top[0].vector->y *= fmul;
									VM_stack_top[0].vector->z *= fmul;
								}

								break;

							case ML_TYPE_FLUMBER:

								{
									float fmul = VM_stack_top[1].flumber;

									VM_stack_top[0].vector->x *= fmul;
									VM_stack_top[0].vector->y *= fmul;
									VM_stack_top[0].vector->z *= fmul;
								}

								break;

							default:
								
								//
								// ERROR!
								//

								ASSERT(0);

								break;
						}
						

						break;

					default:

						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_DIVIDE:
				
				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// Must do type conversion...
					//

					VM_convert_stack_top_to_same_numerical_type();
				}

				ASSERT(VM_stack_top[0].type == VM_stack_top[1].type);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER:
						VM_stack_top[0].slumber = VM_stack_top[0].slumber / VM_stack_top[1].slumber;
						break;

					case ML_TYPE_FLUMBER:
						VM_stack_top[0].flumber = VM_stack_top[0].flumber / VM_stack_top[1].flumber;
						break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_MOD:
				
				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// Must do type conversion...
					//

					VM_convert_stack_top_to_same_numerical_type();
				}

				ASSERT(VM_stack_top[0].type == VM_stack_top[1].type);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER:
						VM_stack_top[0].slumber = VM_stack_top[0].slumber % VM_stack_top[1].slumber;
						break;

					case ML_TYPE_FLUMBER:
						VM_stack_top[0].flumber = fmodf(VM_stack_top[0].flumber,VM_stack_top[1].flumber);
						break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_IF_FALSE_GOTO:

				VM_POP_STACK(1);

				if (VM_stack_top[0].type != ML_TYPE_BOOLEAN)
				{
					//
					// ERROR! Or maybe type conversion? What if there is a string
					// that says "YES", for instance?
					//

					ASSERT(0);
				}

				if (!VM_stack_top[0].boolean)
				{
					//
					// Valid address?
					//

					ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
					ASSERT(WITHIN(VM_code_pointer[0], 0, VM_code_upto - 1));

					VM_code_pointer = &VM_code[VM_code_pointer[0]];
				}
				else
				{
					//
					// Skip over the address instruction.
					//

					VM_code_pointer++;
				}

				break;

			case ML_DO_AND:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// ERROR! AND only works on the same values!
					//

					ASSERT(0);
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_BOOLEAN: VM_stack_top[0].boolean = VM_stack_top[0].boolean && VM_stack_top[1].boolean; break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top++;

				break;

			case ML_DO_OR:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// ERROR! AND only works on the same values!
					//

					ASSERT(0);
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_BOOLEAN: VM_stack_top[0].boolean = VM_stack_top[0].boolean || VM_stack_top[1].boolean; break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top++;

				break;

			case ML_DO_EQUALS:
			case ML_DO_NOTEQUAL:
			case ML_DO_JNEQ_POP_1:

				{
					SLONG are_equal = FALSE;

					VM_POP_STACK(2);

					if (VM_stack_top[0].type !=	VM_stack_top[1].type)
					{
						//
						// Must do some type conversion???
						//

						if ((VM_stack_top[0].type == ML_TYPE_STRVAR || VM_stack_top[0].type == ML_TYPE_STRCONST) &&
							(VM_stack_top[1].type == ML_TYPE_STRVAR || VM_stack_top[1].type == ML_TYPE_STRCONST))
						{
							//
							// Both are strings... 
							//
						}
						else
						{
							//
							// Different types aren't equal.
							//

							are_equal = FALSE;

							goto found_out_equality;
						}
					}

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_BOOLEAN: are_equal = VM_stack_top[0].boolean == VM_stack_top[1].boolean; break;
						case ML_TYPE_SLUMBER: are_equal = VM_stack_top[0].slumber == VM_stack_top[1].slumber; break;
						case ML_TYPE_FLUMBER: are_equal = VM_stack_top[0].flumber == VM_stack_top[1].flumber; break;

						case ML_TYPE_STRCONST:
						case ML_TYPE_STRVAR:
							
							{
								if (strcmp(
										VM_get_string(VM_stack_top[0]), 
										VM_get_string(VM_stack_top[1])) == 0)
								{
									are_equal = TRUE;
								}
								else
								{
									are_equal = FALSE;
								}
							}

							break;

						case ML_TYPE_UNDEFINED:
							are_equal = TRUE;
							break;

						default:

							//
							// ERROR!
							//

							ASSERT(0);

							break;
					}

				  found_out_equality:;

					switch(VM_code_pointer[-1])
					{
						case ML_DO_NOTEQUAL:

							VM_data_free(VM_stack_top[0]);
							VM_data_free(VM_stack_top[1]);

							VM_stack_top[0].type    =  ML_TYPE_BOOLEAN;
							VM_stack_top[0].boolean = !are_equal;

							VM_stack_top++;

							break;

						case ML_DO_JNEQ_POP_1:

							if (VM_stack_top[0].boolean)
							{
								//
								// Pop both values.
								//

								VM_data_free(VM_stack_top[0]);
								VM_data_free(VM_stack_top[1]);

								//
								// Step over the jump address.
								//

								VM_code_pointer += 1;
							}
							else
							{
								//
								// Pop just one value and jump.
								//

								VM_data_free(VM_stack_top[1]);

								VM_stack_top++;

								//
								// Valid address?
								//

								ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
								ASSERT(WITHIN(VM_code_pointer[0], 0, VM_code_upto - 1));

								VM_code_pointer = &VM_code[VM_code_pointer[0]];
							}

							break;

						case ML_DO_EQUALS:

							VM_data_free(VM_stack_top[0]);
							VM_data_free(VM_stack_top[1]);

							VM_stack_top[0].type    = ML_TYPE_BOOLEAN;
							VM_stack_top[0].boolean = are_equal;

							VM_stack_top++;

							break;

						default:
							ASSERT(0);
							break;
					}
				}

				break;

			case ML_DO_GT:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type !=	VM_stack_top[1].type)
				{
					//
					// Must do some type conversion.
					//

					VM_convert_stack_top_to_same_numerical_type();
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].slumber > VM_stack_top[1].slumber; break;
					case ML_TYPE_FLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].flumber > VM_stack_top[1].flumber; break;

					default:

						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top[0].type = ML_TYPE_BOOLEAN;
				VM_stack_top++;

				break;

			case ML_DO_LT:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type !=	VM_stack_top[1].type)
				{
					//
					// Must do some type conversion.
					//

					VM_convert_stack_top_to_same_numerical_type();
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].slumber < VM_stack_top[1].slumber; break;
					case ML_TYPE_FLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].flumber < VM_stack_top[1].flumber; break;

					default:

						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top[0].type = ML_TYPE_BOOLEAN;
				VM_stack_top++;

				break;

			case ML_DO_GTEQ:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type !=	VM_stack_top[1].type)
				{
					//
					// Must do some type conversion.
					//
				
					VM_convert_stack_top_to_same_numerical_type();
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].slumber >= VM_stack_top[1].slumber; break;
					case ML_TYPE_FLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].flumber >= VM_stack_top[1].flumber; break;

					default:

						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top[0].type = ML_TYPE_BOOLEAN;
				VM_stack_top++;

				break;

			case ML_DO_LTEQ:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type !=	VM_stack_top[1].type)
				{
					//
					// Must do some type conversion.
					//

					VM_convert_stack_top_to_same_numerical_type();
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].slumber <= VM_stack_top[1].slumber; break;
					case ML_TYPE_FLUMBER: VM_stack_top[0].boolean = VM_stack_top[0].flumber <= VM_stack_top[1].flumber; break;

					default:

						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top[0].type = ML_TYPE_BOOLEAN;
				VM_stack_top++;

				break;

			case ML_DO_NOT:

				VM_POP_STACK(1);

				switch (VM_stack_top[0].type)
				{
					case ML_TYPE_BOOLEAN: VM_stack_top[0].boolean = !VM_stack_top[0].boolean; break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top++;
				
				break;

			case ML_DO_SQRT:
				
				VM_POP_STACK(1);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER: VM_stack_top[0].slumber = (SLONG) sqrtf(float(VM_stack_top[0].slumber)); break;
					case ML_TYPE_FLUMBER: VM_stack_top[0].flumber =         sqrtf(float(VM_stack_top[0].flumber)); break;

					default:
						
						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_NEWLINE:
				CONSOLE_print("");
				break;

			case ML_DO_ABS:

				VM_POP_STACK(1);

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_SLUMBER: VM_stack_top[0].slumber = abs  (VM_stack_top[0].slumber); break;
					case ML_TYPE_FLUMBER: VM_stack_top[0].flumber = fabsf(VM_stack_top[0].flumber); break;

					default:
						
						//
						// ERROR!
						//

						ASSERT(0);

						break;
				}

				VM_stack_top += 1;

				break;

			case ML_DO_PUSH_GLOBAL_ADDRESS:
				
				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN(VM_code_pointer + 1, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(VM_code_pointer[0], 0, VM_global_max - 1));

				VM_stack_top[0].type =  ML_TYPE_POINTER;
				VM_stack_top[0].data = &VM_global[*VM_code_pointer++];

				VM_stack_top++;

				break;

			case ML_DO_ASSIGN:

				VM_POP_STACK(2);

				ASSERT(VM_stack_top[1].type == ML_TYPE_POINTER);
				
				//
				// Free any memory used by the current variable.
				//

				VM_data_free(*VM_stack_top[1].data);

				//
				// Overwrite old value.
				//

			   *VM_stack_top[1].data = VM_stack_top[0];

				break;

			case ML_DO_PUSH_FIELD_ADDRESS:

				VM_POP_STACK(1);

				ASSERT(WITHIN(VM_code_pointer + 1, VM_code, VM_code + VM_code_upto - 1));

				if (VM_stack_top[0].type == ML_TYPE_POINTER)
				{
					SLONG    field_id;
					ML_Data *data;

					field_id = *VM_code_pointer++;
					data     =  VM_stack_top[0].data;

					switch(data->type)
					{
						case ML_TYPE_STRUCTURE:

							//
							// Does this variable already have this field?
							//

							SLONG i;

							for (i = 0; i < data->structure->num_fields; i++)
							{
								if (data->structure->field[i].field_id == field_id)
								{
									//
									// Found our field. Push the address onto the stack.
									//

									VM_stack_top[0].type =  ML_TYPE_POINTER;
									VM_stack_top[0].data = &data->structure->field[i].data;

									VM_stack_top++;

									break;
								}
							}

							//
							// Must add another field.
							//

							ML_Structure *ms;
							
							ms = (ML_Structure *) MEM_alloc(sizeof(ML_Structure) + (data->structure->num_fields + 1) * sizeof(ML_Field));

							memcpy(ms, data->structure, sizeof(ML_Structure) + data->structure->num_fields * sizeof(ML_Field));

							ms->field[ms->num_fields].field_id  = field_id;
							ms->field[ms->num_fields].data.type = ML_TYPE_UNDEFINED;

							//
							// Push address onto the stack.
							//

							VM_stack_top[0].type =  ML_TYPE_POINTER;
							VM_stack_top[0].data = &ms->field[ms->num_fields].data;

							VM_stack_top++;

							ms->num_fields += 1;

							//
							// Get rid of old memory- point to new memory.
							//

							MEM_free(data->structure);

							data->structure = ms;

							break;

						case ML_TYPE_MATRIX:

							//
							// Can only access fields (x,y,z)
							//

							if (!WITHIN(field_id, SYSVAR_FIELD_X, SYSVAR_FIELD_Z))
							{
								//
								// ERROR!
								//

								ASSERT(0);
							}

							//
							// Push address onto the stack.
							//

							VM_stack_top[0].type   =  ML_TYPE_VOINTER;
							VM_stack_top[0].vector = &data->matrix->vector[field_id];

							VM_stack_top++;

							break;

						case ML_TYPE_VECTOR:

							//
							// Can only access fields (x,y,z)
							//

							if (!WITHIN(field_id, SYSVAR_FIELD_X, SYSVAR_FIELD_Z))
							{
								//
								// ERROR!
								//

								ASSERT(0);
							}

							//
							// Push address onto the stack.
							//

							VM_stack_top[0].type     = ML_TYPE_FLOINTER;
							VM_stack_top[0].flointer = &(&data->vector->x)[field_id];

							VM_stack_top++;

							break;

						default:

							//
							// Create a struture with just one field marked as undefined.
							//

							VM_data_free(*data);

							data->type                          = ML_TYPE_STRUCTURE;
							data->structure                     = (ML_Structure *) MEM_alloc(sizeof(ML_Structure) + sizeof(ML_Field) * 1);
							data->structure->num_fields         = 1;
							data->structure->field[0].field_id  = field_id;
							data->structure->field[0].data.type = ML_TYPE_UNDEFINED;

							//
							// Push the address of this field onto the stack.
							//

							VM_stack_top[0].type =  ML_TYPE_POINTER;
							VM_stack_top[0].data = &data->structure->field[0].data;

							VM_stack_top++;

							break;
					}
				}
				else
				if (VM_stack_top[0].type == ML_TYPE_VOINTER)
				{
					SLONG    field_id;
					ML_Data *data;

					field_id = *VM_code_pointer++;

					//
					// Can only access fields (x,y,z)
					//

					if (!WITHIN(field_id, SYSVAR_FIELD_X, SYSVAR_FIELD_Z))
					{
						//
						// ERROR!
						//

						ASSERT(0);
					}

					//
					// Push address onto the stack.
					//

					VM_stack_top[0].type     = ML_TYPE_FLOINTER;
					VM_stack_top[0].flointer = &(&VM_stack_top[0].vector->x)[field_id];

					VM_stack_top++;
				}
				else
				if (VM_stack_top[0].type == ML_TYPE_FLOINTER)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}
				else
				{
					//
					// ERROR! This is an internal error - not a real error.
					//

					ASSERT(0);
				}

				break;

			case ML_DO_PUSH_FIELD_VALUE:
			case ML_DO_PUSH_FIELD_QUICK:

				VM_POP_STACK(1);

				ASSERT(WITHIN(VM_code_pointer + 1, VM_code, VM_code + VM_code_upto - 1));

				{
					SLONG field_id;

					field_id = *VM_code_pointer++;

					//
					// Is this variable already a structure?
					//

					if (VM_stack_top[0].type != ML_TYPE_STRUCTURE)
					{
						if (VM_stack_top[0].type == ML_TYPE_MATRIX)
						{
							if (WITHIN(field_id, SYSVAR_FIELD_X, SYSVAR_FIELD_Z))
							{
								ML_Data ans;

								if (VM_code_pointer[-2] == ML_DO_PUSH_FIELD_QUICK)
								{
									//
									// Point to the same data as the matrix.
									//

									ans.type   =  ML_TYPE_VECTOR;
									ans.vector = &VM_stack_top[0].matrix->vector[field_id];
								}
								else
								{
									//
									// Copy the matrix data to create a new vector.
									//

									ans.type     = ML_TYPE_VECTOR;
									ans.vector   = (ML_Vector *) MEM_alloc(sizeof(ML_Vector));
								   *(ans.vector) = VM_stack_top[0].matrix->vector[field_id];

									VM_stack_top[0] = ans;
									VM_stack_top++;
								}
							}
							else
							{
								//
								// No such field... push <UNDEFINED> onto the stack.
								//

								VM_stack_top[0].type = ML_TYPE_UNDEFINED;

								VM_stack_top++;
							}
						}
						else
						if (VM_stack_top[0].type == ML_TYPE_VECTOR)
						{
							if (WITHIN(field_id, SYSVAR_FIELD_X, SYSVAR_FIELD_Z))
							{
								ML_Data ans;

								//
								// Point to the same data as the matrix.
								//

								ans.type    = ML_TYPE_FLUMBER;
								ans.flumber = (&VM_stack_top[0].vector->x)[field_id];

								VM_stack_top[0] = ans;
								VM_stack_top++;
							}
							else
							{
								//
								// No such field... push <UNDEFINED> onto the stack.
								//

								VM_stack_top[0].type = ML_TYPE_UNDEFINED;

								VM_stack_top++;
							}
						}
						else
						{
							//
							// No such field... push <UNDEFINED> onto the stack.
							//

							VM_stack_top[0].type = ML_TYPE_UNDEFINED;

							VM_stack_top++;
						}
					}
					else
					{
						//
						// Does this variable already have this field?
						//

						SLONG i;

						for (i = 0; i < VM_stack_top[0].structure->num_fields; i++)
						{
							if (VM_stack_top[0].structure->field[i].field_id == field_id)
							{
								//
								// Found our field. Push a copy of the value onto the stack.
								//

								if (VM_code_pointer[-2] == ML_DO_PUSH_FIELD_QUICK)
								{
									//
									// Push on the actual data rather than a copy.
									//

									VM_stack_top[0] = VM_stack_top[0].structure->field[i].data;
								}
								else
								{
									//
									// Push on a copy of the data.
									//

									VM_stack_top[0] = VM_data_copy(VM_stack_top[0].structure->field[i].data);
								}

								VM_stack_top++;

								goto pushed_field_value;
							}
						}

						VM_stack_top[0].type = ML_TYPE_UNDEFINED;

						VM_stack_top++;

					  pushed_field_value:;
					}
				}

				break;

			case ML_DO_PUSH_ARRAY_ADDRESS:

				{
					SLONG i;
					SLONG j;
					SLONG array_length;
					SLONG num_dimensions;

					ASSERT(WITHIN(VM_code_pointer + 1, VM_code, VM_code + VM_code_upto - 1));

					num_dimensions = *VM_code_pointer++;

					VM_POP_STACK(num_dimensions + 1);

					//
					// Make sure all the indices are given by integers!
					//

					for (i = 0; i < num_dimensions; i++)
					{
						if (VM_stack_top[i + 1].type != ML_TYPE_SLUMBER)
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						if (VM_stack_top[i + 1].slumber <= 0)
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						if (VM_stack_top[i + 1].slumber >= 1024 * 1024 * 2 / 8)
						{
							//
							// ERROR! Limit to 2meg arrays to guard against ridiculous memory usage?
							//

							ASSERT(0);
						} 
					}

					//
					// Make sure we have a pointer on the stack.
					//

					if (VM_stack_top[0].type != ML_TYPE_POINTER)
					{
						//
						// ERROR!
						//

						ASSERT(0);
					}

					ML_Data *data = VM_stack_top[0].data;

					//
					// Do we already have an array on the stack of the correct
					// dimensions?
					//

					if (data->type != ML_TYPE_ARRAY || data->array->num_dimensions != num_dimensions)
					{
						//
						// Free old value.
						//

						VM_data_free(*data);

						//
						// How long should the array be?
						//

						array_length = VM_stack_top[1].slumber;

						for (i = 1; i < num_dimensions; i++)
						{
							array_length *= VM_stack_top[i + 1].slumber;
						}

						//
						// Create the array.
						//

						data->type                  = ML_TYPE_ARRAY;
						data->array                 = (ML_Array *) MEM_alloc(sizeof(ML_Array) + sizeof(ML_Dimension) * num_dimensions);
						data->array->length         = array_length;
						data->array->num_dimensions = num_dimensions;
						data->array->data           = (ML_Data *) MEM_alloc(sizeof(ML_Data) * array_length);

						for (i = 0; i < num_dimensions; i++)
						{
							data->array->dimension[i].size   = VM_stack_top[i + 1].slumber;
							data->array->dimension[i].stride = 1;

							for (j = i + 1; j < num_dimensions; j++)
							{
								data->array->dimension[i].stride *= VM_stack_top[j + 1].slumber;
							}
						}

						//
						// Make all elements of the array undefined.
						//

						memset(data->array->data, 0, sizeof(ML_Data) * array_length);
					}

					//
					// Should have a compatible data type by now...
					//

					ASSERT(data->type == ML_TYPE_ARRAY);
					ASSERT(data->array->num_dimensions == num_dimensions);

					//
					// Is this array big enough?
					//

					for (i = 0; i < num_dimensions; i++)
					{
						if (VM_stack_top[i + 1].slumber > data->array->dimension[i].size)
						{
							VM_grow_array(data->array, VM_stack_top + 1);

							goto grown_array;
						}
					}

				  grown_array:;

					//
					// What's this index of this element into the array?
					//

					SLONG index = 0;

					for (i = 0; i < num_dimensions; i++)
					{
						index += data->array->dimension[i].stride * (VM_stack_top[i + 1].slumber - 1);
					}

					ASSERT(WITHIN(index, 0, data->array->length - 1));

					//
					// Push the address of the element onto the stack.
					//

					VM_stack_top[0].type = ML_TYPE_POINTER;
					VM_stack_top[0].data = data->array->data + index;

					VM_stack_top++;
				}

				break;

			case ML_DO_PUSH_ARRAY_VALUE:
			case ML_DO_PUSH_ARRAY_QUICK:

				{
					SLONG i;
					SLONG index;
					SLONG num_dimensions;

					ASSERT(WITHIN(VM_code_pointer + 1, VM_code, VM_code + VM_code_upto - 1));

					num_dimensions = *VM_code_pointer++;

					VM_POP_STACK(num_dimensions + 1);
				
					//
					// Make sure all the indices are given by integers!
					//

					for (i = 0; i < num_dimensions; i++)
					{
						if (VM_stack_top[i + 1].type != ML_TYPE_SLUMBER)
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						if (VM_stack_top[i + 1].slumber <= 0)
						{
							//
							// ERROR!
							//

							ASSERT(0);
						}

						if (VM_stack_top[i + 1].slumber >= 1024 * 1024 * 2 / 8)
						{
							//
							// ERROR! Limit to 2meg arrays to guard against ridiculous memory usage?
							//

							ASSERT(0);
						} 
					}

					if (VM_stack_top[0].type != ML_TYPE_ARRAY || VM_stack_top[0].array->num_dimensions != num_dimensions)
					{
						//
						// Incompatible data type- push <UNDEFINED>
						//

						VM_stack_top[0].type = ML_TYPE_UNDEFINED;

						VM_stack_top++;
					}
					else
					{
						//
						// Are all indices in range?
						//

						for (i = 0; i < num_dimensions; i++)
						{
							if (VM_stack_top[i + 1].slumber > VM_stack_top[0].array->dimension[i].size)
							{
								//
								// Referencing outside the array.
								//

								VM_stack_top[0].type = ML_TYPE_UNDEFINED;

								VM_stack_top++;
								
								goto outside_the_array;
							}
						}

						//
						// What's this index of this element into the array?
						//

						index = 0;

						for (i = 0; i < num_dimensions; i++)
						{
							index += VM_stack_top[0].array->dimension[i].stride * (VM_stack_top[i + 1].slumber - 1);
						}

						//
						// Push a copy of this element onto the stack.
						//

						ASSERT(WITHIN(index, 0, VM_stack_top[0].array->length - 1));

						if (VM_code_pointer[-2] == ML_DO_PUSH_ARRAY_QUICK)
						{
							//
							// Push the actual value rather than a copy.
							//

							VM_stack_top[0] = VM_stack_top[0].array->data[index];
						}
						else
						{
							//
							// Push a copy of the data.
							//

							VM_stack_top[0] = VM_data_copy(VM_stack_top[0].array->data[index]);
						}

						VM_stack_top++;

					  outside_the_array:;
					}
				}

				break;

			case ML_DO_PUSH_INPUT:
				
				{
					CBYTE *string;

					//
					// Get user input!
					//

					string = CONSOLE_input();

					VM_CHECK_STACK_PUSH();

					VM_stack_top[0].type   = ML_TYPE_STRVAR;
					VM_stack_top[0].strvar = (CBYTE *) MEM_alloc(strlen(string) + 1);

					strcpy(VM_stack_top[0].strvar, string);

					VM_stack_top++;
				}

				break;

			case ML_DO_GOSUB:

				VM_CHECK_STACK_PUSH();

				//
				// Push the return address onto the stack.
				//

				VM_stack_top[0].type         = ML_TYPE_CODE_POINTER;
				VM_stack_top[0].code_pointer = VM_code_pointer + 1;

				VM_stack_top++;

				//
				// Valid address?
				//

				ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(VM_code_pointer[0], 0, VM_code_upto - 1));

				VM_code_pointer = &VM_code[VM_code_pointer[0]];

				break;
			
			case ML_DO_RETURN:

				VM_POP_STACK(1);

				ASSERT(VM_stack_top[0].type == ML_TYPE_CODE_POINTER);
				ASSERT(WITHIN(VM_stack_top[0].code_pointer, VM_code, VM_code + VM_code_upto - 1));

				VM_code_pointer = VM_stack_top[0].code_pointer;

				break;

			case ML_DO_XOR:

				VM_POP_STACK(2);

				if (VM_stack_top[0].type != VM_stack_top[1].type)
				{
					//
					// ERROR! AND only works on the same values!
					//

					ASSERT(0);
				}

				switch(VM_stack_top[0].type)
				{
					case ML_TYPE_BOOLEAN: VM_stack_top[0].boolean = VM_stack_top[0].boolean ^ VM_stack_top[1].boolean; break;

					default:
						ASSERT(0);
						break;
				}

				VM_stack_top++;

				break;

			case ML_DO_IF_TRUE_GOTO:

				VM_POP_STACK(1);

				if (VM_stack_top[0].type != ML_TYPE_BOOLEAN)
				{
					//
					// ERROR! Or maybe type conversion? What if there is a string
					// that says "YES", for instance?
					//

					ASSERT(0);
				}

				if (VM_stack_top[0].boolean)
				{
					//
					// Valid address?
					//

					ASSERT(WITHIN(VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
					ASSERT(WITHIN(VM_code_pointer[0], 0, VM_code_upto - 1));

					VM_code_pointer = &VM_code[VM_code_pointer[0]];
				}
				else
				{
					//
					// Skip over the address instruction.
					//

					VM_code_pointer++;
				}

				break;

			case ML_DO_PUSH_RANDOM_SLUMBER:
				
				VM_CHECK_STACK_PUSH();

				VM_stack_top[0].type    = ML_TYPE_SLUMBER;
				VM_stack_top[0].slumber = rand();

				VM_stack_top++;

				break;
			
			case ML_DO_SWAP:

				VM_POP_STACK(2);

				ASSERT(VM_stack_top[0].type == ML_TYPE_POINTER);
				ASSERT(VM_stack_top[1].type == ML_TYPE_POINTER);

				{
					ML_Data swap_spare;

					swap_spare           = *VM_stack_top[0].data;
				   *VM_stack_top[0].data = *VM_stack_top[1].data;
				   *VM_stack_top[1].data =  swap_spare;
				}

				break;

			case ML_DO_ENTERFUNC:
	
				{
					SLONG i;

					//
					// Do we have the right number of arguments?
					//

					ASSERT(VM_stack_top[-2].type == ML_TYPE_NUM_ARGS    );	// Num args to functions
					ASSERT(VM_stack_top[-1].type == ML_TYPE_CODE_POINTER);	// Return address

					SLONG args_to_function = VM_stack_top[-2].args;

					if (args_to_function == *VM_code_pointer)
					{
						//
						// Get rid of the number of args to the function.
						//

						VM_POP_STACK(1);

						VM_stack_top[-1] = VM_stack_top[0];
					}
					else
					{
						if (args_to_function > *VM_code_pointer)
						{
							//
							// ERROR! Too many argument passed to the function!
							//

							ASSERT(0);
						}
						else
						{
							//
							// Remember the code pointer so we wont overwrite it.
							//

							SLONG *code_pointer = VM_stack_top[-1].code_pointer;

							//
							// Must insert extra undefined arguments.
							//

							VM_POP_STACK(2);
							
							for (i = args_to_function; i < *VM_code_pointer; i++)
							{
								VM_CHECK_STACK_PUSH();

								VM_stack_top[0].type  = ML_TYPE_UNDEFINED;
								VM_stack_top[0].value = 0;	// Not required.

								VM_stack_top++;
							}

							//
							// Now push the return address.
							//

							VM_CHECK_STACK_PUSH();

							VM_stack_top[0].type         = ML_TYPE_CODE_POINTER;
							VM_stack_top[0].code_pointer = code_pointer;

							VM_stack_top++;
						}
					}
				}

				//
				// Push the old base pointer.
				//

				VM_CHECK_STACK_PUSH();

				VM_stack_top[0].type       = ML_TYPE_STACK_BASE;
				VM_stack_top[0].stack_base = VM_stack_base;

				VM_stack_top++;

				//
				// Work out the new stack base.
				//

				ASSERT(WITHIN( VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(*VM_code_pointer, 0, 256));	// Sensible numbers of arguments?

				//
				// Minus 2 to skip over the return address and the old stack base.
				//

				VM_stack_base = VM_stack_top - *VM_code_pointer++ - 2;

				break;

			case ML_DO_ENDFUNC:

				{
					//
					// On the stack there should be the return value of the function,
					// the old base pointer and the return address.
					//

					VM_POP_STACK(3);

					ASSERT(VM_stack_top[0].type == ML_TYPE_CODE_POINTER);
					ASSERT(VM_stack_top[1].type == ML_TYPE_STACK_BASE  );

					SLONG *return_instruction = VM_stack_top[0].code_pointer;

					//
					// Pop the locals off the stack.
					//

					ASSERT(WITHIN( VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
					ASSERT(WITHIN(*VM_code_pointer, 0, 256));	// Sensible numbers of arguments?

					VM_POP_STACK(*VM_code_pointer);

					//
					// Free all the locals.
					//

					SLONG i;

					for (i = 0; i < *VM_code_pointer; i++)
					{
						VM_data_free(VM_stack_top[i]);
					}

					//
					// Push the return value onto the bottom of the stack.
					//

					VM_stack_top[0] = VM_stack_top[*VM_code_pointer + 2];

					VM_stack_top++;

					//
					// Continue execution from the return address.
					//

					VM_code_pointer = return_instruction;
				}

				break;

			case ML_DO_POP:
				VM_POP_STACK(1);
				break;

			case ML_DO_PUSH_LOCAL_VALUE:

				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN( VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(*VM_code_pointer, 0, 256));	// Sensible local index?

				ASSERT(VM_stack_base);
				ASSERT(VM_stack_base + *VM_code_pointer <= VM_stack_top - 2);

				ASSERT(VM_stack_base[*VM_code_pointer].type != ML_TYPE_CODE_POINTER);
				ASSERT(VM_stack_base[*VM_code_pointer].type != ML_TYPE_STACK_BASE);

				//
				// Push a copy of the local onto the stack. Any memory allocated
				// by the original will be duplicated.
				//

			   *VM_stack_top++ = VM_data_copy(VM_stack_base[*VM_code_pointer++]);

				break;

			case ML_DO_PUSH_LOCAL_ADDRESS:

				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN( VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(*VM_code_pointer, 0, 256));	// Sensible local index?

				ASSERT(VM_stack_base);
				ASSERT(VM_stack_base + *VM_code_pointer < VM_stack_top - 2);

				ASSERT(VM_stack_base[*VM_code_pointer].type != ML_TYPE_CODE_POINTER);
				ASSERT(VM_stack_base[*VM_code_pointer].type != ML_TYPE_STACK_BASE);

				if (VM_stack_base[*VM_code_pointer].type == ML_TYPE_POINTER)
				{
					//
					// This local is already a pointer, just push on
					// it's current value.
					//

					VM_stack_top[0] = VM_stack_base[*VM_code_pointer++];
				}
				else
				{
					//
					// Push on the address of this local.
					//

					VM_stack_top[0].type = ML_TYPE_POINTER;
					VM_stack_top[0].data = VM_stack_base + *VM_code_pointer++;
				}

				VM_stack_top++;

				break;

			case ML_DO_PUSH_LOCAL_QUICK:

				VM_CHECK_STACK_PUSH();

				ASSERT(WITHIN( VM_code_pointer, VM_code, VM_code + VM_code_upto - 1));
				ASSERT(WITHIN(*VM_code_pointer, 0, 256));	// Sensible local index?

				ASSERT(VM_stack_base);
				ASSERT(VM_stack_base + *VM_code_pointer < VM_stack_top - 2);

				ASSERT(VM_stack_base[*VM_code_pointer].type != ML_TYPE_CODE_POINTER);
				ASSERT(VM_stack_base[*VM_code_pointer].type != ML_TYPE_STACK_BASE);

				//
				// Push a copy of the local onto the stack - don't copy data.
				//

			   *VM_stack_top++ = VM_stack_base[*VM_code_pointer++];

				break;

			case ML_DO_TEXTURE: VM_do_texture(); break;
			case ML_DO_BUFFER:  VM_do_buffer();  break;
			case ML_DO_DRAW:    VM_do_draw();    break;
			case ML_DO_CLS:     VM_do_cls();     break;

			case ML_DO_FLIP:

				//
				// Less than 10 milliseconds since our last FLIP?
				//

				while(VM_flip_tick > OS_ticks() - 10);

				LL_flip();

				VM_flip_tick = OS_ticks();

				break;

			case ML_DO_KEY_VALUE:

				VM_POP_STACK(1);

				if (VM_stack_top[0].type != ML_TYPE_SLUMBER)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				if (!WITHIN(VM_stack_top[0].slumber, 1, 255))
				{
					//
					// Outside the array...
					//

					VM_stack_top[0].type = ML_TYPE_UNDEFINED;
				}
				else
				{
					if (KEY_on[VM_stack_top[0].slumber] == 0 ||
						KEY_on[VM_stack_top[0].slumber] == 1)
					{
						//
						// This key has been pressed since the last time
						// it was assigned to.  Free the current key value.
						//

						VM_data_free(VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[0].slumber)]);

						//
						// Create a new value.
						//

						VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[0].slumber)].type    = ML_TYPE_BOOLEAN;
						VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[0].slumber)].boolean = KEY_on[VM_stack_top[0].slumber];

						//
						// Push it onto the stack.
						//

						VM_stack_top[0] = VM_data_copy(VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[0].slumber)]);
					}
					else
					{
						//
						// Use the value the user wrote to the KEY[] array...
						//

						VM_stack_top[0] = VM_data_copy(VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[0].slumber)]);
					}
				}

				VM_stack_top++;

				break;

			case ML_DO_KEY_ASSIGN:

				VM_POP_STACK(2);

				if (VM_stack_top[1].type != ML_TYPE_SLUMBER)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				if (!WITHIN(VM_stack_top[1].slumber, 1, 255))
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				//
				// Free the current value.
				//

				VM_data_free(VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[1].slumber)]);

				//
				// Assign the new number.
				//

				VM_global[VM_EXTRA_GLOBAL_KEY(VM_stack_top[1].slumber)] = VM_stack_top[0];

				//
				// Overwrite the real KEY_on array with a special value so we know
				// if the key has been pressed or release since it was assigned to...
				//

				KEY_on[VM_stack_top[1].slumber] = 42;

				break;

			case ML_DO_INKEY_VALUE:
				
				VM_CHECK_STACK_PUSH();

				if (KEY_inkey)
				{
					VM_data_free(VM_global[VM_EXTRA_GLOBAL_INKEY]);

					VM_global[VM_EXTRA_GLOBAL_INKEY].type    = ML_TYPE_SLUMBER;
					VM_global[VM_EXTRA_GLOBAL_INKEY].slumber = KEY_inkey;

					KEY_inkey = 0;
				}

				VM_stack_top[0] = VM_data_copy(VM_global[VM_EXTRA_GLOBAL_INKEY]);

				VM_stack_top++;
				
				break;

			case ML_DO_INKEY_ASSIGN:

				VM_POP_STACK(1);

				//
				// Free memory used by the current inkey variable.
				//

				VM_data_free(VM_global[VM_EXTRA_GLOBAL_INKEY]);

				//
				// Assign the new value.
				//

				VM_global[VM_EXTRA_GLOBAL_INKEY] = VM_stack_top[0];

				//
				// Overwrite the real inkey too!
				//

				KEY_inkey = 0;

				break;

			case ML_DO_TIMER:

				VM_CHECK_STACK_PUSH();

				VM_stack_top[0].type    = ML_TYPE_FLUMBER;
				VM_stack_top[0].flumber = float(OS_ticks()) * 0.001F;

				VM_stack_top++;

				break;

			case ML_DO_SIN:

				VM_POP_STACK(1);
				
				{
					float val;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = sinf(val);
				}

				VM_stack_top++;

				break;

			case ML_DO_COS:

				VM_POP_STACK(1);
				
				{
					float val;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = cosf(val);
				}

				VM_stack_top++;

				break;

			case ML_DO_TAN:

				VM_POP_STACK(1);
				
				{
					float val;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = tanf(val);
				}

				VM_stack_top++;

				break;

			case ML_DO_ASIN:

				VM_POP_STACK(1);
				
				{
					float val;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = asinf(val);
				}

				VM_stack_top++;

				break;

			case ML_DO_ACOS:

				VM_POP_STACK(1);
				
				{
					float val;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = acosf(val);
				}

				VM_stack_top++;

				break;

			case ML_DO_ATAN:

				VM_POP_STACK(1);
				
				{
					float val;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = atanf(val);
				}

				VM_stack_top++;

				break;

			case ML_DO_ATAN2:

				VM_POP_STACK(2);
				
				{
					float val1;
					float val2;

					switch(VM_stack_top[0].type)
					{
						case ML_TYPE_SLUMBER: val1 = float(VM_stack_top[0].slumber); break;
						case ML_TYPE_FLUMBER: val1 =       VM_stack_top[0].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					switch(VM_stack_top[1].type)
					{
						case ML_TYPE_SLUMBER: val2 = float(VM_stack_top[1].slumber); break;
						case ML_TYPE_FLUMBER: val2 =       VM_stack_top[1].flumber;  break;

						default:
							ASSERT(0);
							break;
					}

					VM_stack_top[0].type    = ML_TYPE_FLUMBER;
					VM_stack_top[0].flumber = atan2f(val1,val2);
				}

				VM_stack_top++;

				break;

			case ML_DO_NOP:
				break;

			case ML_DO_LEFT:

				VM_POP_STACK(2);

				//
				// Check types.
				//

				if (VM_stack_top[0].type != ML_TYPE_STRVAR &&
					VM_stack_top[0].type != ML_TYPE_STRCONST)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				if (VM_stack_top[1].type != ML_TYPE_SLUMBER)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				{
					ML_Data ans;

					CBYTE *input = VM_get_string(VM_stack_top[0]);
					SLONG  left  = VM_stack_top[1].slumber;

					if (left < 0)
					{
						//
						// ERROR!
						//

						ASSERT(0);
					}

					ans.type   = ML_TYPE_STRVAR;
					ans.strvar = (CBYTE *) MEM_alloc(left + 1);

					CBYTE *src = input;
					CBYTE *dst = ans.strvar;

					while(left > 0)
					{
						if (*src == '\000')
						{
							break;
						}

					   *dst++ = *src++;

						left--;
					}

				   *dst = '\000';

					//
					// Free args.
					//

					VM_data_free(VM_stack_top[0]);
					VM_data_free(VM_stack_top[1]);

					//
					// Push answer onto the stack.
					//

					VM_CHECK_STACK_PUSH();

					VM_stack_top[0] = ans;
					VM_stack_top++;
				}

				break;

			case ML_DO_MID:

				VM_POP_STACK(3);

				//
				// Check types.
				//

				if (VM_stack_top[0].type != ML_TYPE_STRVAR &&
					VM_stack_top[0].type != ML_TYPE_STRCONST)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				if (VM_stack_top[1].type != ML_TYPE_SLUMBER ||
					VM_stack_top[2].type != ML_TYPE_SLUMBER)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				{
					ML_Data ans;

					CBYTE *input = VM_get_string(VM_stack_top[0]);
					SLONG  in    = VM_stack_top[1].slumber - 1;	// - 1 because BASIC is 1-based not zero based.
					SLONG  num   = VM_stack_top[2].slumber;
					SLONG  len   = strlen(input);

					if (num < 0)
					{
						//
						// ERROR! Bad number of characters.
						//

						ASSERT(0);
					}

					ans.type   = ML_TYPE_STRVAR;
					ans.strvar = (CBYTE *) MEM_alloc(num + 1);

					if (!WITHIN(in, 0, len - 1))
					{
						ans.strvar[0] = '\000';
					}
					else
					{
						CBYTE *src = input + in;
						CBYTE *dst = ans.strvar;

						while(num > 0)
						{
							if (*src == '\000')
							{
								break;
							}

						   *dst++ = *src++;

							num--;
						}

					   *dst = '\000';
					}

					//
					// Free args.
					//

					VM_data_free(VM_stack_top[0]);
					VM_data_free(VM_stack_top[1]);
					VM_data_free(VM_stack_top[2]);

					//
					// Push answer onto the stack.
					//

					VM_CHECK_STACK_PUSH();

					VM_stack_top[0] = ans;
					VM_stack_top++;
				}

				break;

			case ML_DO_RIGHT:

				VM_POP_STACK(2);

				//
				// Check types.
				//

				if (VM_stack_top[0].type != ML_TYPE_STRVAR &&
					VM_stack_top[0].type != ML_TYPE_STRCONST)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				if (VM_stack_top[1].type != ML_TYPE_SLUMBER)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}

				{
					ML_Data ans;

					CBYTE *input = VM_get_string(VM_stack_top[0]);
					SLONG  len   = VM_stack_top[1].slumber;

					ans.type   = ML_TYPE_STRVAR;
					ans.strvar = (CBYTE *) MEM_alloc(len + 1);

					CBYTE *src = input;
					CBYTE *dst = ans.strvar;

					while(*src) {src++;}

					src -= len;

					if (src < input)
					{
						//
						// We want a segment that is bigger that the string!
						//

						src = input;
					}

					while(len > 0)
					{
						if (*src == '\000')
						{
							break;
						}

					   *dst++ = *src++;

						len--;
					}

				   *dst = '\000';

					//
					// Free args.
					//

					VM_data_free(VM_stack_top[0]);
					VM_data_free(VM_stack_top[1]);

					//
					// Push answer onto the stack.
					//

					VM_CHECK_STACK_PUSH();

					VM_stack_top[0] = ans;
					VM_stack_top++;
				}

				break;

			case ML_DO_LEN:

				VM_POP_STACK(1);

				if (VM_stack_top[0].type != ML_TYPE_STRVAR &&
					VM_stack_top[0].type != ML_TYPE_STRCONST)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}
				else
				{
					SLONG len = strlen(VM_get_string(VM_stack_top[0]));

					VM_data_free(VM_stack_top[0]);

					VM_CHECK_STACK_PUSH();

					VM_stack_top[0].type    = ML_TYPE_SLUMBER;
					VM_stack_top[0].slumber = len;

					VM_stack_top++;
				}

				break;
	
			case ML_DO_PUSH_IDENTITY_MATRIX:

				{
					const ML_Vector right    = {1.0F, 0.0F, 0.0F};
					const ML_Vector up       = {0.0F, 1.0F, 0.0F};
					const ML_Vector forwards = {0.0F, 0.0F, 1.0F};

					VM_CHECK_STACK_PUSH();

					VM_stack_top[0].type   = ML_TYPE_MATRIX;
					VM_stack_top[0].matrix = (ML_Matrix *) MEM_alloc(sizeof(ML_Matrix));

					VM_stack_top[0].matrix->vector[0] = right;
					VM_stack_top[0].matrix->vector[1] = up;
					VM_stack_top[0].matrix->vector[2] = forwards;

					VM_stack_top++;
				}

				break;

			case ML_DO_PUSH_ZERO_VECTOR:

				VM_CHECK_STACK_PUSH();

				VM_stack_top[0].type   = ML_TYPE_VECTOR;
				VM_stack_top[0].vector = (ML_Vector *) MEM_alloc(sizeof(ML_Vector));
				
				VM_stack_top[0].vector->x = 0.0F;
				VM_stack_top[0].vector->y = 0.0F;
				VM_stack_top[0].vector->z = 0.0F;

				break;

			case ML_DO_MATRIX:

				VM_POP_STACK(3);

				//
				// Make sure all the arguments are vectors.
				//

				if (VM_stack_top[0].type != ML_TYPE_VECTOR ||
					VM_stack_top[1].type != ML_TYPE_VECTOR ||
					VM_stack_top[2].type != ML_TYPE_VECTOR)
				{
					//
					// ERROR!
					//

					ASSERT(0);
				}
				
				{
					ML_Data res;

					res.type   = ML_TYPE_MATRIX;
					res.matrix = (ML_Matrix *) MEM_alloc(sizeof(ML_Matrix));

					res.matrix->vector[0] = *VM_stack_top[0].vector;
					res.matrix->vector[1] = *VM_stack_top[1].vector;
					res.matrix->vector[2] = *VM_stack_top[2].vector;

					VM_data_free(VM_stack_top[0]);
					VM_data_free(VM_stack_top[1]);
					VM_data_free(VM_stack_top[2]);

					VM_stack_top[0] = res;

					VM_stack_top++;
				}

				break;

			case ML_DO_VECTOR:

				VM_POP_STACK(3);

				{
					ML_Data res;

					res.type   = ML_TYPE_VECTOR;
					res.vector = (ML_Vector *) MEM_alloc(sizeof(ML_Vector));

					     if (VM_stack_top[0].type == ML_TYPE_FLUMBER) {res.vector->x =         VM_stack_top[0].flumber;}
					else if (VM_stack_top[0].type == ML_TYPE_SLUMBER) {res.vector->x = (float) VM_stack_top[0].slumber;}
					else
					{
						//
						// ERROR!
						//

						ASSERT(0);
					}

					     if (VM_stack_top[1].type == ML_TYPE_FLUMBER) {res.vector->y =         VM_stack_top[1].flumber;}
					else if (VM_stack_top[1].type == ML_TYPE_SLUMBER) {res.vector->y = (float) VM_stack_top[1].slumber;}
					else
					{
						//
						// ERROR!
						//

						ASSERT(0);
					}

					     if (VM_stack_top[2].type == ML_TYPE_FLUMBER) {res.vector->z =         VM_stack_top[2].flumber;}
					else if (VM_stack_top[2].type == ML_TYPE_SLUMBER) {res.vector->z = (float) VM_stack_top[2].slumber;}
					else
					{
						//
						// ERROR!
						//

						ASSERT(0);
					}

					//
					// No need to free the arguments... they are all just numbers.
					//

					VM_stack_top[0] = res;

					VM_stack_top++;
				}

				break;

			default:
				ASSERT(0);
				break;
		}
	}
}











void VM_run(CBYTE *fname)
{
	SLONG i;

	FILE *handle;

	handle = fopen(fname, "rb");

	if (handle == NULL)
	{
		fprintf(stderr, "Unable to open file \"%s\"\n", fname);

		return;
	}

	//
	// Load in the file header.
	//

	ML_Header mh;

	if (fread(&mh, sizeof(mh), 1, handle) != 1) {goto file_error;}

	//
	// Valid file?
	//

	if (mh.version != ML_VERSION_NUMBER)
	{
		//
		// This is an old file.
		//

		fprintf(stderr, "File \"%s\" has an old version number.\n", fname);
	
		fclose(handle);

		return;
	}

	//
	// Allocate instruction memory and read in the instructions
	// from the file.
	//

	ASSERT(mh.instructions_memory_in_bytes % sizeof(SLONG) == 0);

	VM_code_max     = (mh.instructions_memory_in_bytes + 32) / sizeof(SLONG);
	VM_code         = (SLONG *) malloc(sizeof(SLONG) * VM_code_max);
	VM_code_upto    = mh.instructions_memory_in_bytes / sizeof(SLONG);
	VM_code_pointer = VM_code;

	if (fread(VM_code, sizeof(UBYTE), mh.instructions_memory_in_bytes, handle) != mh.instructions_memory_in_bytes) goto file_error;
	
	//
	// Allocate static data and read in data from the file.
	//

	VM_data_max = mh.data_table_length_in_bytes;
	VM_data     = (UBYTE *) malloc(sizeof(UBYTE) * mh.data_table_length_in_bytes);

	if (fread(VM_data, sizeof(UBYTE), mh.data_table_length_in_bytes, handle) != mh.data_table_length_in_bytes) goto file_error;

	//
	// Allocate the stack.
	//

	VM_stack_max  = 2048;
	VM_stack      = (ML_Data *) malloc(sizeof(ML_Data) * VM_stack_max);
	VM_stack_top  = VM_stack;
	VM_stack_base = NULL;

	//
	// Allocate the globals and initialise all of them to undefined.
	//

	VM_global_max   = mh.num_globals + VM_EXTRA_GLOBAL_NUMBER;
	VM_global_extra = mh.num_globals;
	VM_global       = (ML_Data *) malloc(sizeof(ML_Data) * VM_global_max);

	memset(VM_global, 0, sizeof(ML_Data) * VM_global_max);

	//
	// Initialise the KEY[] array to FALSE.
	//

	for (i = 0; i < 256; i++)
	{
		VM_global[VM_EXTRA_GLOBAL_KEY(i)].type    = ML_TYPE_BOOLEAN;
		VM_global[VM_EXTRA_GLOBAL_KEY(i)].boolean = FALSE;
	}

	//
	// Initialise the flip tick.
	//

	VM_flip_tick = 0;

	//
	// Start execution!
	//

	VM_execute();

	return;

  file_error:;

	fclose(handle);

	if (VM_code)  {free(VM_code);  VM_code  = NULL;}
	if (VM_stack) {free(VM_stack); VM_stack = NULL;}

	return;
}
