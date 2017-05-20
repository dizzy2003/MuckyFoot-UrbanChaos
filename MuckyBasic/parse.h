//
// A recursive descent parser.
//

#ifndef _PARSE_
#define _PARSE_



//
// Each line produces a node. A node is a tree structure.
//

#define PARSE_NODE_TYPE_NOP		            0	// For a blank line
#define PARSE_NODE_TYPE_EQUALS	            1
#define PARSE_NODE_TYPE_PLUS	            2
#define PARSE_NODE_TYPE_MINUS	            3
#define PARSE_NODE_TYPE_UMINUS	            4 	// Unary minus
#define PARSE_NODE_TYPE_TIMES  	            5
#define PARSE_NODE_TYPE_DIVIDE	            6
#define PARSE_NODE_TYPE_SLUMBER	            7
#define PARSE_NODE_TYPE_FLUMBER	            8
#define PARSE_NODE_TYPE_STRING	            9
#define PARSE_NODE_TYPE_VAR_VALUE          10
#define PARSE_NODE_TYPE_IF		           11
#define PARSE_NODE_TYPE_GOTO	           12
#define PARSE_NODE_TYPE_LABEL	           13	// Child1 is the rest of the statement for the line
#define PARSE_NODE_TYPE_GT		           14
#define PARSE_NODE_TYPE_LT		           15
#define PARSE_NODE_TYPE_GTEQ	           16
#define PARSE_NODE_TYPE_LTEQ	           17
#define PARSE_NODE_TYPE_AND		           18
#define PARSE_NODE_TYPE_OR		           19
#define PARSE_NODE_TYPE_NOT		           20
#define PARSE_NODE_TYPE_DOT		           21	// A fullstop that isn't part of a number
#define PARSE_NODE_TYPE_CALL	           22
#define PARSE_NODE_TYPE_LOCAL	           23	// Child1 may point to another local node...
#define PARSE_NODE_TYPE_PRINT	           24
#define PARSE_NODE_TYPE_ASSIGN             25	// Equals := , assignment...
#define PARSE_NODE_TYPE_VAR_ADDRESS        26
#define PARSE_NODE_TYPE_MOD                27
#define PARSE_NODE_TYPE_BOOLEAN            28
#define PARSE_NODE_TYPE_SQRT	           29
#define PARSE_NODE_TYPE_NEWLINE            30	// A PRINT without anything else...
#define PARSE_NODE_TYPE_ABS                31
#define PARSE_NODE_TYPE_PUSH_FIELD_ADDRESS 32
#define PARSE_NODE_TYPE_FIELD              33
#define PARSE_NODE_TYPE_PUSH_FIELD_VALUE   34
#define PARSE_NODE_TYPE_EXP_LIST           35
#define PARSE_NODE_TYPE_PUSH_ARRAY_ADDRESS 36
#define PARSE_NODE_TYPE_PUSH_ARRAY_VALUE   37
#define PARSE_NODE_TYPE_INPUT              38
#define PARSE_NODE_TYPE_UNDEFINED          39	// The constant value <UNDEFINED>
#define PARSE_NODE_TYPE_STATEMENT_LIST     40
#define PARSE_NODE_TYPE_EXIT               41
#define PARSE_NODE_TYPE_RETURN             42
#define PARSE_NODE_TYPE_GOSUB              43
#define PARSE_NODE_TYPE_XOR                44
#define PARSE_NODE_TYPE_FOR                45
#define PARSE_NODE_TYPE_NEXT               46
#define PARSE_NODE_TYPE_NOTEQUAL           47
#define PARSE_NODE_TYPE_RANDOM             48
#define PARSE_NODE_TYPE_SWAP               49
#define PARSE_NODE_TYPE_MIF                50	// A multi-line IF
#define PARSE_NODE_TYPE_MELSE              51	// The ELSE of a multi-line IF
#define PARSE_NODE_TYPE_MENDIF             52   // THE ENDIF of a multi-line IF
#define PARSE_NODE_TYPE_WHILE              53
#define PARSE_NODE_TYPE_LOOP               54
#define PARSE_NODE_TYPE_FUNCTION           55
#define PARSE_NODE_TYPE_ARGUMENT           56
#define PARSE_NODE_TYPE_ENDFUNC            57
#define PARSE_NODE_TYPE_TEXTURE            58
#define PARSE_NODE_TYPE_BUFFER             59
#define PARSE_NODE_TYPE_DRAW               60
#define PARSE_NODE_TYPE_CLS                61
#define PARSE_NODE_TYPE_FLIP               62
#define PARSE_NODE_TYPE_KEY_ASSIGN         63
#define PARSE_NODE_TYPE_KEY_VALUE          64
#define PARSE_NODE_TYPE_INKEY_ASSIGN       65
#define PARSE_NODE_TYPE_INKEY_VALUE        66
#define PARSE_NODE_TYPE_TIMER              67
#define PARSE_NODE_TYPE_SIN                68
#define PARSE_NODE_TYPE_COS                69
#define PARSE_NODE_TYPE_TAN                70
#define PARSE_NODE_TYPE_ASIN               71
#define PARSE_NODE_TYPE_ACOS               72
#define PARSE_NODE_TYPE_ATAN               73
#define PARSE_NODE_TYPE_ATAN2              74
#define PARSE_NODE_TYPE_EXPORT             75
#define PARSE_NODE_TYPE_LEFT               76
#define PARSE_NODE_TYPE_MID                77
#define PARSE_NODE_TYPE_RIGHT              78
#define PARSE_NODE_TYPE_LEN                79
#define PARSE_NODE_TYPE_MATRIX             80	// Child1 is an expression list or NULL
#define PARSE_NODE_TYPE_VECTOR             81	// Child1 is an expression list or NULL
#define PARSE_NODE_TYPE_DPROD              82
#define PARSE_NODE_TYPE_CPROD              83
#define PARSE_NODE_TYPE_NORMALISE          84
#define PARSE_NODE_TYPE_TRANSPOSE          85



