#include "always.h"
#include "lex.h"
#include "parse.h"

#include <setjmp.h>


//
// The nodes.
//

#define PARSE_MAX_NODES 65536

PARSE_Node PARSE_node[PARSE_MAX_NODES];
SLONG      PARSE_node_upto;

PARSE_Node *PARSE_get_node()
{
	ASSERT(WITHIN(PARSE_node_upto, 0, PARSE_MAX_NODES - 1));

	return &PARSE_node[PARSE_node_upto++];
}



//
// Output variables and others...
//

#define PARSE_MAX_LINES             16384
#define PARSE_MAX_STRING_TABLE_SIZE 65536
#define PARSE_MAX_ERRORS              256

PARSE_Node *PARSE_line[PARSE_MAX_LINES];		// NULL value means that line was blank.
SLONG       PARSE_line_upto;
CBYTE       PARSE_string_table[PARSE_MAX_STRING_TABLE_SIZE];
SLONG       PARSE_string_table_upto;
CBYTE      *PARSE_error[PARSE_MAX_ERRORS];
SLONG       PARSE_error_upto;
SLONG       PARSE_ifcode;			// This gets incremented every time we parse an IF    statement...
SLONG       PARSE_forcode;			// This gets incremented every time we parse a  FOR   statement...
SLONG       PARSE_whilecode;		// This gets incremented every time we parse a  WHILE statement...

//
// The error buffer.
//

#define PARSE_MAX_ERRBUF 32768

CBYTE PARSE_errbuf[PARSE_MAX_ERRBUF];
SLONG PARSE_errbuf_upto;

//
// Adds the given error. If it has run out of room, it
// returns FALSE.
//

SLONG PARSE_add_error(CBYTE *fmt, ...)
{
	if (PARSE_error_upto >= PARSE_MAX_ERRORS)
	{
		return FALSE;
	}

	//
	// Work out the real error.
	//

	CBYTE   error[512];
	va_list	ap;

	va_start(ap, fmt);
	vsprintf(error, fmt, ap);
	va_end  (ap);

	//
	// Put it into the buffer.
	//

	SLONG len = strlen(error) + 1;	// + 1 to include terminating NULL.

	if (PARSE_errbuf_upto + len > PARSE_MAX_ERRBUF)
	{
		return FALSE;
	}
	
	strcpy(PARSE_errbuf + PARSE_errbuf_upto, error);

	PARSE_error[PARSE_error_upto] = PARSE_errbuf + PARSE_errbuf_upto;

	PARSE_error_upto  += 1;
	PARSE_errbuf_upto += len;

	return TRUE;
}

//
// Where to longjmp on an error.
//

jmp_buf PARSE_error_jmp;


//
// The reason the parser jumped to PARSE_error_jmp;
//

CBYTE *PARSE_error_type;



//
// Throws up an error.
//

void PARSE_throw(CBYTE *error = "Parse error")
{
	PARSE_error_type = error;

	longjmp(PARSE_error_jmp, 1);
}





//
// Adds the string to PARSE_string_table and return the address
// where it was copied.
//

CBYTE *PARSE_add_string(CBYTE *string)
{
	SLONG length = strlen(string) + 1;	// + 1 to include the terminating NULL
	
	if (PARSE_string_table_upto + length > PARSE_MAX_STRING_TABLE_SIZE)
	{
		//
		// ERROR!
		//

		PARSE_throw("No more string constant memory");
	}

	CBYTE *ans = PARSE_string_table + PARSE_string_table_upto;

	strcpy(ans, string);

	PARSE_string_table_upto += length;

	return ans;
}


//
// Sets the PARSE_NODE_FLAG_CONDITIONAL flag in the given node.
//

SLONG PARSE_set_conditional_flag(PARSE_Node *pn)
{
	pn->flag |= PARSE_NODE_FLAG_CONDITIONAL;

	return TRUE;
}


//
// Sets the PARSE_NODE_FLAG_EXPRESSION flag in the given node.
//

SLONG PARSE_set_expression_flag(PARSE_Node *pn)
{
	pn->flag |= PARSE_NODE_FLAG_EXPRESSION;

	return TRUE;
}



//
// Returns TRUE if the given expression is sure to return
// a BOOLEAN value.
//

SLONG PARSE_expression_is_boolean(PARSE_Node *exp)
{
	switch(exp->type)
	{
		case PARSE_NODE_TYPE_EQUALS:
		case PARSE_NODE_TYPE_GT:
		case PARSE_NODE_TYPE_LT:
		case PARSE_NODE_TYPE_GTEQ:
		case PARSE_NODE_TYPE_LTEQ:
		case PARSE_NODE_TYPE_AND:
		case PARSE_NODE_TYPE_OR:
		case PARSE_NODE_TYPE_NOT:
		case PARSE_NODE_TYPE_BOOLEAN:
		case PARSE_NODE_TYPE_XOR:
		case PARSE_NODE_TYPE_KEY_VALUE:
			return TRUE;

		case PARSE_NODE_TYPE_NOP:
		case PARSE_NODE_TYPE_PLUS:
		case PARSE_NODE_TYPE_MINUS:
		case PARSE_NODE_TYPE_UMINUS:
		case PARSE_NODE_TYPE_TIMES:
		case PARSE_NODE_TYPE_DIVIDE:
		case PARSE_NODE_TYPE_SLUMBER:
		case PARSE_NODE_TYPE_FLUMBER:
		case PARSE_NODE_TYPE_STRING:
		case PARSE_NODE_TYPE_VAR_VALUE:
		case PARSE_NODE_TYPE_IF:
		case PARSE_NODE_TYPE_GOTO:
		case PARSE_NODE_TYPE_LABEL:
		case PARSE_NODE_TYPE_DOT:
		case PARSE_NODE_TYPE_CALL:
		case PARSE_NODE_TYPE_LOCAL:
		case PARSE_NODE_TYPE_PRINT:
		case PARSE_NODE_TYPE_ASSIGN:
		case PARSE_NODE_TYPE_VAR_ADDRESS:
		case PARSE_NODE_TYPE_MOD:
		case PARSE_NODE_TYPE_SQRT:
		case PARSE_NODE_TYPE_NEWLINE:
		case PARSE_NODE_TYPE_ABS:
		case PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS:
		case PARSE_NODE_TYPE_FIELD:
		case PARSE_NODE_TYPE_PUSH_FIELD_VALUE:
		case PARSE_NODE_TYPE_EXP_LIST:
		case PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS:
		case PARSE_NODE_TYPE_PUSH_ARRAY_VALUE:
		case PARSE_NODE_TYPE_INPUT:
		case PARSE_NODE_TYPE_UNDEFINED:
		case PARSE_NODE_TYPE_STATEMENT_LIST:
		case PARSE_NODE_TYPE_EXIT:
		case PARSE_NODE_TYPE_RETURN:
		case PARSE_NODE_TYPE_GOSUB:
		case PARSE_NODE_TYPE_FOR:
		case PARSE_NODE_TYPE_NEXT:
		case PARSE_NODE_TYPE_NOTEQUAL:
		case PARSE_NODE_TYPE_RANDOM:
		case PARSE_NODE_TYPE_SWAP:
		case PARSE_NODE_TYPE_MIF:
		case PARSE_NODE_TYPE_MELSE:
		case PARSE_NODE_TYPE_MENDIF:
		case PARSE_NODE_TYPE_WHILE:
		case PARSE_NODE_TYPE_LOOP:
		case PARSE_NODE_TYPE_FUNCTION:
		case PARSE_NODE_TYPE_ARGUMENT:
		case PARSE_NODE_TYPE_ENDFUNC:
		case PARSE_NODE_TYPE_TEXTURE:
		case PARSE_NODE_TYPE_BUFFER:
		case PARSE_NODE_TYPE_DRAW:
		case PARSE_NODE_TYPE_CLS:
		case PARSE_NODE_TYPE_FLIP:
		case PARSE_NODE_TYPE_KEY_ASSIGN:
		case PARSE_NODE_TYPE_INKEY_VALUE:
		case PARSE_NODE_TYPE_INKEY_ASSIGN:
		case PARSE_NODE_TYPE_TIMER:
		case PARSE_NODE_TYPE_SIN:
		case PARSE_NODE_TYPE_COS:
		case PARSE_NODE_TYPE_TAN:
		case PARSE_NODE_TYPE_ASIN:
		case PARSE_NODE_TYPE_ACOS:
		case PARSE_NODE_TYPE_ATAN:
		case PARSE_NODE_TYPE_ATAN2:
		case PARSE_NODE_TYPE_EXPORT:
		case PARSE_NODE_TYPE_LEFT:
		case PARSE_NODE_TYPE_MID:
		case PARSE_NODE_TYPE_RIGHT:
		case PARSE_NODE_TYPE_MATRIX:
		case PARSE_NODE_TYPE_VECTOR:
			return FALSE;

		default:
			ASSERT(0);
			break;
	}

	return FALSE;
}


//
// Returns a copy of the parse tree.
//

PARSE_Node *PARSE_copy_tree(PARSE_Node *tree)
{
	PARSE_Node *ans = PARSE_get_node();

   *ans = *tree;

	if (ans->child1) {ans->child1 = PARSE_copy_tree(ans->child1);}
	if (ans->child2) {ans->child2 = PARSE_copy_tree(ans->child2);}
	if (ans->child3) {ans->child3 = PARSE_copy_tree(ans->child3);}

	return ans;
}

//
// Converts an lvalue to an rvalue.
//

