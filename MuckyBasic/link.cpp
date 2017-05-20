//
// The linker
//

#include "always.h"
#include "ml.h"
#include "link.h"
#include "sysvar.h"
#include "st.h"






//
// The data we accumulate as we link.
// 

SLONG            *LINK_instruction;
SLONG             LINK_instruction_max;
SLONG             LINK_instruction_upto;

CBYTE            *LINK_table_data;
SLONG             LINK_table_data_max;
SLONG             LINK_table_data_upto;

LINK_Global      *LINK_global;
SLONG             LINK_global_max;
SLONG             LINK_global_upto;

LINK_Function    *LINK_function;
SLONG             LINK_function_max;
SLONG             LINK_function_upto;

LINK_Line        *LINK_line;
SLONG             LINK_line_max;
SLONG             LINK_line_upto;

LINK_Jump        *LINK_jump;
SLONG             LINK_jump_max;
SLONG             LINK_jump_upto;

LINK_Field       *LINK_field;
SLONG             LINK_field_max;
SLONG             LINK_field_upto;

LINK_Globalref   *LINK_global_ref;
SLONG             LINK_global_ref_max;
SLONG             LINK_global_ref_upto;

LINK_Undefref    *LINK_undef_ref;
SLONG             LINK_undef_ref_max;
SLONG             LINK_undef_ref_upto;

LINK_Fieldref     *LINK_field_ref;
SLONG              LINK_field_ref_max;
SLONG              LINK_field_ref_upto;

LINK_Datatableref *LINK_data_table_ref;
SLONG              LINK_data_table_ref_max;
SLONG              LINK_data_table_ref_upto;

CBYTE            *LINK_debug_data;
SLONG             LINK_debug_data_max;
SLONG             LINK_debug_data_upto;

//
// Each file.
//

typedef struct
{
	SLONG first_instruction;
	SLONG num_instructions;

	SLONG first_table_datum;
	SLONG num_table_data;

	SLONG first_global;
	SLONG num_globals;

	SLONG first_function;
	SLONG num_functions;

	SLONG first_line;
	SLONG num_lines;

	SLONG first_jump;
	SLONG num_jumps;

	SLONG first_field;
	SLONG num_fields;

	SLONG first_global_ref;
	SLONG num_global_refs;
	
	SLONG first_undef_ref;
	SLONG num_undef_refs;

	SLONG first_field_ref;
	SLONG num_field_refs;

	SLONG first_data_table_ref;
	SLONG num_data_table_refs;

	SLONG first_debug_datum;
	SLONG num_debug_data;

} LINK_File;

LINK_File *LINK_file;
SLONG      LINK_file_max;
SLONG      LINK_file_upto;



//
// The number of globals and fields in the executable.
// 

SLONG LINK_global_id_upto;
SLONG LINK_field_id_upto;








//
// Allocates initial memory.
//