#define PARSE_NODE_FLAG_EXTRACT		(1 << 0)	// If this node is the 'child1' of a PUSH_ARRAY_VALUE, PUSH_FIELD_VALUE, PUSH_ARRAY_ADDRESS or PUSH_FIELD_ADDRESS node. 
#define PARSE_NODE_FLAG_CONDITIONAL	(1 << 1)	// This node is in the THEN or ELSE part of an IF statement.
#define PARSE_NODE_FLAG_EXPRESSION  (1 << 2)	// This node is part of an expression.

typedef struct parse_node PARSE_Node;

typedef struct parse_node
{
	UWORD type;
	UWORD flag;

	union
	{
		SLONG       slumber;
		float       flumber;
		CBYTE      *string;
		CBYTE      *label;
		CBYTE      *variable;
		SLONG       boolean;
		CBYTE      *field;		// A string whose first character is '.' to indicate this is a field.
		SLONG       dimensions;	// For a PUSHARRAYADDRESS or PUSHARRAYVALUE node, this gives the number of dimensions
		PARSE_Node *lvalue;		// A FOR node has the lvalue by which it is recognised in here.
		CBYTE       character;	// For a CHAR_CONST
	};

	union
	{
		SLONG forcode;		// A unique, non-zero, random code that pairs up FOR parse nodes with corresponding FAKE_ENDFOR nodes.
		SLONG ifcode;		// IF nodes have a unique, non-zero random code. This code is used by the 'pretend' node types in PARSE_traverse()...
		SLONG whilecode;	// Unique, non-zero and random. Matched up while nodes with corresponding FAKE_WHILE_COND nodes.
		SLONG args;			// The number of arguments in a PARSE_NODE_TYPE_CALL node (depth of child1) or the number of initialisers to a MATRIX or VECTOR
	};
	
	struct parse_node *child1;
	struct parse_node *child2;
	struct parse_node *child3;

} PARSE_Node;



//
// Returns an array of PARSE_Nodes that describes the program.
// It also returns a string table that contains all the strings
// and labels used in the program.  Errors/warnings/status are
// returned in the PARSE_output variable.
//

extern PARSE_Node *PARSE_line[];		// NULL value means that line was blank.
extern SLONG       PARSE_line_upto;
extern SLONG       PARSE_string_table_upto;
extern CBYTE       PARSE_string_table[];
extern CBYTE      *PARSE_error[];
extern SLONG       PARSE_error_upto;

void PARSE_do(CBYTE *fname);




//
// Traverses the given parse tree and called the user_function on
// each node of the tree.  It traverses through the tree in a
// cunning order- to help out the CG module. It also inserts
// the pretend nodes...
//

#define PARSE_NODE_TYPE_FAKE_THEN       10000
#define PARSE_NODE_TYPE_FAKE_ELSE       10001
#define PARSE_NODE_TYPE_FAKE_END_ELSE   10002
#define PARSE_NODE_TYPE_FAKE_END_FOR    10003
#define PARSE_NODE_TYPE_FAKE_FOR_COND   10004
#define PARSE_NODE_TYPE_FAKE_WHILE_COND 10005

// That have their 'ifcode' field set to the IF instruction
// that they correspond to.	 The function should return FALSE
// to abort.
//

void PARSE_traverse(PARSE_Node *pn, SLONG (*user_function)(PARSE_Node *pn));




//
// Returns TRUE if the two PARSE trees are the same.
//

SLONG PARSE_trees_the_same(PARSE_Node *tree1, PARSE_Node *tree2);




#endif