PARSE_Node *PARSE_convert_lvalue_to_rvalue(PARSE_Node *lv)
{
	PARSE_Node *ans = PARSE_get_node();

   *ans = *lv;

	switch(lv->type)
	{
		case PARSE_NODE_TYPE_VAR_ADDRESS:
			ans->type = PARSE_NODE_TYPE_VAR_VALUE;
			break;

		case PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS:

			ans->type   = PARSE_NODE_TYPE_PUSH_FIELD_VALUE;
			ans->child1 = PARSE_convert_lvalue_to_rvalue(ans->child1);
			ans->child2 = PARSE_copy_tree(ans->child2);

			//
			// We need a 'QUICK' copy from the child
			//
			
			ans->child1->flag |= PARSE_NODE_FLAG_EXTRACT;

			break;

		case PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS:
			ans->type = PARSE_NODE_TYPE_PUSH_ARRAY_VALUE;
			ans->child1 = PARSE_convert_lvalue_to_rvalue(ans->child1);
			ans->child2 = PARSE_copy_tree(ans->child2);

			//
			// We need a 'QUICK' copy from the child
			//
			
			ans->child1->flag |= PARSE_NODE_FLAG_EXTRACT;

			break;

		default:
			ASSERT(0);
			break;
	}

	return ans;
}

//
// Converts an rvalue to an lvalue. Only work on certain nodes... of course!
//

void PARSE_convert_rvalue_to_lvalue(PARSE_Node *rv)
{
	switch(rv->type)
	{
		case PARSE_NODE_TYPE_VAR_VALUE:
			rv->type = PARSE_NODE_TYPE_VAR_ADDRESS;
			break;

		case PARSE_NODE_TYPE_PUSH_FIELD_VALUE:

			rv->type = PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS;

			PARSE_convert_rvalue_to_lvalue(rv->child1);

			break;

		case PARSE_NODE_TYPE_PUSH_ARRAY_VALUE:

			rv->type = PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS;

			PARSE_convert_rvalue_to_lvalue(rv->child1);

			break;

		default:
			ASSERT(0);
			break;
	}
}


//
// Call this function on all top-level nodes that are in argument
// lists to function-calls. It makes sure that all variables are
// passed by reference.
//

void PARSE_convert_rvalue_to_argument(PARSE_Node *arg)
{
	//
	// Sometimes we must change the type of the node's children.
	//

	switch(arg->type)
	{
		case PARSE_NODE_TYPE_VAR_VALUE:
		case PARSE_NODE_TYPE_PUSH_FIELD_VALUE:
		case PARSE_NODE_TYPE_PUSH_ARRAY_VALUE:
			PARSE_convert_rvalue_to_lvalue(arg);
			break;

		default:
			break;
	}
}




SLONG PARSE_trees_the_same(PARSE_Node *tree1, PARSE_Node *tree2)
{
	//
	// Make sure both tree have the same type.
	//

	if (tree1->type != tree2->type)
	{
		return FALSE;
	}

	//
	// For nodes with data, make sure the data is the same.
	//

	switch(tree1->type)
	{
		case PARSE_NODE_TYPE_SLUMBER:
			if (tree1->slumber != tree2->slumber) return FALSE;
			break;

		case PARSE_NODE_TYPE_FLUMBER:
			if (tree1->flumber != tree2->flumber) return FALSE;
			break;

		case PARSE_NODE_TYPE_STRING:
			if (strcmp(tree1->string, tree2->string) != 0) return FALSE;
			break;

		case PARSE_NODE_TYPE_VAR_VALUE:
		case PARSE_NODE_TYPE_VAR_ADDRESS:
		case PARSE_NODE_TYPE_CALL:
		case PARSE_NODE_TYPE_FUNCTION:
		case PARSE_NODE_TYPE_FIELD:
		case PARSE_NODE_TYPE_ARGUMENT:
		case PARSE_NODE_TYPE_LOCAL:
		case PARSE_NODE_TYPE_EXPORT:
			if (strcmp(tree1->variable, tree2->variable) != 0) return FALSE;
			break;

		case PARSE_NODE_TYPE_GOTO:
		case PARSE_NODE_TYPE_LABEL:
		case PARSE_NODE_TYPE_GOSUB:
			if (strcmp(tree1->label, tree2->label) != 0) return FALSE;
			break;

		case PARSE_NODE_TYPE_BOOLEAN:
			if (tree1->boolean != tree2->boolean) return FALSE;
			break;

		case PARSE_NODE_TYPE_NOP:
		case PARSE_NODE_TYPE_EQUALS:
		case PARSE_NODE_TYPE_PLUS:
		case PARSE_NODE_TYPE_MINUS:
		case PARSE_NODE_TYPE_UMINUS:
		case PARSE_NODE_TYPE_TIMES:
		case PARSE_NODE_TYPE_DIVIDE:
		case PARSE_NODE_TYPE_IF:
		case PARSE_NODE_TYPE_GT:
		case PARSE_NODE_TYPE_LT:
		case PARSE_NODE_TYPE_GTEQ:
		case PARSE_NODE_TYPE_LTEQ:
		case PARSE_NODE_TYPE_AND:
		case PARSE_NODE_TYPE_OR:
		case PARSE_NODE_TYPE_NOT:
		case PARSE_NODE_TYPE_DOT:
		case PARSE_NODE_TYPE_PRINT:
		case PARSE_NODE_TYPE_ASSIGN:
		case PARSE_NODE_TYPE_MOD:
		case PARSE_NODE_TYPE_SQRT:
		case PARSE_NODE_TYPE_NEWLINE:
		case PARSE_NODE_TYPE_ABS:
		case PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS:
		case PARSE_NODE_TYPE_PUSH_FIELD_VALUE:
		case PARSE_NODE_TYPE_EXP_LIST:
		case PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS:
		case PARSE_NODE_TYPE_PUSH_ARRAY_VALUE:
		case PARSE_NODE_TYPE_INPUT:
		case PARSE_NODE_TYPE_UNDEFINED:
		case PARSE_NODE_TYPE_STATEMENT_LIST:
		case PARSE_NODE_TYPE_EXIT:
		case PARSE_NODE_TYPE_RETURN:
		case PARSE_NODE_TYPE_XOR:
		case PARSE_NODE_TYPE_FOR:
		case PARSE_NODE_TYPE_NEXT:
		case PARSE_NODE_TYPE_NOTEQUAL:
		case PARSE_NODE_TYPE_RANDOM:
		case PARSE_NODE_TYPE_SWAP:
		case PARSE_NODE_TYPE_MIF:
		case PARSE_NODE_TYPE_MELSE:
		case PARSE_NODE_TYPE_MENDIF:
		case PARSE_NODE_TYPE_WHILE:
		case PARSE_NODE_TYPE_LOOP:
		case PARSE_NODE_TYPE_ENDFUNC:
		case PARSE_NODE_TYPE_TEXTURE:
		case PARSE_NODE_TYPE_BUFFER:
		case PARSE_NODE_TYPE_DRAW:
		case PARSE_NODE_TYPE_CLS:
		case PARSE_NODE_TYPE_FLIP:
		case PARSE_NODE_TYPE_KEY_ASSIGN:
		case PARSE_NODE_TYPE_KEY_VALUE:
		case PARSE_NODE_TYPE_INKEY_ASSIGN:
		case PARSE_NODE_TYPE_INKEY_VALUE:
		case PARSE_NODE_TYPE_TIMER:
		case PARSE_NODE_TYPE_SIN:
		case PARSE_NODE_TYPE_COS:
		case PARSE_NODE_TYPE_TAN:
		case PARSE_NODE_TYPE_ASIN:
		case PARSE_NODE_TYPE_ACOS:
		case PARSE_NODE_TYPE_ATAN:
		case PARSE_NODE_TYPE_ATAN2:
		case PARSE_NODE_TYPE_LEFT:
		case PARSE_NODE_TYPE_MID:
		case PARSE_NODE_TYPE_RIGHT:
		case PARSE_NODE_TYPE_MATRIX:
		case PARSE_NODE_TYPE_VECTOR:
			break;

		default:
			ASSERT(0);
			break;
	}

	//
	// Make sure both trees have the same number of children.
	//

	if ( tree1->child1 && !tree2->child1) return FALSE;
	if (!tree1->child1 &&  tree2->child1) return FALSE;

	if ( tree1->child2 && !tree2->child2) return FALSE;
	if (!tree1->child2 &&  tree2->child2) return FALSE;

	if ( tree1->child3 && !tree2->child3) return FALSE;
	if (!tree1->child3 &&  tree2->child3) return FALSE;

	//
	// The children must be the same too.
	//

	if (tree1->child1 && !PARSE_trees_the_same(tree1->child1, tree2->child1)) return FALSE;
	if (tree1->child2 && !PARSE_trees_the_same(tree1->child2, tree2->child2)) return FALSE;
	if (tree1->child3 && !PARSE_trees_the_same(tree1->child3, tree2->child3)) return FALSE;

	//
	// All fine!
	//

	return TRUE;
}





//
// Our parsing functions........
//

PARSE_Node *PARSE_labelled_statement_list();
PARSE_Node *PARSE_statement_list();
PARSE_Node *PARSE_statement();
PARSE_Node *PARSE_expression();
PARSE_Node *PARSE_expression_list();
PARSE_Node *PARSE_p1exp();
PARSE_Node *PARSE_p2exp();
PARSE_Node *PARSE_p3exp();
PARSE_Node *PARSE_p4exp();
PARSE_Node *PARSE_p5exp();
PARSE_Node *PARSE_primary();
PARSE_Node *PARSE_function_call();
PARSE_Node *PARSE_lvalue();
PARSE_Node *PARSE_var();
PARSE_Node *PARSE_struct();
PARSE_Node *PARSE_argument_definition();

SLONG       PARSE_expression_list_depth(PARSE_Node *explist);



//
// Recursive descent parsing...
//

