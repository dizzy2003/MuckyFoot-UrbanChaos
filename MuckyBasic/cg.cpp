//
// The code generator. Converts the output of the parse into
// an executable.
//

#include "always.h"
#include "cg.h"
#include "lex.h"
#include "link.h"
#include "ml.h"
#include "parse.h"
#include "sysvar.h"
#include "st.h"



//
// The result of code generation.
//

CBYTE *CG_output;
SLONG  CG_num_errors;
SLONG  CG_num_warnings;

//
// The instructions
//

#define CG_MAX_INSTRUCTIONS 65536

SLONG CG_instruction[CG_MAX_INSTRUCTIONS];
SLONG CG_instruction_upto;


//
// The line we are currently generating code for.
//

SLONG CG_generating_line;




//
// The functions.
//

typedef struct
{
	CBYTE *name;		// The function name...
	SLONG  debug_name;	// Index into the debug data.
	SLONG  num_args;
	SLONG  arg_upto;
	SLONG  line;		// The entrypoint of the function.
	SLONG  endline;		// The last line of the function.
	SLONG  num_locals;	// How many of the args are actually locals!

} CG_Func;

#define CG_MAX_FUNCS 4096

CG_Func CG_func[CG_MAX_FUNCS];
SLONG   CG_func_upto;



//
// For each line of code.
//

typedef struct
{
	SLONG code;	// Index into the instruction array. Where each line's code is.
 
} CG_Line;

#define CG_MAX_LINES 16384

CG_Line CG_line[CG_MAX_LINES];
SLONG   CG_line_upto;


//
// Line numbers that must be converted to instruction
// indices.
//

#define CG_FORWARD_GOTO_TYPE_GOTO  0
#define CG_FORWARD_GOTO_TYPE_GOSUB 1
#define CG_FORWARD_GOTO_TYPE_CALL  2

typedef struct
{
	SLONG type;
	SLONG instruction;
 
} CG_Forward_goto;

#define CG_MAX_FORWARD_GOTOS 4096

CG_Forward_goto CG_forward_goto[CG_MAX_FORWARD_GOTOS];
SLONG           CG_forward_goto_upto;


//
// Single-line IF statements.
//

typedef struct
{
	SLONG ifcode;
	SLONG instruction;	// The instruction containing the forward jump of the IF condition that jumps over the THEN statement code.

} CG_If;

#define CG_MAX_IFS_PER_LINE 256

CG_If CG_if[CG_MAX_IFS_PER_LINE];
SLONG CG_if_upto;

//
// Multi-line IF statements.
//

#define CG_MIF_FLAG_FOUND_MELSE  (1 << 0)
#define CG_MIF_FLAG_FOUND_MENDIF (1 << 1)

typedef struct
{
	ULONG flag;
	SLONG iffalsejump;
	SLONG afterthenjump;

} CG_Mif;

#define CG_MAX_MIFS 4096

CG_Mif CG_mif[CG_MAX_MIFS];
SLONG  CG_mif_upto;



//
// All the FOR-NEXT loops...
//

#define CG_FOR_TYPE_ANON   0	// No lvalue associated with this FOR-NEXT loop.
#define CG_FOR_TYPE_LVALUE 1

typedef struct
{
	SLONG       type;
	PARSE_Node *lvalue;		// The lvalue that this FOR-NEXT loop is acting on
	SLONG       loopto;		// The instruction that corresponding NEXT statements should jump to.
	SLONG       overstep;	// The instruction which should contain the address of the instruction after the STEP statement.
	SLONG       forcode;	// The unique code identifying this FOR loop.
	SLONG       afternext;	// The instruction which should contain the address of the instruction after the NEXT statement.

} CG_For;

#define CG_MAX_FORS 4096

CG_For CG_for[CG_MAX_FORS];
SLONG  CG_for_upto;

//
// The while loops.
//

#define CG_WHILE_FLAG_FOUND_LOOP (1 << 0)

typedef struct
{
	SLONG flag;
	SLONG iffalsejump;
	SLONG whilecode;
	SLONG loopto;

} CG_While;

#define CG_MAX_WHILES 4096

CG_While CG_while[CG_MAX_WHILES];
SLONG    CG_while_upto;

//
// The number of globals used by the program.
//

SLONG CG_global_upto;

//
// The number of field_ids used so far.
//

SLONG CG_field_id_upto;


//
// The string table.
//

SLONG  CG_data_table_max;
UBYTE *CG_data_table;
SLONG  CG_data_table_upto;

//
// The function we are currently generating code for.
//

SLONG CG_in_function;


//
// Every jump instruction's target address.
//

typedef struct
{
	SLONG instruction;

} CG_Jump;

#define CG_MAX_JUMPS 16384

CG_Jump CG_jump[CG_MAX_JUMPS];
SLONG   CG_jump_upto;


//
// Every reference to a global.
//

typedef struct
{
	SLONG instruction;

} CG_Globalref;

#define CG_MAX_GLOBALREFS 16384

CG_Globalref CG_globalref[CG_MAX_GLOBALREFS];
SLONG        CG_globalref_upto;


//
// Every refrence to a field.
//

typedef struct
{
	SLONG instruction;

} CG_Fieldref;

#define CG_MAX_FIELDREFS 16384

CG_Fieldref CG_fieldref[CG_MAX_FIELDREFS];
SLONG       CG_fieldref_upto;


//
// Every reference to an undefined function.
//

typedef struct
{
	SLONG name;				// Index into the debug data array.
	SLONG instruction;

} CG_Undefref;

#define CG_MAX_UNDEFREFS 16384

CG_Undefref CG_undefref[CG_MAX_UNDEFREFS];
SLONG       CG_undefref_upto;



//
// Every reference into the data table.
//

typedef struct
{
	SLONG instruction;

} CG_Datatableref;

#define CG_MAX_DATATABLEREFS 16384

CG_Datatableref CG_datatableref[CG_MAX_DATATABLEREFS];
SLONG           CG_datatableref_upto;


//
// Debug data.
//

#define CG_MAX_DEBUG_DATA 65536

CBYTE CG_debug_data[CG_MAX_DEBUG_DATA];
SLONG CG_debug_data_upto;




//
// Adds a string to the debug data and returns
// an index to it.
//

SLONG CG_add_string_to_debug_data(CBYTE *string)
{
	SLONG length = strlen(string) + 1;	// + 1 to include the terminating NULL
	
	if (CG_debug_data_upto + length > CG_MAX_DEBUG_DATA)
	{
		//
		// ERROR!
		//

		ASSERT(0);
	}

	SLONG ans = CG_debug_data_upto;

	strcpy(CG_debug_data + CG_debug_data_upto, string);

	CG_debug_data_upto += length;

	return ans;
	
}






//
// Adds a string to the string table and returns the index
// into the string table for where it was stored.
//

SLONG CG_add_string(CBYTE *string)
{
	SLONG length = strlen(string) + 1;	// + 1 to include the terminating NULL.

	if (CG_data_table_upto + length > CG_data_table_max)
	{
		CG_data_table_max *= 2;
		CG_data_table      = (UBYTE *) realloc(CG_data_table, sizeof(UBYTE) * CG_data_table_max);
	}

	SLONG ans = CG_data_table_upto;

	strcpy(((CBYTE *) CG_data_table) + CG_data_table_upto, string);

	CG_data_table_upto += length;

	return ans;
}



//
// The callback function for adding labels and variables to the
// symbol table.
//