void LINK_allocate_memory(void)
{
	LINK_instruction_max  = 1;
	LINK_instruction_upto = 0;
	LINK_instruction      = (SLONG *) malloc(sizeof(SLONG) * LINK_instruction_max);

	LINK_table_data_max  = 1;
	LINK_table_data_upto = 0;
	LINK_table_data      = (CBYTE *) malloc(sizeof(CBYTE) * LINK_table_data_max);

	LINK_global_max  = 1;
	LINK_global_upto = 0;
	LINK_global      = (LINK_Global *) malloc(sizeof(LINK_Global) * LINK_global_max);

	LINK_function_max  = 1;
	LINK_function_upto = 0;
	LINK_function      = (LINK_Function *) malloc(sizeof(LINK_Function) * LINK_function_max);

	LINK_line_max  = 1;
	LINK_line_upto = 0;
	LINK_line      = (LINK_Line *) malloc(sizeof(LINK_Line) * LINK_line_max);

	LINK_jump_max  = 1;
	LINK_jump_upto = 0;
	LINK_jump      = (LINK_Jump *) malloc(sizeof(LINK_Jump) * LINK_jump_max);

	LINK_field_max  = 1;
	LINK_field_upto = 0;
	LINK_field      = (LINK_Field *) malloc(sizeof(LINK_Field) * LINK_field_max);

	LINK_global_ref_max  = 1;
	LINK_global_ref_upto = 0;
	LINK_global_ref      = (LINK_Globalref *) malloc(sizeof(LINK_Globalref) * LINK_global_ref_max);

	LINK_undef_ref_max  = 1;
	LINK_undef_ref_upto = 0;
	LINK_undef_ref      = (LINK_Undefref *) malloc(sizeof(LINK_Undefref) * LINK_undef_ref_max);

	LINK_field_ref_max  = 1;
	LINK_field_ref_upto = 0;
	LINK_field_ref      = (LINK_Fieldref *) malloc(sizeof(LINK_Fieldref) * LINK_field_ref_max);

	LINK_data_table_ref_max  = 1;
	LINK_data_table_ref_upto = 0;
	LINK_data_table_ref      = (LINK_Datatableref *) malloc(sizeof(LINK_Datatableref) * LINK_data_table_ref_max);

	LINK_debug_data_max  = 1;
	LINK_debug_data_upto = 0;
	LINK_debug_data      = (CBYTE *) malloc(sizeof(CBYTE) * LINK_debug_data_max);

	LINK_file_max  = 1;
	LINK_file_upto = 0;
	LINK_file      = (LINK_File *) malloc(sizeof(LINK_File) * LINK_file_max);
	
}


//
// Frees all memory.
//

void LINK_free_memory(void)
{
	free(LINK_instruction   );
	free(LINK_table_data    );
	free(LINK_global        );
	free(LINK_function      );
	free(LINK_line          );
	free(LINK_jump          );
	free(LINK_field         );
	free(LINK_global_ref    );
	free(LINK_undef_ref     );
	free(LINK_field_ref     );
	free(LINK_data_table_ref);
	free(LINK_debug_data    );
	free(LINK_file          );

	LINK_instruction    = NULL;
	LINK_table_data     = NULL;
	LINK_global         = NULL;
	LINK_function       = NULL;
	LINK_line           = NULL;
	LINK_jump           = NULL;
	LINK_field          = NULL;
	LINK_global_ref     = NULL;
	LINK_undef_ref      = NULL;
	LINK_field_ref      = NULL;
	LINK_data_table_ref = NULL;
	LINK_debug_data     = NULL;
	LINK_file           = NULL;
}