PARSE_Node *PARSE_function_call()
{
	PARSE_Node *ans;

	LEX_Token lt;

	//
	// A function call.
	//

	lt = LEX_get();

	if (lt.type != LEX_TOKEN_TYPE_VARIABLE)
	{
		//
		// ERROR! What function are we calling?
		//

		PARSE_throw("Expected a function name");
	}

	LEX_pop();

	//
	// Put "()" onto the front of the function name so
	// we know that it's a function.
	//

	CBYTE name[LEX_MAX_STRING_LENGTH + 32];

	sprintf(name, "()%s", lt.variable);

	//
	// The start of an argument list?
	//

	lt = LEX_get();

	if (lt.type != LEX_TOKEN_TYPE_OPEN)
	{
		//
		// ERROR!
		//

		PARSE_throw("Expected an open bracket after the function name");
	}

	LEX_pop();

	lt = LEX_get();

	if (lt.type == LEX_TOKEN_TYPE_CLOSE)
	{
		//
		// No argments to the function.
		//

		LEX_pop();

		ans           = PARSE_get_node();
		ans->type     = PARSE_NODE_TYPE_CALL;
		ans->child1   = NULL;
		ans->args     = 0;
		ans->variable = PARSE_add_string(name);

		return ans;
	}

	//
	// This function call has an argument list.
	//

	ans           = PARSE_get_node();
	ans->type     = PARSE_NODE_TYPE_CALL;
	ans->child1   = PARSE_expression_list();
	ans->args     = PARSE_expression_list_depth(ans->child1);
	ans->variable = PARSE_add_string(name);

	lt = LEX_get();

	if (lt.type != LEX_TOKEN_TYPE_CLOSE)
	{
		//
		// ERROR!
		//

		PARSE_throw("No close bracket after the list of arguments");
	}

	LEX_pop();

	//
	// Descend the expression list and convert each rvalue to an arguement.
	//

	PARSE_Node *arg = ans->child1;

	while(1)
	{
		if (arg->type == PARSE_NODE_TYPE_EXP_LIST)
		{
			PARSE_convert_rvalue_to_argument(arg->child1);

			arg = arg->child2;
		}
		else
		{
			PARSE_convert_rvalue_to_argument(arg);

			break;
		}
	}

	return ans;

}



PARSE_Node *PARSE_primary()
{
	PARSE_Node *ans;

	LEX_Token lt = LEX_get();

	switch(lt.type)
	{
		case LEX_TOKEN_TYPE_MINUS:

			//
			// A unary minus!
			//

			LEX_pop();

			ans = PARSE_get_node();

			ans->type   = PARSE_NODE_TYPE_UMINUS;
			ans->child1 = PARSE_primary();

			return ans;
	
		case LEX_TOKEN_TYPE_SLUMBER:

			LEX_pop();

			ans = PARSE_get_node();

			ans->type    = PARSE_NODE_TYPE_SLUMBER;
			ans->slumber = lt.slumber;

			return ans;

		case LEX_TOKEN_TYPE_FLUMBER:
			
			LEX_pop();

			ans = PARSE_get_node();

			ans->type    = PARSE_NODE_TYPE_FLUMBER;
			ans->flumber = lt.flumber;

			return ans;

		case LEX_TOKEN_TYPE_VARIABLE:
			
			{
				CBYTE *varname = PARSE_add_string(lt.variable);

				//
				// This could be a function call.
				//

				LEX_pop();

				LEX_Token lookahead = LEX_get();

				if (lookahead.type == LEX_TOKEN_TYPE_OPEN)
				{
					//
					// This is a function call!
					//

					LEX_push(lt);

					ans = PARSE_function_call();
					
					return ans;
				}

				ans = PARSE_get_node();

				ans->type     = PARSE_NODE_TYPE_VAR_VALUE;
				ans->variable = varname;
				
				return ans;
			}

		case LEX_TOKEN_TYPE_OPEN:

			LEX_pop();

			ans = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// ERROR!
				//

				PARSE_throw("No matching close bracket found");
			}
			else
			{
				LEX_pop();

				return ans;
			}

		case LEX_TOKEN_TYPE_TRUE:

			LEX_pop();

			ans = PARSE_get_node();

			ans->type    = PARSE_NODE_TYPE_BOOLEAN;
			ans->boolean = TRUE;

			return ans;

		case LEX_TOKEN_TYPE_FALSE:

			LEX_pop();

			ans = PARSE_get_node();

			ans->type    = PARSE_NODE_TYPE_BOOLEAN;
			ans->boolean = FALSE;

			return ans;

		case LEX_TOKEN_TYPE_NOT:

			LEX_pop();

			ans = PARSE_get_node();

			ans->type   = PARSE_NODE_TYPE_NOT;
			ans->child1 = PARSE_primary();

			return ans;

		case LEX_TOKEN_TYPE_STRING:

			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_STRING;
			ans->string = PARSE_add_string(lt.string);
	
			return ans;

		case LEX_TOKEN_TYPE_INPUT:

			LEX_pop();

			ans = PARSE_get_node();

			ans->type = PARSE_NODE_TYPE_INPUT;

			return ans;

		case LEX_TOKEN_TYPE_UNDEFINED:
			
			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_UNDEFINED;

			return ans;

		case LEX_TOKEN_TYPE_RANDOM:

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_RANDOM;

			return ans;

		case LEX_TOKEN_TYPE_CALL:

			LEX_pop();

			ans = PARSE_function_call();
			
			return ans;
		
		case LEX_TOKEN_TYPE_TEXTURE:
		case LEX_TOKEN_TYPE_BUFFER:
			
			//
			// Creation of a texture or a buffer.
			//

			LEX_pop();

			ans = PARSE_get_node();

			switch(lt.type)
			{
				case LEX_TOKEN_TYPE_TEXTURE: ans->type = PARSE_NODE_TYPE_TEXTURE; break;
				case LEX_TOKEN_TYPE_BUFFER:	 ans->type = PARSE_NODE_TYPE_BUFFER;  break;

				default:
					ASSERT(0);
					break;
			}

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				switch(ans->type)
				{
					case PARSE_NODE_TYPE_TEXTURE: PARSE_throw("No open bracket after the keyword TEXTURE"); break;
					case PARSE_NODE_TYPE_BUFFER:  PARSE_throw("No open bracket after the keyword BUFFER");  break;

					default:
						ASSERT(0);
				}
			}

			LEX_pop();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// No arguments 
				//

				LEX_pop();

				ans->child1 = NULL;
				ans->args   = 0;
			}
			else
			{
				//
				// An argument list.
				//

				ans->child1 = PARSE_expression_list();
				ans->args   = PARSE_expression_list_depth(ans->child1);

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_CLOSE)
				{
					//
					// ERROR!
					//

					switch(ans->type)
					{
						case PARSE_NODE_TYPE_TEXTURE: PARSE_throw("No close bracket after the keyword TEXTURE"); break;
						case PARSE_NODE_TYPE_BUFFER:  PARSE_throw("No close bracket after the keyword BUFFER");  break;

						default:
							ASSERT(0);
					}
				}

				LEX_pop();
			}

			return ans;


		case LEX_TOKEN_TYPE_KEY:

			LEX_pop();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OSQUARE)
			{
				//
				// ERROR!
				//

				PARSE_throw("KEY must be accessed like an array, e.g. KEY[50] or KEY[x]");
			}

			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_KEY_VALUE;
			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CSQUARE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Missing close bracket for the KEY keyword");
			}

			LEX_pop();

			return ans;

		case LEX_TOKEN_TYPE_INKEY:

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_INKEY_VALUE;

			return ans;

		case LEX_TOKEN_TYPE_TIMER:
			
			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_TIMER;

			return ans;

		case LEX_TOKEN_TYPE_SQRT:
		case LEX_TOKEN_TYPE_ABS:
		case LEX_TOKEN_TYPE_SIN:
		case LEX_TOKEN_TYPE_COS:
		case LEX_TOKEN_TYPE_TAN:
		case LEX_TOKEN_TYPE_ASIN:
		case LEX_TOKEN_TYPE_ACOS:
		case LEX_TOKEN_TYPE_ATAN:
		case LEX_TOKEN_TYPE_LEN:

			LEX_pop();

			ans = PARSE_get_node();

			switch(lt.type)
			{
				case LEX_TOKEN_TYPE_SQRT:  ans->type = PARSE_NODE_TYPE_SQRT;  break;
				case LEX_TOKEN_TYPE_ABS:   ans->type = PARSE_NODE_TYPE_ABS;	  break;
				case LEX_TOKEN_TYPE_SIN:   ans->type = PARSE_NODE_TYPE_SIN;	  break;
				case LEX_TOKEN_TYPE_COS:   ans->type = PARSE_NODE_TYPE_COS;	  break;
				case LEX_TOKEN_TYPE_TAN:   ans->type = PARSE_NODE_TYPE_TAN;	  break;
				case LEX_TOKEN_TYPE_ASIN:  ans->type = PARSE_NODE_TYPE_ASIN;  break;
				case LEX_TOKEN_TYPE_ACOS:  ans->type = PARSE_NODE_TYPE_ACOS;  break;
				case LEX_TOKEN_TYPE_ATAN:  ans->type = PARSE_NODE_TYPE_ATAN;  break;
				case LEX_TOKEN_TYPE_LEN:   ans->type = PARSE_NODE_TYPE_LEN;   break;

				default:
					ASSERT(0);
					break;
			}

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				switch(ans->type)
				{
					case PARSE_NODE_TYPE_SQRT: PARSE_throw("Missing open bracket after SQRT"); break;
					case PARSE_NODE_TYPE_ABS:  PARSE_throw("Missing open bracket after ABS");  break;
					case PARSE_NODE_TYPE_SIN:  PARSE_throw("Missing open bracket after SIN");  break;
					case PARSE_NODE_TYPE_COS:  PARSE_throw("Missing open bracket after COS");  break;
					case PARSE_NODE_TYPE_TAN:  PARSE_throw("Missing open bracket after TAN");  break;
					case PARSE_NODE_TYPE_ASIN: PARSE_throw("Missing open bracket after ASIN"); break;
					case PARSE_NODE_TYPE_ACOS: PARSE_throw("Missing open bracket after ACOS"); break;
					case PARSE_NODE_TYPE_ATAN: PARSE_throw("Missing open bracket after ATAN"); break;
					case PARSE_NODE_TYPE_LEN:  PARSE_throw("Missing open bracket after LEN");  break;

					default:
						ASSERT(0);
						break;
				}
			}

			LEX_pop();

			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// ERROR!
				//

				switch(ans->type)
				{
					case PARSE_NODE_TYPE_SQRT: PARSE_throw("Missing close bracket after SQRT"); break;
					case PARSE_NODE_TYPE_ABS:  PARSE_throw("Missing close bracket after ABS");  break;
					case PARSE_NODE_TYPE_SIN:  PARSE_throw("Missing close bracket after SIN");  break;
					case PARSE_NODE_TYPE_COS:  PARSE_throw("Missing close bracket after COS");  break;
					case PARSE_NODE_TYPE_TAN:  PARSE_throw("Missing close bracket after TAN");  break;
					case PARSE_NODE_TYPE_ASIN: PARSE_throw("Missing close bracket after ASIN"); break;
					case PARSE_NODE_TYPE_ACOS: PARSE_throw("Missing close bracket after ACOS"); break;
					case PARSE_NODE_TYPE_ATAN: PARSE_throw("Missing close bracket after ATAN"); break;
					case PARSE_NODE_TYPE_LEN:  PARSE_throw("Missing close bracket after LEN");  break;

					default:
						ASSERT(0);
						break;
				}
			}

			LEX_pop();

			return ans;
	
		case LEX_TOKEN_TYPE_ATAN2:

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_ATAN2;

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				PARSE_throw("Missing open bracket after ATAN2");
			}

			LEX_pop();

			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_COMMA)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a comma separating the two arguments to ATAN2");
			}

			LEX_pop();

			ans->child2 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Missing close bracket after ATAN2");
			}

			LEX_pop();

			return ans;

		case LEX_TOKEN_TYPE_LEFT:
		case LEX_TOKEN_TYPE_RIGHT:
			
			//
			// These functions take either one or two arguments.
			//

			LEX_pop();

			ans = PARSE_get_node();

			switch(lt.type)
			{
				case LEX_TOKEN_TYPE_LEFT:  ans->type = PARSE_NODE_TYPE_LEFT;  break;
				case LEX_TOKEN_TYPE_RIGHT: ans->type = PARSE_NODE_TYPE_RIGHT; break;

				default:
					ASSERT(0);
					break;
			}

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				switch(ans->type)
				{
					case PARSE_NODE_TYPE_LEFT:  PARSE_throw("Missing open bracket after LEFT");  break;
					case PARSE_NODE_TYPE_RIGHT:	PARSE_throw("Missing open bracket after RIGHT"); break;

					default:
						ASSERT(0);
						break;
				}
			}

			LEX_pop();

			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_CLOSE)
			{
				LEX_pop();

				//
				// This is a one-argugment version.
				//

				ans->child2 = NULL;

				return ans;
			}
			else
			if (lt.type != LEX_TOKEN_TYPE_COMMA)
			{
				//
				// ERROR!
				//

				switch(ans->type)
				{
					case PARSE_NODE_TYPE_LEFT:  PARSE_throw("Expected a comma or a close bracket after the first argument to LEFT");  break;
					case PARSE_NODE_TYPE_RIGHT:	PARSE_throw("Expected a comma or a close bracket after the first argument to RIGHT"); break;

					default:
						ASSERT(0);
						break;
				}
			}

			LEX_pop();

			ans->child2 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// ERROR!
				//

				switch(ans->type)
				{
					case PARSE_NODE_TYPE_LEFT:  PARSE_throw("Expected a close bracket after LEFT");  break;
					case PARSE_NODE_TYPE_RIGHT:	PARSE_throw("Expected a close bracket after RIGHT"); break;

					default:
						ASSERT(0);
						break;
				}
			}

			LEX_pop();

			return ans;

		case LEX_TOKEN_TYPE_MID:

			//
			// A two or three arguement function.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_MID;

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected an open bracket after MID");
			}

			LEX_pop();

			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_COMMA)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a comma after the first argument to MID");
			}

			LEX_pop();

			ans->child2 = PARSE_expression();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_CLOSE)
			{
				LEX_pop();

				//
				// This is a two-argument version.
				//

				ans->child3 = NULL;

				return ans;
			}
			else
			if (lt.type != LEX_TOKEN_TYPE_COMMA)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a comma or close bracket after the second argument to MID");
			}

			LEX_pop();

			ans->child3 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a close bracket after the third argument to MID");
			}

			LEX_pop();

			return ans;

		case LEX_TOKEN_TYPE_MATRIX:

			//
			// A matrix constant.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_MATRIX;
			
			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected an open bracket after MATRIX");
			}

			LEX_pop();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// This is the identity matrix.
				//

				ans->child1 = NULL;
			}
			else
			{
				//
				// There must be an expression list.
				//

				ans->child1 = PARSE_expression_list();
				ans->args   = PARSE_expression_list_depth(ans->child1);

				//
				// Right number of arguments? 3 args that must all be vectors,
				//

				if (ans->	args != 3)
				{
					//
					// ERROR!
					//

					PARSE_throw("MATRIX wants three vector arguments");
				}

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_CLOSE)
				{
					//
					// ERROR!
					//

					PARSE_throw("Expected a close bracket after the argument list to MATRIX");
				}

				LEX_pop();
			}

			return ans;

		case LEX_TOKEN_TYPE_VECTOR:

			//
			// A matrix constant.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_VECTOR;
			
			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected an open bracket after VECTOR");
			}

			LEX_pop();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// This is the zero vector (0,0,0).
				//

				ans->child1 = NULL;
			}
			else
			{
				//
				// There must be an expression list.
				//

				ans->child1 = PARSE_expression_list();
				ans->args   = PARSE_expression_list_depth(ans->child1);

				//
				// Right number of arguments?
				//

				if (ans->args != 3)
				{
					//
					// ERROR!
					//

					PARSE_throw("The VECTOR command expects either 3 arguments or an empty argument list");
				}

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_CLOSE)
				{
					//
					// ERROR!
					//

					PARSE_throw("Expected a close bracket after the argument list to VECTOR");
				}

				LEX_pop();
			}

			return ans;

		default:

			//
			// ERROR!
			//

			PARSE_throw();

			return NULL;
	}
}