SLONG CG_callback_add_labels_and_variables(PARSE_Node *pn)
{
	switch(pn->type)
	{
		case PARSE_NODE_TYPE_LABEL:

			if (ST_find(pn->label))
			{
				//
				// ERROR!
				//

				ASSERT(0);

				return FALSE;
			}
			else
			{
				ST_add(ST_TABLE_GLOBAL, pn->label, CG_generating_line, 0);
			}

			break;

		case PARSE_NODE_TYPE_VAR_ADDRESS:
		case PARSE_NODE_TYPE_VAR_VALUE:

			if (ST_find(pn->variable))
			{
				//
				// We've already added this variable to the symbol table
				// and assigned it a 'global' memory number.
				//
			}
			else
			{
				ST_add(ST_TABLE_GLOBAL, pn->variable, CG_global_upto++, 0);
			}

			break;

		case PARSE_NODE_TYPE_FIELD:
			
			if (ST_find(pn->field))
			{
				//
				// Already assigned this structure field a field_id.
				//
			}
			else
			{
				ST_add(ST_TABLE_GLOBAL, pn->field, CG_field_id_upto++, 0);
			}

			break;

		case PARSE_NODE_TYPE_FUNCTION:

			if (CG_in_function)
			{
				//
				// ERROR! Two function definitions in a row without an ENDFUNC.
				//

				ASSERT(0);
			}

			CG_in_function = CG_func_upto;

			//
			// Allocate a new function structure.
			//

			ASSERT(WITHIN(CG_func_upto, 1, CG_MAX_FUNCS - 1));

			CG_func[CG_func_upto].name     = pn->variable;
			CG_func[CG_func_upto].num_args = 0;
			CG_func[CG_func_upto].line     = CG_generating_line;

			//
			// Add the function to the string table.
			//

			ST_add(ST_TABLE_GLOBAL, pn->variable, CG_func_upto, 0);

			CG_func_upto += 1;

			break;

		case PARSE_NODE_TYPE_ARGUMENT:
			
			//
			// We must be inside a function!
			//

			if (!CG_in_function)
			{
				//
				// ERROR!
				//

				ASSERT(0);
			}

			ASSERT(CG_in_function == CG_func_upto - 1);

			//
			// Add this argument to the local variables for this function.
			//

			if (ST_find(pn->variable))
			{
				if (ST_found_table != ST_TABLE_LOCAL)
				{
					//
					// Add this variable to the local symbol table.
					//

					ST_add(ST_TABLE_LOCAL, pn->variable, 0, 0);

					CG_func[CG_in_function].num_args += 1;
				}
				else
				if (ST_found_table == ST_TABLE_LOCAL)
				{
					//
					// ERROR! Mutliple defined arguements!
					//

					ASSERT(0);
				}
			}
			else
			{
				//
				// Add this variable to the local symbol table.
				//

				ST_add(ST_TABLE_LOCAL, pn->variable, 0, 0);

				CG_func[CG_in_function].num_args += 1;
			}

			break;

		case PARSE_NODE_TYPE_ENDFUNC:

			//
			// Remember the line where this function ends.
			//

			ASSERT(WITHIN(CG_in_function, 1, CG_MAX_FUNCS - 1));

			CG_func[CG_in_function].endline = CG_generating_line;

			//
			// Not in a function any more.
			//

			CG_in_function = FALSE;

			//
			// We've lost all the symbols in the local symbol table.
			//

			ST_clear(ST_TABLE_LOCAL);

			break;

		case PARSE_NODE_TYPE_EXPORT:
			
			//
			// Add this variable (manged with a "->" on the front) to
			// the symbol, to denote that it should be exported in the
			// object file.
			//

			{
				CBYTE export[LEX_MAX_STRING_LENGTH + 2];

				sprintf(export, "->%s", pn->variable);

				ST_add(ST_TABLE_GLOBAL, export, 0, 0);
			}

			break;

		case PARSE_NODE_TYPE_LOCAL:

			if (CG_in_function)
			{
				//
				// Add this local to the function.
				//

				ASSERT(WITHIN(CG_in_function, 1, CG_MAX_FUNCS - 1));

				if (ST_find(pn->variable))
				{
					if (ST_found_table == ST_TABLE_LOCAL)
					{
						//
						// ERROR! Multiply defined locals/args
						// 

						ASSERT(0);
					}
					else
					{
						//
						// Add this local to the funcion.
						//

						CG_func[CG_in_function].num_args   += 1;
						CG_func[CG_in_function].num_locals += 1;

						//
						// Add this variable to the local symbol table.
						//

						ST_add(ST_TABLE_LOCAL, pn->variable, 0, 0);
					}
				}
				else
				{
					//
					// Add this local to the funcion.
					//

					CG_func[CG_in_function].num_args   += 1;
					CG_func[CG_in_function].num_locals += 1;

					//
					// Add this variable to the local symbol table.
					//

					ST_add(ST_TABLE_LOCAL, pn->variable, 0, 0);
				}
			}
			else
			{
				//
				// A local declaration outside of a function 
				// means that the variable is local to the file.
				// Add it to the symbol table with "<-" on the front
				// if it.
				//

				CBYTE local[LEX_MAX_STRING_LENGTH + 2];

				sprintf(local, "<-%s", pn->variable);

				ST_add(ST_TABLE_GLOBAL, local, 0, 0);
			}

			break;

		default:
			break;
	}

	return TRUE;
}


//
// The code generation callback function.
//