SLONG LINK_do(CBYTE *object_fname[], SLONG num_object_files, CBYTE *exec_fname)
{
	SLONG i;
	SLONG j;
	SLONG instruction;
	SLONG magic;

	LINK_Header        lh;
	LINK_File         *lf;
	LINK_Function     *lc;
	LINK_Jump         *lj;
	LINK_Global       *lg;
	LINK_Field        *ld;
	LINK_Undefref     *lu;
	LINK_Datatableref *lt;
	FILE              *handle;

	#define LINK_MAGIC_CHECK() {if (fread(&magic, sizeof(SLONG), 1, handle) != 1) goto file_error; ASSERT(magic == 12345678);}

	//
	// Initialise all data.
	//

	LINK_allocate_memory();

	//
	// Load in each file.
	//

	for (i = 0; i < num_object_files; i++)
	{
		//
		// Open the file.
		//

		handle = fopen(object_fname[i], "rb");

		if (handle == NULL)
		{
			//
			// ERROR! Can't open object file...
			//

			goto file_error;
		}

		//
		// Load in the header.
		//

		if (fread(&lh, sizeof(LINK_Header), 1, handle) != 1) goto file_error;

		//
		// Correct version number?
		//

		if (lh.version != 1)
		{
			//
			// ERROR!
			//
		}

		//
		// Get another LINK_File.
		//

		if (LINK_file_upto >= LINK_file_max)
		{
			LINK_file_max *= 2;
			LINK_file      = (LINK_File *) realloc(LINK_file, sizeof(LINK_File) * LINK_file_max);
		}

		lf = &LINK_file[LINK_file_upto++];

		//
		// Initialise.
		//

		memset(lf, 0, sizeof(LINK_File));

		//
		// Instructions.
		//

		lf->first_instruction = LINK_instruction_upto;
		lf->num_instructions  = lh.num_instructions;

		while(LINK_instruction_upto + lh.num_instructions > LINK_instruction_max)
		{
			LINK_instruction_max *= 2;
			LINK_instruction      = (SLONG *) realloc(LINK_instruction, sizeof(SLONG) * LINK_instruction_max);
		}

		if (fread(LINK_instruction + LINK_instruction_upto, sizeof(SLONG), lh.num_instructions, handle) != lh.num_instructions) goto file_error;

		LINK_instruction_upto += lh.num_instructions;

		LINK_MAGIC_CHECK();

		//
		// The data table.
		//

		lf->first_table_datum = LINK_table_data_upto;
		lf->num_table_data    = lh.data_table_length_in_bytes;

		while(LINK_table_data_upto + lh.data_table_length_in_bytes > LINK_table_data_max)
		{
			LINK_table_data_max *= 2;
			LINK_table_data      = (CBYTE *) realloc(LINK_table_data, sizeof(CBYTE) * LINK_table_data_max);
		}

		if (fread(LINK_table_data + LINK_table_data_upto, sizeof(CBYTE), lh.data_table_length_in_bytes, handle) != lh.data_table_length_in_bytes) goto file_error;

		LINK_table_data_upto += lh.data_table_length_in_bytes;

		LINK_MAGIC_CHECK();

		//
		// Globals.
		//

		lf->first_global = LINK_global_upto;
		lf->num_globals  = lh.num_globals;

		while(LINK_global_upto + lh.num_globals > LINK_global_max)
		{
			LINK_global_max *= 2;
			LINK_global      = (LINK_Global *) realloc(LINK_global, sizeof(LINK_Global) * LINK_global_max);
		}

		if (fread(LINK_global + LINK_global_upto, sizeof(LINK_Global), lh.num_globals, handle) != lh.num_globals) goto file_error;

		LINK_global_upto += lh.num_globals;

		LINK_MAGIC_CHECK();

		//
		// Functions.
		//

		lf->first_function = LINK_function_upto;
		lf->num_functions  = lh.num_functions;

		while(LINK_function_upto + lh.num_functions > LINK_function_max)
		{
			LINK_function_max *= 2;
			LINK_function      = (LINK_Function *) realloc(LINK_function, sizeof(LINK_Function) * LINK_function_max);
		}

		if (fread(LINK_function + LINK_function_upto, sizeof(LINK_Function), lh.num_functions, handle) != lh.num_functions) goto file_error;

		LINK_function_upto += lh.num_functions;

		LINK_MAGIC_CHECK();

		//
		// Lines.
		//

		lf->first_line = LINK_line_upto;
		lf->num_lines  = lh.num_lines;

		while(LINK_line_upto + lh.num_lines > LINK_line_max)
		{
			LINK_line_max *= 2;
			LINK_line      = (LINK_Line *) realloc(LINK_line, sizeof(LINK_Line) * LINK_line_max);
		}

		if (fread(LINK_line + LINK_line_upto, sizeof(LINK_Line), lh.num_lines, handle) != lh.num_lines) goto file_error;

		LINK_line_upto += lh.num_lines;

		LINK_MAGIC_CHECK();

		//
		// Jumps.
		//

		lf->first_jump = LINK_jump_upto;
		lf->num_jumps  = lh.num_jumps;

		while(LINK_jump_upto + lh.num_jumps > LINK_jump_max)
		{
			LINK_jump_max *= 2;
			LINK_jump      = (LINK_Jump *) realloc(LINK_jump, sizeof(LINK_Jump) * LINK_jump_max);
		}

		if (fread(LINK_jump + LINK_jump_upto, sizeof(LINK_Jump), lh.num_jumps, handle) != lh.num_jumps) goto file_error;

		LINK_jump_upto += lh.num_jumps;

		LINK_MAGIC_CHECK();

		//
		// Fields.
		//

		lf->first_field = LINK_field_upto;
		lf->num_fields  = lh.num_fields;

		while(LINK_field_upto + lh.num_fields > LINK_field_max)
		{
			LINK_field_max *= 2;
			LINK_field      = (LINK_Field *) realloc(LINK_field, sizeof(LINK_Field) * LINK_field_max);
		}

		if (fread(LINK_field + LINK_field_upto, sizeof(LINK_Field), lh.num_fields, handle) != lh.num_fields) goto file_error;

		LINK_field_upto += lh.num_fields;

		LINK_MAGIC_CHECK();

		//
		// Globalrefs.
		//

		lf->first_global_ref = LINK_global_ref_upto;
		lf->num_global_refs  = lh.num_global_refs;

		while(LINK_global_ref_upto + lh.num_global_refs > LINK_global_ref_max)
		{
			LINK_global_ref_max *= 2;
			LINK_global_ref      = (LINK_Globalref *) realloc(LINK_global_ref, sizeof(LINK_Globalref) * LINK_global_ref_max);
		}

		if (fread(LINK_global_ref + LINK_global_ref_upto, sizeof(LINK_Globalref), lh.num_global_refs, handle) != lh.num_global_refs) goto file_error;

		LINK_global_ref_upto += lh.num_global_refs;

		LINK_MAGIC_CHECK();

		//
		// Undefined function references.
		//

		lf->first_undef_ref = LINK_undef_ref_upto;
		lf->num_undef_refs  = lh.num_undef_refs;

		while(LINK_undef_ref_upto + lh.num_undef_refs > LINK_undef_ref_max)
		{
			LINK_undef_ref_max *= 2;
			LINK_undef_ref      = (LINK_Undefref *) realloc(LINK_undef_ref, sizeof(LINK_Undefref) * LINK_undef_ref_max);
		}

		if (fread(LINK_undef_ref + LINK_undef_ref_upto, sizeof(LINK_Undefref), lh.num_undef_refs, handle) != lh.num_undef_refs) goto file_error;

		LINK_undef_ref_upto += lh.num_undef_refs;

		LINK_MAGIC_CHECK();

		//
		// Fieldrefs.
		//

		lf->first_field_ref = LINK_field_ref_upto;
		lf->num_field_refs  = lh.num_field_refs;

		while(LINK_field_ref_upto + lh.num_field_refs > LINK_field_ref_max)
		{
			LINK_field_ref_max *= 2;
			LINK_field_ref      = (LINK_Fieldref *) realloc(LINK_field_ref, sizeof(LINK_Fieldref) * LINK_field_ref_max);
		}

		if (fread(LINK_field_ref + LINK_field_ref_upto, sizeof(LINK_Fieldref), lh.num_field_refs, handle) != lh.num_field_refs) goto file_error;

		LINK_field_ref_upto += lh.num_field_refs;

		LINK_MAGIC_CHECK();

		//
		// Data table refs.
		//

		lf->first_data_table_ref = LINK_data_table_ref_upto;
		lf->num_data_table_refs  = lh.num_data_table_refs;

		while(LINK_data_table_ref_upto + lh.num_data_table_refs > LINK_data_table_ref_max)
		{
			LINK_data_table_ref_max *= 2;
			LINK_data_table_ref      = (LINK_Datatableref *) realloc(LINK_data_table_ref, sizeof(LINK_Datatableref) * LINK_data_table_ref_max);
		}

		if (fread(LINK_data_table_ref + LINK_data_table_ref_upto, sizeof(LINK_Datatableref), lh.num_data_table_refs, handle) != lh.num_data_table_refs) goto file_error;

		LINK_data_table_ref_upto += lh.num_data_table_refs;

		LINK_MAGIC_CHECK();

		//
		// Load in the debug data.
		//

		lf->first_debug_datum = LINK_debug_data_upto;

		if (fread(&lf->num_debug_data, sizeof(SLONG), 1, handle) != 1) goto file_error;

		while(LINK_debug_data_upto + lf->num_debug_data > LINK_debug_data_max)
		{
			LINK_debug_data_max *= 2;
			LINK_debug_data      = (CBYTE *) realloc(LINK_debug_data, sizeof(CBYTE) * LINK_debug_data_max);
		}

		if (fread(LINK_debug_data + LINK_debug_data_upto, sizeof(CBYTE), lf->num_debug_data, handle) != lf->num_debug_data) goto file_error;

		LINK_debug_data_upto += lf->num_debug_data;

		//
		// Finished. We should have read the entire file.
		//

		#ifdef _DEBUG

		//
		// Make sure there is no file left!
		//

		{
			UBYTE junk[1024];
			SLONG read;

			read = fread(junk, 1, 1024, handle);

			ASSERT(read == 0);
		}

		#endif

		//
		// Finished reading the file.
		//

		fclose(handle);
	}

	//
	// Add all exported globals, fields and exported functions
	// to the symbol table. This helps us allocate global_ids
	// and field_ids and resolve calls to functions across files.
	//

	ST_clear_all();
	SYSVAR_init();

	LINK_global_id_upto = 0;
	LINK_field_id_upto  = SYSVAR_FIELD_NUMBER;

	for (i = 0; i < LINK_file_upto; i++)
	{
		lf = &LINK_file[i];

		//
		// Overwrite each global's index field with the new global_id.
		//

		for (j = 0; j < lf->num_globals; j++)
		{
			lg = &LINK_global[lf->first_global + j];

			if (lg->export)
			{
				ASSERT(WITHIN(lg->name + lf->first_debug_datum, 0, LINK_debug_data_upto - 1));

				//
				// Is this global already in the symbol table?
				//

				if (ST_find(LINK_debug_data + lg->name + lf->first_debug_datum))
				{
					//
					// ERROR! Multiply defined exported global.
					//

					ASSERT(0);
				}
				else
				{
					//
					// The global wasn't found. Allocate a new global_id and add
					// this global to the symbol table.
					//

					lg->index = LINK_global_id_upto++;

					ST_add(ST_TABLE_GLOBAL, LINK_debug_data + lg->name + lf->first_debug_datum, lg->index, 0);
				}
			}
			else
			{
				ASSERT(WITHIN(lg->name + lf->first_debug_datum, 0, LINK_debug_data_upto - 1));

				if (lg->local)
				{
					//
					// This global is local for sure, so it needs it's own ID.
					//

					lg->index = LINK_global_id_upto++;
				}
				else
				{
					//
					// Is this global already in the symbol table?
					//

					if (ST_find(LINK_debug_data + lg->name + lf->first_debug_datum))
					{
						//
						// Use the same global id.
						//

						lg->index = ST_found_value;
					}
					else
					{
						//
						// Allocate a new global_id.
						//

						lg->index = LINK_global_id_upto++;
					}
				}
			}
		}

		//
		// Overwrite each field's index field with the new field_id.
		//

		for (j = 0; j < lf->num_fields; j++)
		{	
			ld = &LINK_field[lf->first_field + j];

			ASSERT(WITHIN(ld->name + lf->first_debug_datum, 0, LINK_debug_data_upto - 1));

			//
			// Is this field already in the symbol table?
			//

			if (ST_find(LINK_debug_data + ld->name + lf->first_debug_datum))
			{
				//
				// Use the same field_id.
				//

				ld->index = ST_found_value;
			}
			else
			{
				//
				// Allocate a new field_id and add it to the symbol table.
				//

				ld->index = LINK_field_id_upto++;

				ST_add(ST_TABLE_GLOBAL, LINK_debug_data + ld->name + lf->first_debug_datum, ld->index, 0);
			}
		}

		//
		// Add all the exported functions to the symbol table.
		//

		for (j = 0; j < lf->num_functions; j++)
		{
			lc = &LINK_function[lf->first_function + j];

			if (lc->export)
			{
				ASSERT(WITHIN(lc->name + lf->first_debug_datum, 0, LINK_debug_data_upto - 1));

				//
				// The value associated with the function is the first instruction
				// of the function body.
				//

				if (ST_find(LINK_debug_data + lc->name + lf->first_debug_datum))
				{
					//
					// ERROR! Multiply defined function!
					//

					ASSERT(0);
				}
				else
				{
					//
					// What is the first instruction of the function body?
					//

					ASSERT(WITHIN(lc->line_start, 0, lf->num_lines - 1));
					ASSERT(WITHIN(lc->line_start + lf->first_line, 0, LINK_line_upto - 1));

					instruction = LINK_line[lf->first_line + lc->line_start].instruction;

					ASSERT(WITHIN(instruction, 0, lf->num_instructions - 1));

					instruction += lf->first_instruction;

					ASSERT(WITHIN(instruction, 0, LINK_instruction_upto - 1));
				
					//
					// The first two instructions of a function are a GOTO to the
					// end of the funciton. This is so that execution will jump
					// over the function instead of leaking into it accidentally.
					//

					instruction += 2;

					//
					// Add the function to the symbol table.
					//

					ST_add(
						ST_TABLE_GLOBAL,
						LINK_debug_data + lc->name + lf->first_debug_datum,
						instruction,
						0);
				}
			}
		}
	}

	//
	// Do the actual linking.
	//

	for (i = 0; i < LINK_file_upto; i++)
	{
		lf = &LINK_file[i];

		//
		// Fix instructions that refer to globals.
		//

		for (j = 0; j < lf->num_global_refs; j++)
		{
			ASSERT(WITHIN(lf->first_global_ref + j, 0, LINK_global_ref_upto - 1));

			instruction  = LINK_global_ref[lf->first_global_ref + j].instruction;
			instruction += lf->first_instruction;

			ASSERT(WITHIN(instruction, 0, LINK_instruction_upto - 1));

			//
			// We have the instruction that contains the old global_id.
			// Overwrite it with the new one.
			//

			ASSERT(WITHIN(LINK_instruction[instruction], 0, lf->num_globals - 1));

			LINK_instruction[instruction] = LINK_global[lf->first_global + LINK_instruction[instruction]].index;
		}

		//
		// Fix instructions that refer to fields.
		//

		for (j = 0; j < lf->num_field_refs; j++)
		{
			ASSERT(WITHIN(lf->first_field_ref + j, 0, LINK_field_ref_upto - 1));

			instruction  = LINK_field_ref[lf->first_field_ref + j].instruction;
			instruction += lf->first_instruction;

			ASSERT(WITHIN(instruction, 0, LINK_instruction_upto - 1));

			//
			// We have the instruction that contains the old field_id.
			// Overwrite it with the new one.
			//

			ASSERT(WITHIN(LINK_instruction[instruction], 0, lf->num_fields - 1));

			LINK_instruction[instruction] = LINK_field[lf->first_field + LINK_instruction[instruction]].index;
		}

		//
		// Fix calls to functions across files.
		//

		for (j = 0; j < lf->num_undef_refs; j++)
		{
			ASSERT(WITHIN(lf->first_undef_ref + j, 0, LINK_undef_ref_upto - 1));

			lu = &LINK_undef_ref[lf->first_undef_ref + j];

			//
			// Look for this undefined function in the symbol table.
			//

			ASSERT(WITHIN(lu->name + lf->first_debug_datum, 0, LINK_debug_data_upto - 1));

			if (!ST_find(LINK_debug_data + lu->name + lf->first_debug_datum))
			{
				//
				// ERROR! Undefined function.
				//

				ASSERT(0);
			}
			else
			{
				//
				// Write the address of the function into the instructions.
				//

				ASSERT(WITHIN(lu->instruction, 0, lf->num_instructions - 1));
				ASSERT(WITHIN(lf->first_instruction + lu->instruction, 0, LINK_instruction_upto - 1));
				ASSERT(WITHIN(ST_found_value, 0, LINK_instruction_upto - 1));

				LINK_instruction[lf->first_instruction + lu->instruction] = ST_found_value;
			}
		}

		//
		// Fix jumps.
		//

		for (j = 0; j < lf->num_jumps; j++)
		{
			ASSERT(WITHIN(lf->first_jump + j, 0, LINK_jump_upto - 1));
			
			lj = &LINK_jump[lf->first_jump + j];

			ASSERT(WITHIN(lj->instruction, 0, lf->num_instructions - 1));
			ASSERT(WITHIN(lj->instruction + lf->first_instruction, 0, LINK_instruction_upto - 1));

			LINK_instruction[lf->first_instruction + lj->instruction] += lf->first_instruction;
		}

		//
		// Fix references into the data table.
		//

		for (j = 0; j < lf->num_data_table_refs; j++)
		{
			ASSERT(WITHIN(lf->first_data_table_ref + j, 0, LINK_data_table_ref_upto - 1));

			lt = &LINK_data_table_ref[lf->first_data_table_ref + j];

			ASSERT(WITHIN(lt->instruction, 0, lf->num_instructions - 1));
			ASSERT(WITHIN(lt->instruction + lf->first_instruction, 0, LINK_instruction_upto - 1));
			ASSERT(WITHIN(LINK_instruction[lt->instruction + lf->first_instruction], 0, lf->num_table_data - 1));

			LINK_instruction[lt->instruction + lf->first_instruction] += lf->first_table_datum;

			ASSERT(WITHIN(LINK_instruction[lt->instruction + lf->first_instruction], 0, LINK_table_data_upto - 1));
		}

		//
		// The last instruction of any file is an EXIT. Replace all these
		// EXIT instrucitons with NOPs except for the last one.
		//

		if (i == LINK_file_upto - 1)
		{
			//
			// This is the last file.
			//
		}
		else
		{
			ASSERT(WITHIN(lf->first_instruction + lf->num_instructions - 1, 0, LINK_instruction_upto - 1));
			ASSERT(LINK_instruction[lf->first_instruction + lf->num_instructions - 1] == ML_DO_EXIT);

			LINK_instruction[lf->first_instruction + lf->num_instructions - 1] = ML_DO_NOP;
		}
	}

	//
	// Ready to write out the executable now. Create the
	// header of an executable.
	//

	ML_Header mh;

	mh.version                      = ML_VERSION_NUMBER;
	mh.instructions_memory_in_bytes = LINK_instruction_upto * sizeof(SLONG);
	mh.data_table_length_in_bytes   = LINK_table_data_upto;
	mh.num_globals                  = LINK_global_upto;

	//
	// Open the executable file.
	//

	handle = fopen(exec_fname, "wb");

	if (handle == NULL)
	{
		//
		// ERROR!
		//

		goto file_error;
	}

	if (fwrite(&mh, sizeof(mh), 1, handle) != 1) goto file_error;

	//
	// The instructions.
	//

	if (fwrite(LINK_instruction, sizeof(SLONG), LINK_instruction_upto, handle) != LINK_instruction_upto) goto file_error;

	//
	// The string table.
	//

	if (fwrite(LINK_table_data, sizeof(UBYTE), LINK_table_data_upto, handle) != LINK_table_data_upto) goto file_error;

	fclose(handle);

	//
	// All done.
	//

	LINK_free_memory();

	return TRUE;

  file_error:;

	//
	// ERROR!
	//

	if (handle)
	{
		fclose(handle);
	}

	LINK_free_memory();

	return FALSE;
}