PARSE_Node *PARSE_p6exp()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_primary();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_DOT:

				LEX_pop();

				rhs = PARSE_struct();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				ans->type   = PARSE_NODE_TYPE_PUSH_FIELD_VALUE;
				ans->child1 = lhs;
				ans->child2 = rhs;

				lhs->flag |= PARSE_NODE_FLAG_EXTRACT;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			case LEX_TOKEN_TYPE_OSQUARE:
				
				LEX_pop();

				rhs = PARSE_expression_list();

				ans = PARSE_get_node();

				ans->type       = PARSE_NODE_TYPE_PUSH_ARRAY_VALUE;
				ans->dimensions = PARSE_expression_list_depth(rhs);
				ans->child1     = lhs;
				ans->child2     = rhs;

				lhs->flag |= PARSE_NODE_FLAG_EXTRACT;

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_CSQUARE)
				{
					//
					// ERROR!
					//

					PARSE_throw("Missing close square bracket in an array access");
				}

				LEX_pop();

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}




PARSE_Node *PARSE_p5exp()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_p6exp();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_TIMES:
			case LEX_TOKEN_TYPE_DIVIDE:
			case LEX_TOKEN_TYPE_MOD:
			case LEX_TOKEN_TYPE_CPROD:

				LEX_pop();

				rhs = PARSE_p6exp();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				switch(lt.type)
				{
					case LEX_TOKEN_TYPE_TIMES:  ans->type = PARSE_NODE_TYPE_TIMES;  break;
					case LEX_TOKEN_TYPE_DIVIDE: ans->type = PARSE_NODE_TYPE_DIVIDE; break;
					case LEX_TOKEN_TYPE_MOD:    ans->type = PARSE_NODE_TYPE_MOD;    break;

					default:
						ASSERT(0);
						break;
				}

				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}



PARSE_Node *PARSE_p4exp()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_p5exp();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_PLUS:
			case LEX_TOKEN_TYPE_MINUS:

				LEX_pop();

				rhs = PARSE_p5exp();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				switch(lt.type)
				{
					case LEX_TOKEN_TYPE_PLUS:  ans->type = PARSE_NODE_TYPE_PLUS;  break;
					case LEX_TOKEN_TYPE_MINUS: ans->type = PARSE_NODE_TYPE_MINUS; break;

					default:
						ASSERT(0);
						break;
				}

				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}

PARSE_Node *PARSE_p3exp()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_p4exp();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_LT:
			case LEX_TOKEN_TYPE_GT:
			case LEX_TOKEN_TYPE_LTEQ:
			case LEX_TOKEN_TYPE_GTEQ:

				LEX_pop();

				rhs = PARSE_p4exp();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				switch(lt.type)
				{
					case LEX_TOKEN_TYPE_LT:   ans->type = PARSE_NODE_TYPE_LT;   break;
					case LEX_TOKEN_TYPE_GT:   ans->type = PARSE_NODE_TYPE_GT;   break;
					case LEX_TOKEN_TYPE_LTEQ: ans->type = PARSE_NODE_TYPE_LTEQ; break;
					case LEX_TOKEN_TYPE_GTEQ: ans->type = PARSE_NODE_TYPE_GTEQ; break;

					default:
						ASSERT(0);
						break;
				}

				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}