SLONG CG_callback_generate_code(PARSE_Node *pn)
{
	//
	// Always make sure we have 3 SLONGS worth of instructions spare.
	//

	if (!WITHIN(CG_instruction_upto, 0, CG_MAX_INSTRUCTIONS - 3))
	{	
		//
		// Out of memory!
		//

		return FALSE;
	}

	switch(pn->type)
	{
		case PARSE_NODE_TYPE_NOP:
			break;

		case PARSE_NODE_TYPE_EQUALS:
			CG_instruction[CG_instruction_upto++] = ML_DO_EQUALS;
			break;

		case PARSE_NODE_TYPE_PLUS:
			CG_instruction[CG_instruction_upto++] = ML_DO_ADD;
			break;

		case PARSE_NODE_TYPE_MINUS:
			CG_instruction[CG_instruction_upto++] = ML_DO_MINUS;
			break;

		case PARSE_NODE_TYPE_UMINUS:
			CG_instruction[CG_instruction_upto++] = ML_DO_UMINUS;
			break;

		case PARSE_NODE_TYPE_TIMES:
			CG_instruction[CG_instruction_upto++] = ML_DO_TIMES;
			break;

		case PARSE_NODE_TYPE_DIVIDE:
			CG_instruction[CG_instruction_upto++] = ML_DO_DIVIDE;
			break;

		case PARSE_NODE_TYPE_MOD:
			CG_instruction[CG_instruction_upto++] = ML_DO_MOD;
			break;

		case PARSE_NODE_TYPE_SLUMBER:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_SLUMBER;
			CG_instruction[CG_instruction_upto++] = pn->slumber;
			break;

		case PARSE_NODE_TYPE_FLUMBER:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_FLUMBER;
			((float *) CG_instruction)[CG_instruction_upto++] = pn->flumber;
			break;

		case PARSE_NODE_TYPE_STRING:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_STRCONST;
			CG_instruction[CG_instruction_upto++] = CG_add_string(pn->string);

			//
			// Remember the reference into the data table.
			//

			ASSERT(WITHIN(CG_datatableref_upto, 0, CG_MAX_DATATABLEREFS - 1));

			CG_datatableref[CG_datatableref_upto++].instruction = CG_instruction_upto - 1;

			break;

		case PARSE_NODE_TYPE_VAR_VALUE:

			if (!ST_find(pn->variable))
			{
				ASSERT(0);
			}
			else
			{
				SLONG local = (ST_found_table == ST_TABLE_LOCAL);
				SLONG quick = (pn->flag & PARSE_NODE_FLAG_EXTRACT);
				SLONG instruction;

				//
				// Can't be both these types...
				//

				if (local)
				{
					if (quick)
					{
						instruction = ML_DO_PUSH_LOCAL_QUICK;
					}
					else
					{
						instruction = ML_DO_PUSH_LOCAL_VALUE;
					}
				}
				else
				{
					if (quick)
					{
						instruction = ML_DO_PUSH_GLOBAL_QUICK;
					}
					else
					{
						instruction = ML_DO_PUSH_GLOBAL_VALUE;
					}

					//
					// Remember the reference to the global.
					//

					ASSERT(WITHIN(CG_globalref_upto, 0, CG_MAX_GLOBALREFS - 1));

					CG_globalref[CG_globalref_upto++].instruction = CG_instruction_upto + 1;
				}

				CG_instruction[CG_instruction_upto++] = instruction;
				CG_instruction[CG_instruction_upto++] = ST_found_value;
			}

			break;

		case PARSE_NODE_TYPE_IF:

			//
			// Store where this if instruction is so we can set the
			// if condition goto to be after the 'then' code.
			// 

			ASSERT(WITHIN(CG_if_upto, 0, CG_MAX_IFS_PER_LINE - 1));

			CG_if[CG_if_upto].ifcode      = pn->ifcode;
			CG_if[CG_if_upto].instruction = CG_instruction_upto + 1;

			CG_if_upto++;

			CG_instruction[CG_instruction_upto++] = ML_DO_IF_FALSE_GOTO;
			CG_instruction[CG_instruction_upto++] = 0;	// To be filled in when we get the corresponding PARSE_NODE_TYPE_FAKE_THEN node.

			//
			// Remember the jump instruction.
			//

			ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

			CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

			break;

		case PARSE_NODE_TYPE_FAKE_THEN:

			//
			// Look for the corresponding IF.
			//

			{
				SLONG i;

				for (i = 0; i < CG_if_upto; i++)
				{
					if (CG_if[i].ifcode == pn->ifcode)
					{
						//
						// Found it! Make the IF_FALSE_GOTO point to the current instruction.
						//

						ASSERT(WITHIN(CG_if[i].instruction, 0, CG_instruction_upto - 1));

						CG_instruction[CG_if[i].instruction] = CG_instruction_upto;

						break;
					}
				}
				
				//
				// THEN found without matching IF?
				//

				ASSERT(i != CG_if_upto);
			}

			break;

		case PARSE_NODE_TYPE_FAKE_ELSE:

			//
			// Look for the corresponding IF.
			//

			{
				SLONG i;

				for (i = 0; i < CG_if_upto; i++)
				{
					if (CG_if[i].ifcode == pn->ifcode)
					{
						//
						// Found it! This IF_FALSE_GOTO instruction must
						// jump an instruction further to skip over the
						// goto we are just about to put in...
						//

						ASSERT(WITHIN(CG_if[i].instruction, 0, CG_instruction_upto - 1));

						CG_instruction[CG_if[i].instruction] = CG_instruction_upto + 2;
						
						//
						// Insert a GOTO so that the THEN code doesn't
						// leak into the ELSE code.
						//

						CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;
						CG_instruction[CG_instruction_upto++] = 0;	// To be filled in when we get the corresponding PARSE_NODE_TYPE_FAKE_ENDIF node.

						CG_if[i].instruction = CG_instruction_upto - 1;

						//
						// Remember where the jump instruction is.
						//

						ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

						CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

						break;
					}
				}
				
				//
				// ELSE found without matching IF?
				//

				ASSERT(i != CG_if_upto);
			}

			break;

		case PARSE_NODE_TYPE_FAKE_END_ELSE:

			//
			// Look for the corresponding IF.
			//

			{
				SLONG i;

				for (i = 0; i < CG_if_upto; i++)
				{
					if (CG_if[i].ifcode == pn->ifcode)
					{
						//
						// Found it! Make the GOTO after the 'THEN' code point to the current instruction.
						//

						ASSERT(WITHIN(CG_if[i].instruction, 0, CG_instruction_upto - 1));

						CG_instruction[CG_if[i].instruction] = CG_instruction_upto;

						break;
					}
				}
				
				//
				// ENDELSE found without matching IF?
				//

				ASSERT(i != CG_if_upto);

			}

			break;

		case PARSE_NODE_TYPE_GOTO:
		case PARSE_NODE_TYPE_GOSUB:

			if (!ST_find(pn->label))
			{
				//
				// ERROR!
				//

				ASSERT(0);

				return FALSE;
			}
			else
			{
				switch(pn->type)
				{
					case PARSE_NODE_TYPE_GOTO:  CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;  break;
					case PARSE_NODE_TYPE_GOSUB: CG_instruction[CG_instruction_upto++] = ML_DO_GOSUB; break;

					default:
						ASSERT(0);
						break;
				}

				//
				// If this is a backwards goto or gosub then we can put the instruction
				// index here.
				//

				if (ST_found_value <= CG_generating_line)
				{
					CG_instruction[CG_instruction_upto++] = CG_line[ST_found_value].code;
				}
				else
				{
					//
					// The line number to be converted to instruction after code generation.
					//

					CG_instruction[CG_instruction_upto++] = ST_found_value;

					//
					// Remember all the line numbers we must convert.
					//

					ASSERT(WITHIN(CG_forward_goto_upto, 0, CG_MAX_FORWARD_GOTOS - 1));

					CG_forward_goto[CG_forward_goto_upto].instruction = CG_instruction_upto - 1;

					switch(pn->type)
					{
						case PARSE_NODE_TYPE_GOTO:  CG_forward_goto[CG_forward_goto_upto].type = CG_FORWARD_GOTO_TYPE_GOTO;  break;
						case PARSE_NODE_TYPE_GOSUB: CG_forward_goto[CG_forward_goto_upto].type = CG_FORWARD_GOTO_TYPE_GOSUB; break;

						default:
							ASSERT(0);
							break;
					}

					CG_forward_goto_upto++;
				}

				//
				// Remember where the jump instruction is.
				//

				ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

				CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;
			}

			break;

		case PARSE_NODE_TYPE_LABEL:
			break;

		case PARSE_NODE_TYPE_GT:
			CG_instruction[CG_instruction_upto++] = ML_DO_GT;
			break;

		case PARSE_NODE_TYPE_LT:
			CG_instruction[CG_instruction_upto++] = ML_DO_LT;
			break;

		case PARSE_NODE_TYPE_GTEQ:
			CG_instruction[CG_instruction_upto++] = ML_DO_GTEQ;
			break;

		case PARSE_NODE_TYPE_LTEQ:
			CG_instruction[CG_instruction_upto++] = ML_DO_LTEQ;
			break;

		case PARSE_NODE_TYPE_AND:
			CG_instruction[CG_instruction_upto++] = ML_DO_AND;
			break;

		case PARSE_NODE_TYPE_OR:
			CG_instruction[CG_instruction_upto++] = ML_DO_OR;
			break;

		case PARSE_NODE_TYPE_NOT:
			CG_instruction[CG_instruction_upto++] = ML_DO_NOT;
			break;

		case PARSE_NODE_TYPE_DOT:
			break;

		case PARSE_NODE_TYPE_LOCAL:

			//
			// LOCAL statement outside of functions have already been
			// handled (they've added their variable with "<-" at the start
			// to the symbol table).
			//

			if (CG_in_function)
			{
				//
				// There shouldn't already be a local with this name.
				//

				ASSERT(!(ST_find(pn->variable) && ST_found_table == ST_TABLE_LOCAL));

				//
				// Add the local to the symbol table.
				//

				ST_add(ST_TABLE_LOCAL, pn->variable, CG_func[CG_in_function].arg_upto, 0);

				CG_func[CG_in_function].arg_upto += 1;
			}

			break;

		case PARSE_NODE_TYPE_PRINT:
			CG_instruction[CG_instruction_upto++] = ML_DO_PRINT;
			break;

		case PARSE_NODE_TYPE_ASSIGN:
			CG_instruction[CG_instruction_upto++] = ML_DO_ASSIGN;
			break;

		case PARSE_NODE_TYPE_VAR_ADDRESS:

			if (!ST_find(pn->variable))
			{
				ASSERT(0);
			}
			else
			{
				CG_instruction[CG_instruction_upto++] = (ST_found_table == ST_TABLE_LOCAL) ? ML_DO_PUSH_LOCAL_ADDRESS : ML_DO_PUSH_GLOBAL_ADDRESS;
				CG_instruction[CG_instruction_upto++] = ST_found_value;

				if (ST_found_table != ST_TABLE_LOCAL)
				{
					//
					// Remember the reference to the global.
					//

					ASSERT(WITHIN(CG_globalref_upto, 0, CG_MAX_GLOBALREFS - 1));

					CG_globalref[CG_globalref_upto++].instruction = CG_instruction_upto - 1;
				}
			}

			break;

		case PARSE_NODE_TYPE_BOOLEAN:

			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_BOOLEAN;
			CG_instruction[CG_instruction_upto++] = pn->boolean;

			break;

		case PARSE_NODE_TYPE_SQRT:
			CG_instruction[CG_instruction_upto++] = ML_DO_SQRT;
			break;

		case PARSE_NODE_TYPE_NEWLINE:
			CG_instruction[CG_instruction_upto++] = ML_DO_NEWLINE;
			break;

		case PARSE_NODE_TYPE_ABS:
			CG_instruction[CG_instruction_upto++] = ML_DO_ABS;
			break;

		case PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_FIELD_ADDRESS;
			break;

		case PARSE_NODE_TYPE_FIELD:

			if (!ST_find(pn->field))
			{
				ASSERT(0);
			}
			else
			{
				CG_instruction[CG_instruction_upto++] = ST_found_value;

				//
				// Remember the reference to the field.
				//

				ASSERT(WITHIN(CG_fieldref_upto, 0, CG_MAX_FIELDREFS - 1));

				CG_fieldref[CG_fieldref_upto++].instruction = CG_instruction_upto - 1;
			}

			break;

		case PARSE_NODE_TYPE_PUSH_FIELD_VALUE:

			if (pn->flag & PARSE_NODE_FLAG_EXTRACT)
			{
				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_FIELD_QUICK;
			}
			else
			{
				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_FIELD_VALUE;
			}

			break;

		case PARSE_NODE_TYPE_EXP_LIST:
			break;

		case PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_ARRAY_ADDRESS;
			CG_instruction[CG_instruction_upto++] = pn->dimensions;
			break;

		case PARSE_NODE_TYPE_PUSH_ARRAY_VALUE:

			if (pn->flag & PARSE_NODE_FLAG_EXTRACT)
			{
				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_ARRAY_QUICK;
			}
			else
			{
				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_ARRAY_VALUE;
			}

			CG_instruction[CG_instruction_upto++] = pn->dimensions;

			break;

		case PARSE_NODE_TYPE_INPUT:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_INPUT;
			break;

		case PARSE_NODE_TYPE_UNDEFINED:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_UNDEFINED;
			CG_instruction[CG_instruction_upto++] = 0;
			break;

		case PARSE_NODE_TYPE_STATEMENT_LIST:
			break;

		case PARSE_NODE_TYPE_EXIT:
			CG_instruction[CG_instruction_upto++] = ML_DO_EXIT;
			break;

		case PARSE_NODE_TYPE_RETURN:

			if (CG_in_function)
			{
				ASSERT(WITHIN(CG_in_function, 0, CG_func_upto - 1));

				if (pn->child1 == NULL)
				{
					//
					// The function doesn't return anything. Make it
					// return UNDEFINED.
					//

					CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
					CG_instruction[CG_instruction_upto++] = ML_TYPE_UNDEFINED;
					CG_instruction[CG_instruction_upto++] = 0;
				}

				//
				// A return from function rather than a simple GOSUB.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_ENDFUNC;
				CG_instruction[CG_instruction_upto++] = CG_func[CG_in_function].num_args;
			}
			else
			{
				CG_instruction[CG_instruction_upto++] = ML_DO_RETURN;

				if (pn->child1)
				{
					//
					// ERROR! Only RETURNs inside functions can return values.
					//

					ASSERT(0);
				}
			}

			break;

		case PARSE_NODE_TYPE_XOR:
			CG_instruction[CG_instruction_upto++] = ML_DO_XOR;
			break;

		case PARSE_NODE_TYPE_FOR:

			//
			// This is a forward goto so we skip over the STEP code.
			//

			CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;
			CG_instruction[CG_instruction_upto++] = 0;	// To be filled in later by the FAKE_ENDFOR parse node.

			//
			// Remember where the jump instruction is.
			//

			ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

			CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

			//
			// This is where we GOTO to for another iteration.
			//

			ASSERT(WITHIN(CG_for_upto, 0, CG_MAX_FORS - 1));

			CG_for[CG_for_upto].type     = CG_FOR_TYPE_ANON;
			CG_for[CG_for_upto].lvalue   = NULL;
			CG_for[CG_for_upto].loopto   = CG_instruction_upto;
			CG_for[CG_for_upto].overstep = CG_instruction_upto - 1;
			CG_for[CG_for_upto].forcode  = pn->forcode;

			if (pn->lvalue)
			{
				CG_for[CG_for_upto].type   = CG_FOR_TYPE_LVALUE;
				CG_for[CG_for_upto].lvalue = pn->lvalue;
			}

			CG_for_upto += 1;

			break;

		case PARSE_NODE_TYPE_FAKE_END_FOR:

			//
			// Find the FOR statement corresponding to this ENDFOR node.
			//

			{
				SLONG i;
				
				for (i = CG_for_upto - 1; i >= 0; i--)
				{
					if (CG_for[i].forcode == pn->forcode)
					{
						ASSERT(WITHIN(CG_for[i].overstep, 0, CG_instruction_upto - 1));

						CG_instruction[CG_for[i].overstep] = CG_instruction_upto;

						goto found_corresponding_for;
					}
				}

				ASSERT(0);

			  found_corresponding_for:;
			}

			break;

		case PARSE_NODE_TYPE_NEXT:

			{
				SLONG i;

				for (i = CG_for_upto - 1; i >= 0; i--)
				{
					if (CG_for[i].forcode == 0)
					{
						//
						// This FOR has alreay been matched by a next...
						//

						continue;
					}

					if (pn->lvalue == NULL)
					{
						//
						// Match this FOR loop...
						//
					}
					else
					{
						//
						// Only match a FOR loop with the correct variable.
						//

						if (CG_for[i].type == CG_FOR_TYPE_LVALUE && PARSE_trees_the_same(CG_for[i].lvalue, pn->lvalue))
						{
							//
							// This is our FOR loop!
							//
						}
						else
						{
							//
							// This is not our FOR loop.
							//

							continue;
						}
					}

					//
					// This is the FOR-LOOP to match up with- free it up.
					//

					CG_for[i].forcode = 0;

					//
					// Loop back to the increment and condition.
					//

					CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;
					CG_instruction[CG_instruction_upto++] = CG_for[i].loopto;

					//
					// Remember where the jump instruction is.
					//

					ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

					CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

					//
					// Fill in the jump for the condition.
					//

					ASSERT(WITHIN(CG_for[i].afternext, 0, CG_instruction_upto - 1));

					CG_instruction[CG_for[i].afternext] = CG_instruction_upto;

					//
					// Get rid of the lvalue node so the tree traversal doesn't 
					// generate code for it.
					//

					pn->lvalue = NULL;

					goto found_matching_for;
				}

				//
				// ERROR! No matching FOR loop for the NEXT statement.
				//

				ASSERT(0);

			  found_matching_for:;
			}

			break;

		case PARSE_NODE_TYPE_FAKE_FOR_COND:

			//
			// Find the FOR statement corresponding to this FOR_COND node.
			//

			{
				SLONG i;
				
				for (i = CG_for_upto - 1; i >= 0; i--)
				{
					if (CG_for[i].forcode == pn->forcode)
					{
						//
						// Generate code to jump to after the NEXT in the
						// condition is TRUE.
						//

						CG_instruction[CG_instruction_upto++] = ML_DO_IF_TRUE_GOTO;
						CG_instruction[CG_instruction_upto++] = 0;

						CG_for[i].afternext = CG_instruction_upto - 1;

						//
						// Remember the jump instruction.
						//

						ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

						CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

						goto found_the_for;
					}
				}

				//
				// ERROR! No matching FOR loop for the NEXT statement.
				//

				ASSERT(0);

			  found_the_for:;
			}

			break;

		case PARSE_NODE_TYPE_NOTEQUAL:
			CG_instruction[CG_instruction_upto++] = ML_DO_NOTEQUAL;
			break;

		case PARSE_NODE_TYPE_RANDOM:
			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_RANDOM_SLUMBER;
			break;

		case PARSE_NODE_TYPE_SWAP:
			CG_instruction[CG_instruction_upto++] = ML_DO_SWAP;
			break;

		case PARSE_NODE_TYPE_MIF:

			//
			// Create code to jump over the THEN code if the condition
			// code evaluates to FALSE.
			//

			CG_instruction[CG_instruction_upto++] = ML_DO_IF_FALSE_GOTO;
			CG_instruction[CG_instruction_upto++] = 0;	// To be filled in later when we get the MELSE or MENDIF node.

			//
			// Remember the jump instruction.
			//

			ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

			CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

			//
			// Create a new entry in the multi-line IF array.
			//

			ASSERT(WITHIN(CG_mif_upto, 0, CG_MAX_MIFS - 1));

			CG_mif[CG_mif_upto].flag          = 0;
			CG_mif[CG_mif_upto].iffalsejump   = CG_instruction_upto - 1;
			CG_mif[CG_mif_upto].afterthenjump = 0;

			CG_mif_upto += 1;

			break;

		case PARSE_NODE_TYPE_MELSE:

			//
			// An ELSE instruction in a multi-line IF. Match it up with
			// the nearest MIF.
			//

			{
				SLONG i;

				for (i = CG_mif_upto - 1; i >= 0; i--)
				{
					if (CG_mif[i].flag & CG_MIF_FLAG_FOUND_MENDIF)
					{
						//
						// This MIF has already been matched with an ENDIF.
						//
					}
					else
					{
						//
						// This is our MIF instruction.
						//

						if (CG_mif[i].flag & CG_MIF_FLAG_FOUND_MELSE)
						{
							//
							// ERROR! Badly formed, nested multi-line ifs...
							//

							ASSERT(0);
						}

						//
						// Add instructions to jump over the ELSE code to
						// the end of the THEN code.
						//

						CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;
						CG_instruction[CG_instruction_upto++] = 0;	// To be filled in later when we get the MENDIF node.

						//
						// Remember where the jump instruction is.
						//

						ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

						CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

						//
						// Update the multiline-if structure now we have found the else.
						//

						CG_mif[i].flag         |= CG_MIF_FLAG_FOUND_MELSE;
						CG_mif[i].afterthenjump = CG_instruction_upto - 1;

						//
						// Set the target of the IF_FALSE_GOTO jump instruction
						// inserted by the MIF node.
						//

						ASSERT(WITHIN(CG_mif[i].iffalsejump, 0, CG_instruction_upto - 2));

						CG_instruction[CG_mif[i].iffalsejump] = CG_instruction_upto;

						goto found_mif_for_melse;
					}
				}

				//
				// ERROR!
				//

				ASSERT(0);

			  found_mif_for_melse:;
			}

			break;

		case PARSE_NODE_TYPE_MENDIF:

			//
			// An ENDIF instruction in a multi-line IF. Match it up with
			// the nearest MIF.
			//

			{
				SLONG i;

				for (i = CG_mif_upto - 1; i >= 0; i--)
				{
					if (CG_mif[i].flag & CG_MIF_FLAG_FOUND_MENDIF)
					{
						//
						// This MIF has already been matched with an ENDIF.
						//
					}
					else
					{
						CG_mif[i].flag |= CG_MIF_FLAG_FOUND_MENDIF;

						if (CG_mif[i].flag & CG_MIF_FLAG_FOUND_MELSE)
						{
							//
							// Set the target of the jump put in by the MELSE node.
							//

							ASSERT(WITHIN(CG_mif[i].afterthenjump, 0, CG_instruction_upto - 2));

							CG_instruction[CG_mif[i].afterthenjump] = CG_instruction_upto;
						}
						else
						{
							//
							// Set the target of the jump put in by the MIF node.
							//

							ASSERT(WITHIN(CG_mif[i].iffalsejump, 0, CG_instruction_upto - 2));

							CG_instruction[CG_mif[i].iffalsejump] = CG_instruction_upto;
						}

						goto found_mif_for_endif;
					}
				}
				
				//
				// ERROR!
				//

				ASSERT(0);

			  found_mif_for_endif:;
			}

			break;

		case PARSE_NODE_TYPE_WHILE:

			//
			// Create a new entry in the while array.
			//

			ASSERT(WITHIN(CG_while_upto, 0, CG_MAX_WHILES - 1));

			CG_while[CG_while_upto].flag        = 0;
			CG_while[CG_while_upto].iffalsejump = 0;
			CG_while[CG_while_upto].whilecode   = pn->whilecode;
			CG_while[CG_while_upto].loopto      = CG_instruction_upto;

			CG_while_upto += 1;

			break;

		case PARSE_NODE_TYPE_FAKE_WHILE_COND:

			//
			// Create code to jump over the body of the While-loop if the condition
			// code evaluates to FALSE.
			//

			CG_instruction[CG_instruction_upto++] = ML_DO_IF_FALSE_GOTO;
			CG_instruction[CG_instruction_upto++] = 0;	// To be filled in later when we LOOP node.

			//
			// Remember the jump instruction.
			//

			ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

			CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

			//
			// Find our while loop.
			//
			
			{
				SLONG i;

				for (i = CG_while_upto - 1; i >= 0; i--)
				{
					if (CG_while[i].whilecode == pn->whilecode)
					{
						//
						// Found our while loop!
						//

						CG_while[i].iffalsejump = CG_instruction_upto - 1;

						goto found_our_while_loop;
					}
				}

				//
				// Oh dear...
				//

				ASSERT(0);

			  found_our_while_loop:;
			}

			break;

		case PARSE_NODE_TYPE_LOOP:
			
			//
			// Look for the nearest unmatched while loop.
			//

			{
				SLONG i;

				for (i = CG_while_upto - 1; i >= 0; i--)
				{
					if (CG_while[i].flag & CG_WHILE_FLAG_FOUND_LOOP)
					{
						//
						// Already been matched up.
						//
					}
					else
					{
						ASSERT(WITHIN(CG_while[i].loopto     , 0, CG_instruction_upto - 1));
						ASSERT(WITHIN(CG_while[i].iffalsejump, 0, CG_instruction_upto - 1));

						CG_while[i].flag |= CG_WHILE_FLAG_FOUND_LOOP;

						//
						// Loop back to the beginning of the while loop.
						//

						CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;
						CG_instruction[CG_instruction_upto++] = CG_while[i].loopto;

						//
						// Remember where the jump instruction is.
						//

						ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

						CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

						//
						// Set the target of the jump put in by the WHILE node.
						//

						CG_instruction[CG_while[i].iffalsejump] = CG_instruction_upto;

						goto found_while;
					}
				}

				//
				// ERROR!
				//

				ASSERT(0);

			  found_while:;
			}

			break;

		case PARSE_NODE_TYPE_FUNCTION:

			if (CG_in_function)
			{
				//
				// ERROR!
				//

				ASSERT(0);
			}
			else
			{
				//
				// Find our function.
				//

				if (!ST_find(pn->variable))
				{
					//
					// ERROR!
					// 

					ASSERT(0);
				}
				else
				{
					CG_in_function = ST_found_value;

					ASSERT(WITHIN(CG_in_function, 0, CG_func_upto - 1));
					ASSERT(strcmp(CG_func[CG_in_function].name, pn->variable) == 0);

					//
					// Code to jump over the body of the function if execution
					// leaks down to the function.
					//

					CG_instruction[CG_instruction_upto++] = ML_DO_GOTO;
					CG_instruction[CG_instruction_upto++] = CG_func[CG_in_function].endline + 1;

					//
					// Remember where the jump instruction is.
					//

					ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

					CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;

					//
					// This is a forward goto..
					//

					ASSERT(WITHIN(CG_forward_goto_upto, 0, CG_MAX_FORWARD_GOTOS - 1));

					CG_forward_goto[CG_forward_goto_upto].type        = CG_FORWARD_GOTO_TYPE_GOTO;
					CG_forward_goto[CG_forward_goto_upto].instruction = CG_instruction_upto - 1;

					CG_forward_goto_upto++;

					//
					// The numbering of our arugments and local variables
					//

					CG_func[CG_in_function].arg_upto = 0;

					//
					// Enter the function. Easy!
					//

					CG_instruction[CG_instruction_upto++] = ML_DO_ENTERFUNC;
					CG_instruction[CG_instruction_upto++] = CG_func[CG_in_function].num_args;
				}
			}

			break;

		case PARSE_NODE_TYPE_ARGUMENT:
			
			//
			// We must be inside a function!
			//

			if (!CG_in_function)
			{
				//
				// ERROR!
				//

				ASSERT(0);
			}

			//
			// There shouldn't already be a local with this name.
			//

			ASSERT(!(ST_find(pn->variable) && ST_found_table == ST_TABLE_LOCAL));

			//
			// Add this argument to the local variables for this function.
			//

			ST_add(ST_TABLE_LOCAL, pn->variable, CG_func[CG_in_function].arg_upto, 0);

			CG_func[CG_in_function].arg_upto += 1;

			break;

		case PARSE_NODE_TYPE_ENDFUNC:

			//
			// The function doesn't return anything. Make it
			// return UNDEFINED.
			//

			ASSERT(WITHIN(CG_in_function, 0, CG_func_upto - 1));

			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_UNDEFINED;
			CG_instruction[CG_instruction_upto++] = 0;

			CG_instruction[CG_instruction_upto++] = ML_DO_ENDFUNC;
			CG_instruction[CG_instruction_upto++] = CG_func[CG_in_function].num_args;

			//
			// Not in a function any more.
			//

			CG_in_function = FALSE;

			//
			// We've lost all the symbols in the local symbol table.
			//

			ST_clear(ST_TABLE_LOCAL);

			break;

		case PARSE_NODE_TYPE_CALL:

			//
			// Push the number of arguments in the function call onto the stack.
			// The function will check for the correct number and insert
			// extra UNDEFINED locals as required.
			//

			CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
			CG_instruction[CG_instruction_upto++] = ML_TYPE_NUM_ARGS;
			CG_instruction[CG_instruction_upto++] = pn->args;

			if (!ST_find(pn->variable))
			{
				//
				// This is an undeclared function.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_GOSUB;
				CG_instruction[CG_instruction_upto++] = 0;
				
				//
				// Remember where the reference to the function is.
				//

				ASSERT(WITHIN(CG_undefref_upto, 0, CG_MAX_UNDEFREFS - 1));

				CG_undefref[CG_undefref_upto].name        = CG_add_string_to_debug_data(pn->variable);
				CG_undefref[CG_undefref_upto].instruction = CG_instruction_upto - 1;

				CG_undefref_upto += 1;
			}
			else
			{
				ASSERT(WITHIN(ST_found_value, 0, CG_func_upto - 1));
				ASSERT(strcmp(CG_func[ST_found_value].name, pn->variable) == 0);

				if (CG_generating_line > CG_func[ST_found_value].line)
				{
					//
					// This is a backwards goto so we can fill in the instruction
					// right away.  We add 2 to the instruction to jump over the GOTO
					// over the function...
					//

					CG_instruction[CG_instruction_upto++] = ML_DO_GOSUB;
					CG_instruction[CG_instruction_upto++] = CG_line[CG_func[ST_found_value].line].code + 2;
				}
				else
				{
					//
					// This is a forward goto.
					// 

					CG_instruction[CG_instruction_upto++] = ML_DO_GOSUB;
					CG_instruction[CG_instruction_upto++] = CG_func[ST_found_value].line;

					ASSERT(WITHIN(CG_forward_goto_upto, 0, CG_MAX_FORWARD_GOTOS - 1));

					CG_forward_goto[CG_forward_goto_upto].type        = CG_FORWARD_GOTO_TYPE_CALL;
					CG_forward_goto[CG_forward_goto_upto].instruction = CG_instruction_upto - 1;

					CG_forward_goto_upto++;
				}

				//
				// Remember where the jump instruction is.
				//

				ASSERT(WITHIN(CG_jump_upto, 0, CG_MAX_JUMPS - 1));

				CG_jump[CG_jump_upto++].instruction = CG_instruction_upto - 1;
			}

			if (!(pn->flag & PARSE_NODE_FLAG_EXPRESSION))
			{	
				//
				// If this function call isn't part of an expression, the
				// return value isn't needed.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_POP;
			}

			break;

		case PARSE_NODE_TYPE_TEXTURE:
		case PARSE_NODE_TYPE_BUFFER:
		case PARSE_NODE_TYPE_DRAW:
		case PARSE_NODE_TYPE_CLS:

			{
				SLONG num_args;
				SLONG func_code;

				//
				// How many arguments do we want and what is the function code?
				//

				switch(pn->type)
				{
					case PARSE_NODE_TYPE_TEXTURE: num_args = 2; func_code = ML_DO_TEXTURE; break;
					case PARSE_NODE_TYPE_BUFFER:  num_args = 4; func_code = ML_DO_BUFFER;  break;
					case PARSE_NODE_TYPE_DRAW:	  num_args = 3; func_code = ML_DO_DRAW;    break;
					case PARSE_NODE_TYPE_CLS:     num_args = 2; func_code = ML_DO_CLS;     break;

					default:
						ASSERT(0);
						break;
				}

				//
				// Do we have the right number of arguments for the function?
				//

				if (num_args == pn->args)
				{
					//
					// The right number of arguements!
					//
				}
				else
				if (num_args > pn->args)
				{
					//
					// Need to push some undefined members onto the stack.
					//

					SLONG i;

					for (i = pn->args; i < num_args; i++)
					{
						CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
						CG_instruction[CG_instruction_upto++] = ML_TYPE_UNDEFINED;
						CG_instruction[CG_instruction_upto++] = 0;
					}
				}
				else
				{
					//
					// ERROR! More arguments than the function needs?!
					//

					ASSERT(0);
				}

				//
				// The function code.
				//

				CG_instruction[CG_instruction_upto++] = func_code;
			}

			break;

		case PARSE_NODE_TYPE_FLIP:
			CG_instruction[CG_instruction_upto++] = ML_DO_FLIP;
			break;

		case PARSE_NODE_TYPE_KEY_VALUE:
			CG_instruction[CG_instruction_upto++] = ML_DO_KEY_VALUE;
			break;

		case PARSE_NODE_TYPE_KEY_ASSIGN:
			CG_instruction[CG_instruction_upto++] = ML_DO_KEY_ASSIGN;
			break;

		case PARSE_NODE_TYPE_INKEY_VALUE:
			CG_instruction[CG_instruction_upto++] = ML_DO_INKEY_VALUE;
			break;

		case PARSE_NODE_TYPE_INKEY_ASSIGN:
			CG_instruction[CG_instruction_upto++] = ML_DO_INKEY_ASSIGN;
			break;

		case PARSE_NODE_TYPE_TIMER:
			CG_instruction[CG_instruction_upto++] = ML_DO_TIMER;
			break;

		case PARSE_NODE_TYPE_SIN:
			CG_instruction[CG_instruction_upto++] = ML_DO_SIN;
			break;

		case PARSE_NODE_TYPE_COS:
			CG_instruction[CG_instruction_upto++] = ML_DO_COS;
			break;

		case PARSE_NODE_TYPE_TAN:
			CG_instruction[CG_instruction_upto++] = ML_DO_TAN;
			break;

		case PARSE_NODE_TYPE_ASIN:
			CG_instruction[CG_instruction_upto++] = ML_DO_ASIN;
			break;

		case PARSE_NODE_TYPE_ACOS:
			CG_instruction[CG_instruction_upto++] = ML_DO_ACOS;
			break;

		case PARSE_NODE_TYPE_ATAN:
			CG_instruction[CG_instruction_upto++] = ML_DO_ATAN;
			break;

		case PARSE_NODE_TYPE_ATAN2:
			CG_instruction[CG_instruction_upto++] = ML_DO_ATAN2;
			break;

		case PARSE_NODE_TYPE_EXPORT:
			break;

		case PARSE_NODE_TYPE_LEFT:

			if (pn->child2 == NULL)
			{
				//
				// This is a one-argument version. Push the constant 1 onto the stack.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
				CG_instruction[CG_instruction_upto++] = ML_TYPE_SLUMBER;
				CG_instruction[CG_instruction_upto++] = 1;
			}

			CG_instruction[CG_instruction_upto++] = ML_DO_LEFT;

			break;

		case PARSE_NODE_TYPE_RIGHT:

			if (pn->child2 == NULL)
			{
				//
				// This is a one-argument version. Push the constant 1 onto the stack.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
				CG_instruction[CG_instruction_upto++] = ML_TYPE_SLUMBER;
				CG_instruction[CG_instruction_upto++] = 1;
			}

			CG_instruction[CG_instruction_upto++] = ML_DO_RIGHT;

			break;

		case PARSE_NODE_TYPE_MID:

			if (pn->child3 == NULL)
			{
				//
				// This is a two-argument version. Push the constant 1 onto the stack.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_CONSTANT;
				CG_instruction[CG_instruction_upto++] = ML_TYPE_SLUMBER;
				CG_instruction[CG_instruction_upto++] = 1;
			}

			CG_instruction[CG_instruction_upto++] = ML_DO_MID;

			break;

		case PARSE_NODE_TYPE_LEN:
			CG_instruction[CG_instruction_upto++] = ML_DO_LEN;
			break;

		case PARSE_NODE_TYPE_MATRIX:

			if (pn->child1)
			{
				//
				// Constructing a matrix from three vectors.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_MATRIX;
			}
			else
			{
				//
				// The identity matrix.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_IDENTITY_MATRIX;
			}

			break;

		case PARSE_NODE_TYPE_VECTOR:

			if (pn->child1)
			{
				//
				// Constructing a vector from three numbers.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_VECTOR;
			}
			else
			{
				//
				// The zero vector.
				//

				CG_instruction[CG_instruction_upto++] = ML_DO_PUSH_ZERO_VECTOR;
			}

			break;

		default:
			ASSERT(0);
			break;
	}

	return TRUE;
}