PARSE_Node *PARSE_p2exp()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_p3exp();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_EQUALS:
			case LEX_TOKEN_TYPE_NOTEQUAL:

				LEX_pop();

				rhs = PARSE_p3exp();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				switch(lt.type)
				{
					case LEX_TOKEN_TYPE_EQUALS:	  ans->type = PARSE_NODE_TYPE_EQUALS;   break;
					case LEX_TOKEN_TYPE_NOTEQUAL: ans->type = PARSE_NODE_TYPE_NOTEQUAL; break;

					default:
						ASSERT(0);
						break;
				}

				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}



PARSE_Node *PARSE_p1exp()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_p2exp();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_AND:

				LEX_pop();

				rhs = PARSE_p2exp();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				ans->type   = PARSE_NODE_TYPE_AND;
				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}




PARSE_Node *PARSE_expression()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_p1exp();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_OR:
			case LEX_TOKEN_TYPE_XOR:

				LEX_pop();

				rhs = PARSE_p1exp();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				switch(lt.type)
				{
					case LEX_TOKEN_TYPE_OR:	 ans->type = PARSE_NODE_TYPE_OR;  break;
					case LEX_TOKEN_TYPE_XOR: ans->type = PARSE_NODE_TYPE_XOR; break;

					default:
						ASSERT(0);
						break;
				}
				
				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:

				//
				// Make this whole tree as belonging to an expression.
				//

				PARSE_traverse(lhs, PARSE_set_expression_flag);

				return lhs;
		}
	}
}


PARSE_Node *PARSE_expression_list()
{
	LEX_Token lt;

	PARSE_Node *exp;
	PARSE_Node *ans;

	exp = PARSE_expression();

	lt = LEX_get();

	if (lt.type == LEX_TOKEN_TYPE_COMMA)
	{
		LEX_pop();

		ans = PARSE_get_node();

		ans->type   = PARSE_NODE_TYPE_EXP_LIST;
		ans->child1 = exp;
		ans->child2 = PARSE_expression_list();

		return ans;
	}
	else
	{
		return exp;
	}
}

//
// Returns the depth of an expressionlist.
//

SLONG PARSE_expression_list_depth(PARSE_Node *explist)
{
	if (explist->type != PARSE_NODE_TYPE_EXP_LIST)
	{
		return 1;
	}
	else
	{
		return 1 + PARSE_expression_list_depth(explist->child2);
	}
}



PARSE_Node *PARSE_var()
{
	//
	// No arrays for now!
	//

	PARSE_Node *ans;

	LEX_Token lt = LEX_get();

	if (lt.type == LEX_TOKEN_TYPE_VARIABLE)
	{
		LEX_pop();

		ans = PARSE_get_node();

		ans->type     = PARSE_NODE_TYPE_VAR_ADDRESS;
		ans->variable = PARSE_add_string(lt.variable);

		return ans;
	}

	//
	// ERROR!
	//

	PARSE_throw();

	return NULL;
}

PARSE_Node *PARSE_struct()
{
	//
	// No arrays for now!
	//

	PARSE_Node *ans;

	LEX_Token lt = LEX_get();

	if (lt.type == LEX_TOKEN_TYPE_VARIABLE)
	{
		LEX_pop();

		ans = PARSE_get_node();

		ans->type = PARSE_NODE_TYPE_FIELD;

		{
			//
			// Build the field name (insert a '.' at the beginning)
			//

			CBYTE field[LEX_MAX_STRING_LENGTH + 32];

			sprintf(field, ".%s", lt.variable);

			ans->field = PARSE_add_string(field);
		}

		return ans;
	}

	//
	// ERROR!
	//

	PARSE_throw();

	return NULL;
}




PARSE_Node *PARSE_lvalue()
{
	LEX_Token lt;

	PARSE_Node *lhs;
	PARSE_Node *rhs;
	PARSE_Node *ans;

	lhs = PARSE_var();
	
	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_DOT:

				LEX_pop();

				rhs = PARSE_struct();

				//
				// Build a little tree...
				//

				ans = PARSE_get_node();

				ans->type   = PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS;
				ans->child1 = lhs;
				ans->child2 = rhs;

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			case LEX_TOKEN_TYPE_OSQUARE:

				LEX_pop();

				//
				// Get a list of numbers...
				//

				rhs = PARSE_expression_list();

				ans = PARSE_get_node();

				ans->type       = PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS;
				ans->dimensions = PARSE_expression_list_depth(rhs);
				ans->child1     = lhs;
				ans->child2     = rhs;

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_CSQUARE)
				{
					//
					// ERROR!
					//

					PARSE_throw("Missing close square bracket in array access");
				}

				LEX_pop();

				//
				// Now carry on...
				//

				lhs = ans;

				break;

			default:
				return lhs;
		}
	}
}


PARSE_Node *PARSE_argument_definition()
{
	LEX_Token lt;

	//
	// Initailise our answer.
	//

	PARSE_Node *ans  = NULL;
	PARSE_Node *arg  = NULL;
	PARSE_Node *last = NULL;

	lt = LEX_get();

	if (lt.type != LEX_TOKEN_TYPE_OPEN)
	{
		//
		// No arguements?
		//

		return NULL;
	}

	LEX_pop();

	lt = LEX_get();

	if (lt.type == LEX_TOKEN_TYPE_CLOSE)
	{
		LEX_pop();

		//
		// An empty argument call...
		//

		ans       = PARSE_get_node();
		ans->type = PARSE_NODE_TYPE_NOP;

		return ans;
	}

	while(1)
	{
		lt = LEX_get();

		switch(lt.type)
		{
			case LEX_TOKEN_TYPE_VARIABLE:

				LEX_pop();

				//
				// Create another argument.
				//

				arg = PARSE_get_node();

				arg->type     = PARSE_NODE_TYPE_ARGUMENT;
				arg->variable = PARSE_add_string(lt.variable);
				arg->child1   = NULL;

				//
				// Is this the first argument?
				//

				if (ans == NULL)
				{
					ans = arg;
				}

				//
				// Do we have to put this onto the end of existing tree
				// of arguments?
				//

				if (last)
				{
					last->child1 = arg;
				}

				last = arg;

				//
				// End of the list or is there another one?
				//

				lt = LEX_get();

				if (lt.type == LEX_TOKEN_TYPE_COMMA)
				{
					//
					// Another variable... 
					//

					LEX_pop();
				}
				else
				if (lt.type == LEX_TOKEN_TYPE_CLOSE)
				{
					//
					// End of the list.
					//

					LEX_pop();

					return ans;
				}
				else
				{
					//
					// ERROR!
					//

					PARSE_throw("Expected a comma or close bracket in argument list");
				}

				break;

			default:
				
				//
				// ERROR!
				//

				PARSE_throw("Something odd in the argument list - only variables allowed!");
		}
	}
}





PARSE_Node *PARSE_statement()
{
	PARSE_Node *ans;

	LEX_Token lt = LEX_get();

	switch(lt.type)
	{
		case LEX_TOKEN_TYPE_PRINT:

			LEX_pop();

			ans = PARSE_get_node();

			if (LEX_get().type == LEX_TOKEN_TYPE_NEWLINE ||
				LEX_get().type == LEX_TOKEN_TYPE_COLON)
			{
				ans->type = PARSE_NODE_TYPE_NEWLINE;
			}
			else
			{
				ans->type   = PARSE_NODE_TYPE_PRINT;
				ans->child1 = PARSE_expression();
			}

			return ans;

		case LEX_TOKEN_TYPE_GOTO:
		case LEX_TOKEN_TYPE_GOSUB:

			LEX_pop();

			ans = PARSE_get_node();

			switch(lt.type)
			{
				case LEX_TOKEN_TYPE_GOTO:  ans->type = PARSE_NODE_TYPE_GOTO;  break;
				case LEX_TOKEN_TYPE_GOSUB: ans->type = PARSE_NODE_TYPE_GOSUB; break;

				default:
					ASSERT(0);
					break;
			}

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_SLUMBER)
			{
				LEX_pop();

				CBYTE  label[32];
				CBYTE *ch;

				itoa(lt.slumber, label, 10);

				for (ch = label; *ch; ch++);

				ch[0] = ':';
				ch[1] = '\000';

				ans->label = PARSE_add_string(label);
			}
			else
			if (lt.type == LEX_TOKEN_TYPE_VARIABLE)
			{
				LEX_pop();

				strcat(lt.variable, ":");

				ans->label = PARSE_add_string(lt.variable);
			}
			else
			{
				//
				// ERROR!
				//

				PARSE_throw("You can only GOTO labels or line numbers");
			}

			return ans;

		case LEX_TOKEN_TYPE_VARIABLE:

			{
				CBYTE *varname = PARSE_add_string(lt.variable);

				//
				// This could be a function call.
				//

				LEX_pop();

				LEX_Token lookahead = LEX_get();

				if (lookahead.type == LEX_TOKEN_TYPE_OPEN)
				{
					//
					// This is a function call!
					//

					LEX_push(lt);

					ans = PARSE_function_call();
					
					return ans;
				}

				//
				// The 'variable' part could have been
				// overwritten by the LEX_get()...
				//

				lt.variable = varname;

				LEX_push(lt);

				//
				// Variable assignement...
				//

				ans = PARSE_get_node();

				ans->type   = PARSE_NODE_TYPE_ASSIGN;
				ans->child1 = PARSE_lvalue();

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_EQUALS)
				{
					//
					// ERROR!
					//

					PARSE_throw("Missing equals after a variable (assuming you wanted to assign a value!)");
				}

				LEX_pop();

				ans->child2 = PARSE_expression();

				return ans;
			}

		case LEX_TOKEN_TYPE_IF:

			//
			// IF THEN (ELSE) statement.
			//

			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_IF;
			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_NEWLINE)
			{
				//
				// This must be a multi-line if.
				//

				ans->type   = PARSE_NODE_TYPE_MIF;
				ans->child2 = NULL;
				ans->child3 = NULL;

				return ans;
			}

			if (lt.type != LEX_TOKEN_TYPE_THEN)
			{
				//
				// ERROR!
				//

				PARSE_throw("Missing THEN after an IF statement");
			}

			LEX_pop();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_NEWLINE)
			{
				//
				// This must be a multi-line if.
				//

				ans->type   = PARSE_NODE_TYPE_MIF;
				ans->child2 = NULL;
				ans->child3 = NULL;

				return ans;
			}

			ans->child2 = PARSE_statement_list();

			//
			// Tell the THEN statements they are part of a conditional.
			//

			PARSE_traverse(ans->child2, PARSE_set_conditional_flag);

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_ELSE)
			{
				//
				// This IF statement has an ELSE bit!
				//

				LEX_pop();

				ans->child3 = PARSE_statement_list();

				//
				// Tell the ELSE statements they are part of a conditional.
				//

				PARSE_traverse(ans->child3, PARSE_set_conditional_flag);
			}
			else
			{
				//
				// IF and THEN without an ELSE.
				//

				ans->child3 = NULL;
			}

			return ans;

		case LEX_TOKEN_TYPE_NEWLINE:
			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_NOP;
			return ans;

		case LEX_TOKEN_TYPE_EXIT:

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_EXIT;

			return ans;

		case LEX_TOKEN_TYPE_FOR:

			{
				LEX_pop();

				PARSE_forcode++;

				ans          = PARSE_get_node();
				ans->type    = PARSE_NODE_TYPE_FOR;
				ans->forcode = PARSE_forcode;

				//
				// The initialisation.
				//

				ans->child1  = PARSE_statement_list();

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_TO)
				{
					//
					// ERROR!
					//

					PARSE_throw("Missing TO in the FOR statement");
				}

				LEX_pop();

				//
				// Find the variable by which the FOR loop is recognised.
				//

				{
					PARSE_Node *init;

					if (ans->child1->type == PARSE_NODE_TYPE_STATEMENT_LIST)
					{
						init = ans->child1->child1;
					}
					else
					{
						init = ans->child1;
					}

					if (init->type == PARSE_NODE_TYPE_ASSIGN)
					{
						//
						// Store the lvalue the FOR loop is initialising.
						//

						ans->lvalue = init->child1;
					}
					else
					{
						//
						// This is an anonymous FOR loop.
						//

						ans->lvalue = NULL;
					}
				}
				
				//
				// Now parse the end condition.
				//

				ans->child2 = PARSE_expression();

				if (PARSE_expression_is_boolean(ans->child2))
				{
					//
					// We don't have to create a boolean expression.
					//
				}
				else
				{
					//
					// Replace the expression with the condition that
					// this FOR loop's lvalue > the expression.
					//

					if (ans->lvalue == NULL)
					{
						//
						// ERROR!
						//

						PARSE_throw("Cannot establish a condition for terminating the FOR loop. Try making the TO part of the FOR loop a BOOLEAN expression");
					}

					PARSE_Node *cond = PARSE_get_node();

					cond->type   = PARSE_NODE_TYPE_GT;
					cond->child1 = PARSE_convert_lvalue_to_rvalue(ans->lvalue);
					cond->child2 = ans->child2;

					ans->child2 = cond;
				}

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_STEP)
				{
					if (ans->lvalue == NULL)
					{
						//
						// ERROR! Can't build a default STEP for an anonymous FOR loop.
						//

						PARSE_throw( "Cannot create a default STEP for the FOR loop. You must specify a STEP instruction");
					}

					//
					// Build a parse tree to increment this variable.
					//

					PARSE_Node *assign;
					PARSE_Node *varaddress;
					PARSE_Node *varvalue;
					PARSE_Node *one;
					PARSE_Node *add;

					varaddress = PARSE_copy_tree(ans->lvalue);
 					varvalue   = PARSE_convert_lvalue_to_rvalue(ans->lvalue);

					one          = PARSE_get_node();
					one->type    = PARSE_NODE_TYPE_SLUMBER;
					one->slumber = 1;

					add         = PARSE_get_node();
					add->type   = PARSE_NODE_TYPE_PLUS;
					add->child1 = varvalue;
					add->child2 = one;

					assign         = PARSE_get_node();
					assign->type   = PARSE_NODE_TYPE_ASSIGN;
					assign->child1 = varaddress;
					assign->child2 = add;

					//
					// And make this our STEP instruction.
					//

					ans->child3 = assign;
				}
				else
				{
					LEX_pop();

					//
					// We have a STEP statement!
					//

					ans->child3 = PARSE_statement_list();
				}
			}

			return ans;

		case LEX_TOKEN_TYPE_NEXT:
			
			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_NEXT;

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_VARIABLE)
			{
				//
				// The NEXT statement that knows which FOR loop it belongs to.
				//

				ans->lvalue = PARSE_lvalue();
			}
			else
			{
				//
				// An anonymous NEXT.
				//

				ans->lvalue = NULL;
			}

			return ans;

		case LEX_TOKEN_TYPE_SWAP:
			
			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_SWAP;
			ans->child1 = PARSE_lvalue();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_AND)
			{
				//
				// ERROR!
				//

				PARSE_throw("The two things to swap must be separated by an AND with a SWAP statment. e.g. SWAP x AND w");
			}

			LEX_pop();

			ans->child2 = PARSE_lvalue();

			return ans;

		case LEX_TOKEN_TYPE_ELSE:

			//
			// This must be the ELSE instruction of a multi-line IF.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_MELSE;

			return ans;

		case LEX_TOKEN_TYPE_ENDIF:

			//
			// This must be the ELSE instruction of a multi-line IF.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_MENDIF;

			return ans;

		case LEX_TOKEN_TYPE_WHILE:
			
			//
			// The start of a WHILE loop.
			//

			LEX_pop();

			PARSE_whilecode += 1;

			ans            = PARSE_get_node();
			ans->type      = PARSE_NODE_TYPE_WHILE;
			ans->whilecode = PARSE_whilecode;
			ans->child1    = PARSE_expression();

			return ans;

		case LEX_TOKEN_TYPE_LOOP:
			
			//
			// The end of a while loop.
			//

			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_LOOP;

			return ans;

		case LEX_TOKEN_TYPE_FUNCTION:
			
			//
			// A function definition!
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_FUNCTION;

			//
			// Name of the function...
			//

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_VARIABLE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected the name of the function after the FUNCTION keyword");
			}

			LEX_pop();

			{
				//
				// Put "()" onto the front of the function name so
				// we know that it's a function.
				//

				CBYTE name[LEX_MAX_STRING_LENGTH + 32];

				sprintf(name, "()%s", lt.variable);

				ans->variable = PARSE_add_string(name);
			}

			//
			// Argument declaration.
			//

			ans->child1 = PARSE_argument_definition();

			return ans;

		case LEX_TOKEN_TYPE_RETURN:

			//
			// Return from a function or subroutine.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_RETURN;

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_ELSE  ||
				lt.type == LEX_TOKEN_TYPE_ENDIF ||
				lt.type == LEX_TOKEN_TYPE_COLON ||
				lt.type == LEX_TOKEN_TYPE_NEWLINE)
			{
				//
				// No argument to return...
				//

				ans->child1 = NULL;
			}
			else
			{
				//
				// The expression to be returned
				// 

				ans->child1 = PARSE_expression();
			}

			return ans;

		case LEX_TOKEN_TYPE_ENDFUNC:
			
			//
			// The end of a function declaration...
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_ENDFUNC;

			return ans;

		case LEX_TOKEN_TYPE_CALL:

			LEX_pop();

			ans = PARSE_function_call();
			
			return ans;

		case LEX_TOKEN_TYPE_DRAW:
			
			//
			// Drawing something!
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_DRAW;

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// ERROR!
				//

				PARSE_throw("Missing open bracket after DRAW");
			}

			LEX_pop();

			lt = LEX_get();

			if (lt.type == LEX_TOKEN_TYPE_CLOSE)
			{
				//
				// No arguments 
				//

				LEX_pop();

				ans->child1 = NULL;
				ans->args   = 0;
			}
			else
			{
				//
				// An argument list.
				//

				ans->child1 = PARSE_expression_list();
				ans->args   = PARSE_expression_list_depth(ans->child1);

				lt = LEX_get();

				if (lt.type != LEX_TOKEN_TYPE_CLOSE)
				{
					//
					// ERROR!
					//

					PARSE_throw("Missing close bracket after DRAW");
				}

				LEX_pop();
			}

			return ans;

		case LEX_TOKEN_TYPE_CLS:
			
			//
			// Clear the screen.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_CLS;

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OPEN)
			{
				//
				// A CLS without an argument list.
				//

				ans->child1 = NULL;
				ans->args   = 0;
			}
			else
			{
				LEX_pop();

				lt = LEX_get();

				if (lt.type == LEX_TOKEN_TYPE_CLOSE)
				{
					//
					// No arguments 
					//

					LEX_pop();

					ans->child1 = NULL;
					ans->args   = 0;
				}
				else
				{
					//
					// An argument list.
					//

					ans->child1 = PARSE_expression_list();
					ans->args   = PARSE_expression_list_depth(ans->child1);

					lt = LEX_get();

					if (lt.type != LEX_TOKEN_TYPE_CLOSE)
					{
						//
						// ERROR!
						//

						PARSE_throw("Missing close bracket after CLS");
					}

					LEX_pop();
				}
			}

			return ans;

		case LEX_TOKEN_TYPE_FLIP:
			
			//
			// A FLIP statement.
			//

			LEX_pop();

			ans       = PARSE_get_node();
			ans->type = PARSE_NODE_TYPE_FLIP;

			return ans;

		case LEX_TOKEN_TYPE_KEY:

			//
			// Assiging to the KEY array.
			//

			LEX_pop();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_OSQUARE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected an open square bracket after KEY");
			}

			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_KEY_ASSIGN;
			ans->child1 = PARSE_expression();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_CSQUARE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a close square bracket after KEY");
			}

			LEX_pop();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_EQUALS)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected an equals after KEY[...]");
			}

			LEX_pop();

			ans->child2 = PARSE_expression();

			return ans;

		case LEX_TOKEN_TYPE_INKEY:
			
			//
			// Assigning to INKEY
			//

			LEX_pop();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_EQUALS)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected an equals after INKEY");
			}

			LEX_pop();

			ans         = PARSE_get_node();
			ans->type   = PARSE_NODE_TYPE_INKEY_ASSIGN;
			ans->child1 = PARSE_expression();

			return ans;

		case LEX_TOKEN_TYPE_EXPORT:

			//
			// Exporting a global or function.
			//

			LEX_pop();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_VARIABLE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a variable list after EXPORT");
			}
			
			LEX_pop();

			ans           = PARSE_get_node();
			ans->type     = PARSE_NODE_TYPE_EXPORT;
			ans->variable = PARSE_add_string(lt.variable);

			{
				PARSE_Node *higher = ans;
				PARSE_Node *lower;

				//
				// We can have a whole long list of exports.
				//

				while(1)
				{
					lt = LEX_get();

					if (lt.type != LEX_TOKEN_TYPE_COMMA)
					{
						break;
					}

					LEX_pop();

					lt = LEX_get();

					if (lt.type != LEX_TOKEN_TYPE_VARIABLE)
					{
						//
						// ERROR!
						//

						PARSE_throw("Expected a variable after the comma in the EXPORT statement");
					}

					LEX_pop();

					//
					// Create a new node.
					//

					lower           = PARSE_get_node();
					lower->type     = PARSE_NODE_TYPE_EXPORT;
					lower->variable = PARSE_add_string(lt.variable);

					higher->child1 = lower;
					higher         = lower;
				}
			}

			return ans;

		case LEX_TOKEN_TYPE_LOCAL:

			//
			// Declaring a variable to be a local.
			//

			LEX_pop();

			lt = LEX_get();

			if (lt.type != LEX_TOKEN_TYPE_VARIABLE)
			{
				//
				// ERROR!
				//

				PARSE_throw("Expected a variable list after LOCAL");
			}

			LEX_pop();

			ans           = PARSE_get_node();
			ans->type     = PARSE_NODE_TYPE_LOCAL;
			ans->variable = PARSE_add_string(lt.variable);
			
			{
				PARSE_Node *higher = ans;
				PARSE_Node *lower;

				//
				// We can have a whole long list of locals.
				//

				while(1)
				{
					lt = LEX_get();

					if (lt.type != LEX_TOKEN_TYPE_COMMA)
					{
						break;
					}

					LEX_pop();

					lt = LEX_get();

					if (lt.type != LEX_TOKEN_TYPE_VARIABLE)
					{
						//
						// ERROR!
						//

						PARSE_throw("Expected a variable after the comma in the LOCAL statement");
					}

					LEX_pop();

					//
					// Create a new node.
					//

					lower           = PARSE_get_node();
					lower->type     = PARSE_NODE_TYPE_LOCAL;
					lower->variable = PARSE_add_string(lt.variable);

					higher->child1 = lower;
					higher         = lower;
				}
			}

			return ans;

		default:
			
			//
			// ERROR!
			//

			PARSE_throw();

			return NULL;	// To stop the compiler complaining that not all control paths return a value.
	}
}