SLONG CG_do(CBYTE *fname, SLONG output)
{
	SLONG i;

	PARSE_Node   *pn;
	FILE         *handle;
	ML_Header     mh;
	LINK_Header   lh;
	CG_Func      *cf;
	LINK_Global  *lg;
	LINK_Function lf;
	LINK_Field   *ld;

	CBYTE export_string [LEX_MAX_STRING_LENGTH + 2];
	CBYTE local_string  [LEX_MAX_STRING_LENGTH + 2];
	CBYTE string_backup [LEX_MAX_STRING_LENGTH + 2];
	SLONG value_backup;
	SLONG export;
	SLONG local;

	#ifdef _DEBUG
	SLONG globals_found;
	SLONG fields_found;
	#endif

	//
	// Clear symbol tables and add the system variables.
	//

	ST_clear_all();
	SYSVAR_init();

	//
	// Initialise output.
	//

	CG_output       = NULL;
	CG_num_errors   = 0;
	CG_num_warnings = 0;

	memset(CG_instruction,  0, sizeof(CG_instruction ));
	memset(CG_line,         0, sizeof(CG_line        ));
	memset(CG_func,         0, sizeof(CG_func        ));
	memset(CG_for,          0, sizeof(CG_for         ));
	memset(CG_forward_goto, 0, sizeof(CG_forward_goto));
	memset(CG_if,           0, sizeof(CG_if          ));
	memset(CG_mif,          0, sizeof(CG_mif         ));
	memset(CG_while,        0, sizeof(CG_while       ));

	memset(CG_jump,         0, sizeof(CG_jump        ));
	memset(CG_globalref,    0, sizeof(CG_globalref   ));
	memset(CG_fieldref,     0, sizeof(CG_fieldref	 ));
	memset(CG_undefref,     0, sizeof(CG_undefref	 ));
	memset(CG_datatableref, 0, sizeof(CG_datatableref));
	memset(CG_debug_data,   0, sizeof(CG_debug_data  ));

	CG_instruction_upto  = 0;
	CG_line_upto         = 0;
	CG_field_id_upto     = SYSVAR_FIELD_NUMBER;
	CG_for_upto          = 0;
	CG_mif_upto          = 0;
	CG_while_upto        = 0;
	CG_func_upto         = 1;	// 0 func => top level code.
	CG_forward_goto_upto = 0;

	CG_generating_line   = 0;
	CG_global_upto       = 0;

	CG_jump_upto         = 0;
	CG_globalref_upto    = 0;
	CG_fieldref_upto     = 0;
	CG_undefref_upto     = 0;
	CG_datatableref_upto = 0;
	CG_debug_data_upto   = 0;

	//
	// The string table.
	//

	CG_data_table_max  = 512;
	CG_data_table      = (UBYTE *) malloc(sizeof(UBYTE) * CG_data_table_max);
	CG_data_table_upto = 0;

	//
	// Go through the PARSE tree adding all labels and variables
	// to the symbol table and finding our functions.
	//

	CG_in_function = FALSE;	// Not in a function when we start...

	for (i = 0; i < PARSE_line_upto; i++)
	{
		pn = PARSE_line[i];

		CG_generating_line = i;

		PARSE_traverse(pn, CG_callback_add_labels_and_variables);
	}

	//
	// Generate code!
	//

	CG_in_function = FALSE;	// Not in a function when we start...

	for (i = 0; i < PARSE_line_upto; i++)
	{
		pn = PARSE_line[i];

		//
		// Each line knows where it's instructions are.
		//

		ASSERT(WITHIN(i, 0, CG_MAX_LINES - 1));

		CG_line[i].code = CG_instruction_upto;
		CG_line_upto    = i + 1;

		//
		// Clear the Ifs-per-line array.
		//

		CG_if_upto = 0;

		//
		// Generate code for this line.
		//

		CG_generating_line = i;

		PARSE_traverse(pn, CG_callback_generate_code);
	}

	//
	// The pretend last line that holds the EXIT instruction.
	//

	ASSERT(WITHIN(CG_line_upto, 0, CG_MAX_LINES - 1));

	CG_line[CG_line_upto++].code = CG_instruction_upto;

	//
	// And write out an EXIT instruction.
	//

	if (!(WITHIN(CG_instruction_upto, 0, CG_MAX_INSTRUCTIONS - 1)))
	{
		//
		// This is a bit unlucky! ERROR!
		//

		ASSERT(0);

		return FALSE;
	}

	CG_instruction[CG_instruction_upto++] = ML_DO_EXIT;
	
	//
	// Replace all GOTO addresses with the instruction index rather
	// than the line number.
	//

	for (i = 0; i < CG_forward_goto_upto; i++)
	{
		ASSERT(WITHIN(CG_forward_goto[i].instruction, 0, CG_instruction_upto - 1));
		ASSERT(WITHIN(CG_instruction[CG_forward_goto[i].instruction], 0, CG_line_upto - 1));

		CG_instruction[CG_forward_goto[i].instruction] = CG_line[CG_instruction[CG_forward_goto[i].instruction]].code;

		if (CG_forward_goto[i].type == CG_FORWARD_GOTO_TYPE_CALL)
		{
			//
			// The first couple of instructions of a function are
			// a GOTO over the body of the function- incase the
			// functions are defined at the top of the file. So
			// when we call the function, we must jump to a couple
			// of instructions ahead.
			//

			CG_instruction[CG_forward_goto[i].instruction] += 2;
		}
	}
	
	//
	// Open our output filename.
	//

	handle = fopen(fname, "wb");

	if (!handle)
	{
		//
		// ERROR!
		//

		ASSERT(0);

		return FALSE;
	}

	//
	// Write out the file type.
	//

	if (output & CG_OUTPUT_EXECUTABLE)
	{
		if (CG_undefref_upto != 0)
		{
			//
			// ERROR! You can't have undefined functions when you
			// output an executable.
			//

			ASSERT(0);
		}

		//
		// The header of an executable.
		//

		mh.version                      = ML_VERSION_NUMBER;
		mh.instructions_memory_in_bytes = CG_instruction_upto * sizeof(SLONG);
		mh.data_table_length_in_bytes   = CG_data_table_upto;
		mh.num_globals                  = CG_global_upto;

		if (fwrite(&mh, sizeof(mh), 1, handle) != 1) goto file_error;

		//
		// The instructions.
		//

		if (fwrite(CG_instruction, sizeof(SLONG), CG_instruction_upto, handle) != CG_instruction_upto) goto file_error;

		//
		// The string table.
		//

		if (fwrite(CG_data_table, sizeof(UBYTE), CG_data_table_upto, handle) != CG_data_table_upto) goto file_error;

		fclose(handle);

		//
		// Find all our symbols.
		//

		ST_find_all_start();

		while(ST_find_all_next())
		{
			OS_string("Symbol \"%s\" = %d\n", ST_found_string, ST_found_value);
		}

		return TRUE;
	}
	else
	{
		//
		// Writing out the magic number after each block.
		//

		SLONG magic = 12345678;

		#define CG_WRITE_MAGIC() {if (fwrite(&magic, sizeof(SLONG), 1, handle) != 1) goto file_error;}

		//
		// The header of the object file.
		//

		lh.version                     = 1;
		lh.num_instructions            = CG_instruction_upto;
		lh.data_table_length_in_bytes  = CG_data_table_upto;
		lh.num_globals                 = CG_global_upto;
		lh.num_functions               = CG_func_upto - 1;	// Function 0 is not used
		lh.num_lines                   = CG_line_upto;
		lh.num_jumps                   = CG_jump_upto;
		lh.num_fields                  = CG_field_id_upto;
		lh.num_global_refs             = CG_globalref_upto;
		lh.num_undef_refs              = CG_undefref_upto;
		lh.num_field_refs              = CG_fieldref_upto;
		lh.num_data_table_refs         = CG_datatableref_upto;

		if (fwrite(&lh, sizeof(lh), 1, handle) != 1) goto file_error;

		//
		// Each block of data.
		//

		if (fwrite(CG_instruction, sizeof(SLONG), CG_instruction_upto, handle) != CG_instruction_upto) goto file_error; CG_WRITE_MAGIC();
		if (fwrite(CG_data_table,  sizeof(UBYTE), CG_data_table_upto,  handle) != CG_data_table_upto ) goto file_error;	CG_WRITE_MAGIC();

		//
		// Write out the globals sorted by their global_id.
		//
		
		{
			lg = (LINK_Global *) malloc(sizeof(LINK_Global) * CG_global_upto);
			

			#ifdef _DEBUG
			globals_found = 0;
			#endif

			ST_find_all_start();

			while(ST_find_all_next())
			{
				if (ST_found_string[0] == '.')
				{
					//
					// This is a field.
					//
				}
				else
				if (ST_found_string[0] == '(')
				{
					//
					// This is a function.
					//
				}
				else
				if (ST_found_string[0] == '-')
				{
					//
					// This is an export directive.
					//
				}
				else
				if (ST_found_string[0] == '<')
				{
					//
					// This is a local directive.
					//
				}
				else
				{
					//
					// This must be a global. Backup ST_found_string and
					// ST_found_value because we are going to access the
					// symbol table again to find out if this global
					// should be exported or not.
					//
	
					strcpy(string_backup, ST_found_string);

					value_backup = ST_found_value;

					//
					// Should we export it?
					//

					sprintf(export_string, "->%s", string_backup);

					export = ST_find(export_string);
					
					//
					// Should it be a local?
					//

					sprintf(local_string, "<-%s", string_backup);

					local = ST_find(local_string);

					//
					// Can't be both local and exported!
					//

					ASSERT(!(local && export));

					//
					// Write out the global.
					//

					ASSERT(WITHIN(value_backup, 0, CG_global_upto - 1));

					lg[value_backup].export = export;
					lg[value_backup].local  = local;
					lg[value_backup].index  = value_backup;
					lg[value_backup].name   = CG_add_string_to_debug_data(string_backup);

					#ifdef _DEBUG
					globals_found += 1;
					#endif
				}
			}

			ASSERT(globals_found == CG_global_upto);

			//
			// Write out the global array.
			//

			if (fwrite(lg, sizeof(LINK_Global), CG_global_upto, handle) != CG_global_upto) goto file_error; CG_WRITE_MAGIC();

			//
			// Free memory.
			//

			free(lg);
		}

		//
		// Write out all the defined functions.
		//

		for (i = 1; i < CG_func_upto; i++)
		{
			cf = &CG_func[i];

			//
			// Is this function exported?
			//

			sprintf(export_string, "->%s", cf->name + 2);	// + 2 skips over the () at the beginning of the function name.

			export = ST_find(export_string);

			//
			// Write out the function.
			//

			lf.name       = CG_add_string_to_debug_data(cf->name);
			lf.line_start = cf->line;
			lf.line_end   = cf->endline;
			lf.num_args   = cf->num_args;
			lf.export     = export;

			if (fwrite(&lf, sizeof(lf), 1, handle) != 1) goto file_error;
		}

		CG_WRITE_MAGIC();

		//
		// Write out each line and each jump.
		//

		if (fwrite(CG_line, sizeof(CG_Line), CG_line_upto, handle) != CG_line_upto) goto file_error; CG_WRITE_MAGIC();
		if (fwrite(CG_jump, sizeof(CG_Jump), CG_jump_upto, handle) != CG_jump_upto) goto file_error; CG_WRITE_MAGIC();

		//
		// Write out all the fields sorted by their field_id
		//

		{
			ld = (LINK_Field *) malloc(sizeof(LINK_Field) * CG_field_id_upto);

			#ifdef _DEBUG
			fields_found = 0;
			#endif

			ST_find_all_start();

			while(ST_find_all_next())
			{
				if (ST_found_string[0] == '.')
				{
					ASSERT(WITHIN(ST_found_value, 0, CG_field_id_upto - 1));

					//
					// This is a field.
					//

					ld[ST_found_value].index = ST_found_value;
					ld[ST_found_value].name  = CG_add_string_to_debug_data(ST_found_string);

					#ifdef _DEBUG
					fields_found += 1;
					#endif
				}
			}

			ASSERT(fields_found == CG_field_id_upto);

			//
			// Write out all the fields.
			//

			if (fwrite(ld, sizeof(LINK_Field), CG_field_id_upto, handle) != CG_field_id_upto) goto file_error; CG_WRITE_MAGIC();

			//
			// Free memory.
			//

			free(ld);
		}

		//
		// The references to each global, the undefined function references
		// and the field references.
		//

		ASSERT(sizeof(CG_Globalref) == sizeof(LINK_Globalref));
		ASSERT(sizeof(CG_Undefref ) == sizeof(LINK_Undefref ));
		ASSERT(sizeof(CG_Fieldref ) == sizeof(LINK_Fieldref ));

		if (fwrite(CG_globalref,    sizeof(CG_Globalref   ), CG_globalref_upto,    handle) != CG_globalref_upto   ) goto file_error; CG_WRITE_MAGIC();
		if (fwrite(CG_undefref,     sizeof(CG_Undefref    ), CG_undefref_upto,     handle) != CG_undefref_upto    ) goto file_error; CG_WRITE_MAGIC();
		if (fwrite(CG_fieldref,     sizeof(CG_Fieldref    ), CG_fieldref_upto,     handle) != CG_fieldref_upto    ) goto file_error; CG_WRITE_MAGIC();
		if (fwrite(CG_datatableref, sizeof(CG_Datatableref), CG_datatableref_upto, handle) != CG_datatableref_upto) goto file_error; CG_WRITE_MAGIC();

		//
		// The debug data.
		//

		if (fwrite(&CG_debug_data_upto, sizeof(SLONG), 1,                  handle) != 1                 ) goto file_error;
		if (fwrite( CG_debug_data,      sizeof(CBYTE), CG_debug_data_upto, handle) != CG_debug_data_upto) goto file_error;

		//
		// All done.
		//

		fclose(handle);

		return TRUE;
	}

  file_error:;

	//
	// ERROR!
	//

	ASSERT(0);

	fclose(handle);

	return FALSE;
}