PARSE_Node *PARSE_statement_list()
{
	LEX_Token lt;

	PARSE_Node *statement;

	statement = PARSE_statement();

	//
	// Should we continue to PARSE another statement?
	//

	SLONG another_statement = FALSE;

	lt = LEX_get();

	switch(lt.type)
	{
		case LEX_TOKEN_TYPE_COLON:

			//
			// We've found a colon separator.
			//

			LEX_pop();

			another_statement = TRUE;

			break;

		case LEX_TOKEN_TYPE_ENDIF:

			//
			// We don't need a colon before an ENDIF instruction...
			//
			
			another_statement = TRUE;

			break;
		
		case LEX_TOKEN_TYPE_NEWLINE:
			break;

		default:

			if (statement->type == PARSE_NODE_TYPE_MELSE)
			{
				//
				// We dont need a COLON before the next statement.
				//

				another_statement = TRUE;
			}

			break;
	}

	if (another_statement)
	{
		PARSE_Node *ans;

		ans = PARSE_get_node();

		ans->type   = PARSE_NODE_TYPE_STATEMENT_LIST;
		ans->child1 = statement;
		ans->child2 = PARSE_statement_list();

		return ans;
	}

	return statement;
}



PARSE_Node *PARSE_labelled_statement_list()
{
	PARSE_Node *ans;

	LEX_Token lt = LEX_get();

	if (lt.type == LEX_TOKEN_TYPE_FLUMBER)
	{
		//
		// ERROR! Labelling a line with a floating point number?
		//

		PARSE_throw("You can't label a line with a floating point number");
	}

	if (lt.type == LEX_TOKEN_TYPE_SLUMBER)
	{
		LEX_pop();

		//
		// This line is labelled with a number.
		//

		ans = PARSE_get_node();

		ans->type   = PARSE_NODE_TYPE_LABEL;

		CBYTE  label[32];
		CBYTE *ch;

		itoa(lt.slumber, label, 10);

		for (ch = label; *ch; ch++);

		ch[0] = ':';
		ch[1] = '\000';

		ans->label  = PARSE_add_string(label);
		ans->child1 = PARSE_statement();
	}
	else if (lt.type == LEX_TOKEN_TYPE_LABEL)
	{
		LEX_pop();

		//
		// This line is labelled with a proper label.
		//

		ans = PARSE_get_node();

		ans->type   = PARSE_NODE_TYPE_LABEL;
		ans->label  = PARSE_add_string(lt.label);
		ans->child1 = PARSE_statement();
	}
	else
	{
		//
		// No label...
		//

		ans = PARSE_statement_list();
	}

	lt = LEX_pop();

	if (lt.type != LEX_TOKEN_TYPE_NEWLINE)
	{
		//
		// ERROR!
		//

		PARSE_throw("Expected the end of the line but there was extra stuff");
	}

	return ans;
}



//
// The maximum length of a source file in bytes. This is pants!
//


CBYTE *PARSE_program;
SLONG  PARSE_program_upto;
SLONG  PARSE_program_max;



void PARSE_do(CBYTE *fname)
{
	SLONG       want_to_read;
	SLONG       bytes_read;
	LEX_Token   lt;
	PARSE_Node *nop;

	//
	// Initialise...
	//													   

	PARSE_node_upto         = 0;
	PARSE_string_table_upto = 0;
	PARSE_line_upto         = 0;
	PARSE_error_upto        = 0;
	PARSE_errbuf_upto       = 0;

	memset(PARSE_node, 0, sizeof(PARSE_node));

	if (PARSE_program_max == 0)
	{
		//
		// Allocate memory for the file.
		//

		PARSE_program_max = 16;
		PARSE_program     = (CBYTE *) malloc(sizeof(CBYTE) * PARSE_program_max);

		memset(PARSE_program, 0, sizeof(CBYTE) * PARSE_program_max);
	}

	//
	// Load the file.
	//

	FILE *handle = fopen(fname, "rb");

	if (!handle)
	{
		PARSE_add_error("Could not open source file \"%s\"", fname);

		return;
	}

	PARSE_program_upto = 0;

	while(1)
	{
		//
		// How many bytes till the end of our buffer?
		//

		want_to_read = PARSE_program_max - PARSE_program_upto;

		//
		// Try and load in that many bytes.
		//

		bytes_read = fread(
						PARSE_program + PARSE_program_upto,
						sizeof(CBYTE),
						want_to_read,
						handle);

		PARSE_program_upto += bytes_read;

		if (bytes_read == want_to_read)
		{
			//
			// More data to read, so allocate a bigger buffer.
			//

			PARSE_program_max *= 2;
			PARSE_program      = (CBYTE *) realloc(PARSE_program, sizeof(CBYTE) * PARSE_program_max);

			//
			// Zero out newly allocated memory.
			//

			memset(PARSE_program + (PARSE_program_max / 2), 0, sizeof(CBYTE) * PARSE_program_max / 2);
		}
		else
		{
			//
			// Finished reading program.
			//

			break;
		}
	}

	//
	// Give the input to the lexical analyser.
	//

	LEX_start(PARSE_program);

	//
	// This is where we pop back to when we get an error!
	//

	if (setjmp(PARSE_error_jmp) == 0)
	{
		//
		// This is the first call...
		//

		ASSERT(PARSE_line_upto == 0);
	}
	else
	{
		//
		// An error has occurred.
		//

		lt = LEX_get();

		if (lt.type == LEX_TOKEN_TYPE_ERROR)
		{
			//
			// Override the parse error with the lexical error.
			//

			PARSE_error_type = lt.error;
		}

		if (!PARSE_add_error("ERROR in %s line %d : %s", fname, PARSE_line_upto, PARSE_error_type))
		{
			//
			// Too many errors! ABORT!
			//

			return;
		}

		//
		// Try and recover. Go onto the next line.
		//

		LEX_next_line();

		//
		// Mark this line as blank.
		//

		nop       = PARSE_get_node();
		nop->type = PARSE_NODE_TYPE_NOP;

		PARSE_line[PARSE_line_upto++] = nop;
	}

	while(1)
	{
		lt = LEX_get();

		if (lt.type == LEX_TOKEN_TYPE_EOF)
		{
			//
			// Parsing complete!
			//

			break;
		}

		ASSERT(WITHIN(PARSE_line_upto, 0, PARSE_MAX_LINES - 1));

		PARSE_line[PARSE_line_upto++] = PARSE_labelled_statement_list();
	}
}


void PARSE_traverse_do(PARSE_Node *pn, SLONG (*user_function)(PARSE_Node *pn))
{
	switch(pn->type)
	{
		case PARSE_NODE_TYPE_NOP:
		case PARSE_NODE_TYPE_SLUMBER:
		case PARSE_NODE_TYPE_FLUMBER:
		case PARSE_NODE_TYPE_STRING:
		case PARSE_NODE_TYPE_VAR_VALUE:
		case PARSE_NODE_TYPE_VAR_ADDRESS:
		case PARSE_NODE_TYPE_GOTO:
		case PARSE_NODE_TYPE_BOOLEAN:
		case PARSE_NODE_TYPE_NEWLINE:
		case PARSE_NODE_TYPE_FIELD:
		case PARSE_NODE_TYPE_INPUT:
		case PARSE_NODE_TYPE_UNDEFINED:
		case PARSE_NODE_TYPE_EXIT:
		case PARSE_NODE_TYPE_GOSUB:
		case PARSE_NODE_TYPE_NEXT:
		case PARSE_NODE_TYPE_RANDOM:
		case PARSE_NODE_TYPE_MELSE:
		case PARSE_NODE_TYPE_MENDIF:
		case PARSE_NODE_TYPE_LOOP:
		case PARSE_NODE_TYPE_ENDFUNC:
		case PARSE_NODE_TYPE_FLIP:
		case PARSE_NODE_TYPE_INKEY_VALUE:
		case PARSE_NODE_TYPE_TIMER:
		case PARSE_NODE_TYPE_EXPORT:

			//
			// No children.
			//

			user_function(pn);

			break;

		case PARSE_NODE_TYPE_LABEL:
		case PARSE_NODE_TYPE_UMINUS:
		case PARSE_NODE_TYPE_NOT:
		case PARSE_NODE_TYPE_PRINT:
		case PARSE_NODE_TYPE_SQRT:
		case PARSE_NODE_TYPE_ABS:
		case PARSE_NODE_TYPE_MIF:
		case PARSE_NODE_TYPE_INKEY_ASSIGN:
		case PARSE_NODE_TYPE_KEY_VALUE:
		case PARSE_NODE_TYPE_SIN:
		case PARSE_NODE_TYPE_COS:
		case PARSE_NODE_TYPE_TAN:
		case PARSE_NODE_TYPE_ASIN:
		case PARSE_NODE_TYPE_ACOS:
		case PARSE_NODE_TYPE_ATAN:
		case PARSE_NODE_TYPE_LEN:

			//
			// One child.
			//

			PARSE_traverse_do(pn->child1, user_function);

			user_function(pn);

			break;

		case PARSE_NODE_TYPE_EQUALS:
		case PARSE_NODE_TYPE_PLUS:
		case PARSE_NODE_TYPE_MINUS:
		case PARSE_NODE_TYPE_TIMES:
		case PARSE_NODE_TYPE_DIVIDE:
		case PARSE_NODE_TYPE_GT:
		case PARSE_NODE_TYPE_LT:
		case PARSE_NODE_TYPE_GTEQ:
		case PARSE_NODE_TYPE_LTEQ:
		case PARSE_NODE_TYPE_AND:
		case PARSE_NODE_TYPE_OR:
		case PARSE_NODE_TYPE_XOR:
		case PARSE_NODE_TYPE_DOT:
		case PARSE_NODE_TYPE_MOD:
		case PARSE_NODE_TYPE_EXP_LIST:
		case PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS:
		case PARSE_NODE_TYPE_PUSH_ARRAY_VALUE:
		case PARSE_NODE_TYPE_STATEMENT_LIST:
		case PARSE_NODE_TYPE_NOTEQUAL:
		case PARSE_NODE_TYPE_SWAP:
		case PARSE_NODE_TYPE_ATAN2:

			//
			// Two children.
			//

			PARSE_traverse_do(pn->child1, user_function);
			PARSE_traverse_do(pn->child2, user_function);

			user_function(pn);

			break;

		case PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS:
		case PARSE_NODE_TYPE_PUSH_FIELD_VALUE:

			//
			// Two children- strange order of traversal!
			//

			PARSE_traverse_do(pn->child1, user_function);
			user_function(pn);
			PARSE_traverse_do(pn->child2, user_function);

			break;

		case PARSE_NODE_TYPE_IF:
			
			{
				//
				// Special case order of traversal including
				// putting in fake nodes to help out the CG module!
				//

				PARSE_Node fake_then;
				PARSE_Node fake_else;
				PARSE_Node fake_end_else;

				memset(&fake_then,     0, sizeof(fake_then    ));
				memset(&fake_else,     0, sizeof(fake_else    ));
				memset(&fake_end_else, 0, sizeof(fake_end_else));

				pn->ifcode = PARSE_ifcode;

				fake_then.type   = PARSE_NODE_TYPE_FAKE_THEN;
				fake_then.ifcode = PARSE_ifcode;

				fake_else.type   = PARSE_NODE_TYPE_FAKE_ELSE;
				fake_else.ifcode = PARSE_ifcode;

				fake_end_else.type   = PARSE_NODE_TYPE_FAKE_END_ELSE;
				fake_end_else.ifcode = PARSE_ifcode;

				PARSE_ifcode += 1;

				PARSE_traverse_do(pn->child1, user_function);	// Condition
				user_function(pn);								// IF node ...

				PARSE_traverse_do(pn->child2, user_function);	// 'THEN' statement
				user_function(&fake_then);						// The fake THEN node.

				if (pn->child3)
				{
					//
					// This IF statement has an ELSE.
					//

					user_function(&fake_else);						// The fake ELSE node.
					PARSE_traverse_do(pn->child3, user_function);	// 'ELSE' statement
					user_function(&fake_end_else);					// The fake END_ELSE node.
				}
			}

			break;

		case PARSE_NODE_TYPE_ASSIGN:
		case PARSE_NODE_TYPE_KEY_ASSIGN:

			//
			// Special case order of traversal.
			//

			PARSE_traverse_do(pn->child2, user_function);
			PARSE_traverse_do(pn->child1, user_function);

			user_function(pn);

			break;

		case PARSE_NODE_TYPE_FOR:

			{
				PARSE_Node fake_end_for;
				PARSE_Node fake_for_cond;

				memset(&fake_end_for,  0, sizeof(fake_end_for ));
				memset(&fake_for_cond, 0, sizeof(fake_for_cond));
								  
				fake_end_for.type    = PARSE_NODE_TYPE_FAKE_END_FOR;
				fake_end_for.forcode = pn->forcode;

				fake_for_cond.type    = PARSE_NODE_TYPE_FAKE_FOR_COND;
				fake_for_cond.forcode = pn->forcode;

				//
				// Initialisation.
				//

				PARSE_traverse_do(pn->child1, user_function);

				//
				// The FOR...
				//

				user_function(pn);

				//
				// The STEP code.
				//

				PARSE_traverse_do(pn->child3, user_function);

				//
				// The END_FOR code...
				//

				user_function(&fake_end_for);

				//
				// The condition.
				//

				PARSE_traverse_do(pn->child2, user_function);

				//
				// THE FOR_COND node...
				//

				user_function(&fake_for_cond);
			}

			break;

		case PARSE_NODE_TYPE_WHILE:

			{
				//
				// Build a FAKE_WHILE_COND node.
				//

				PARSE_Node fake_while_cond;

				memset(&fake_while_cond, 0, sizeof(fake_while_cond));

				fake_while_cond.type      = PARSE_NODE_TYPE_FAKE_WHILE_COND;
				fake_while_cond.whilecode = pn->whilecode;

				//
				// Mark the beginning of the while loop.
				//

				user_function(pn);

				//
				// The condition.
				//

				PARSE_traverse_do(pn->child1, user_function);
				
				//
				// Mark the end of the condition.
				//

				user_function(&fake_while_cond);
			}

			break;

		case PARSE_NODE_TYPE_FUNCTION:
		case PARSE_NODE_TYPE_ARGUMENT:
		case PARSE_NODE_TYPE_LOCAL:

			user_function(pn);

			if (pn->child1)
			{
				PARSE_traverse_do(pn->child1, user_function);
			}

			break;

		case PARSE_NODE_TYPE_CALL:
		case PARSE_NODE_TYPE_TEXTURE:
		case PARSE_NODE_TYPE_BUFFER:
		case PARSE_NODE_TYPE_DRAW:
		case PARSE_NODE_TYPE_CLS:
		case PARSE_NODE_TYPE_RETURN:
		case PARSE_NODE_TYPE_MATRIX:
		case PARSE_NODE_TYPE_VECTOR:
			
			//
			// Possibly one child.
			//

			if (pn->child1)
			{
				PARSE_traverse_do(pn->child1, user_function);
			}

			user_function(pn);

			break;

		case PARSE_NODE_TYPE_LEFT:
		case PARSE_NODE_TYPE_RIGHT:

			//
			// One or two kids...
			//

			PARSE_traverse_do(pn->child1, user_function);

			if (pn->child2)
			{
				PARSE_traverse_do(pn->child2, user_function);
			}

			user_function(pn);

			break;

		case PARSE_NODE_TYPE_MID:

			//
			// Two or three kids...
			//

			PARSE_traverse_do(pn->child1, user_function);
			PARSE_traverse_do(pn->child2, user_function);

			if (pn->child3)
			{
				PARSE_traverse_do(pn->child3, user_function);
			}

			user_function(pn);

			break;

		default:
			ASSERT(0);
			break;
	}

	return;
}


void PARSE_traverse(PARSE_Node *pn, SLONG (*user_function)(PARSE_Node *pn))
{
	//
	// Store stack info...
	//

	PARSE_traverse_do(pn, user_function);
}
