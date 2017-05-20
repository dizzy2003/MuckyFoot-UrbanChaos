//
// The language of our virtual machine and the executable
// file format.
//

#ifndef _ML_
#define _ML_

#include "ll.h"


//
// A memory location in the machine is 8-bytes long and contains
// an ML_Data. There is main memory where globals are held and a
// stack where computations are peformed and local variables are stored.
// There is also a string table (CBYTE[]) where string constants are
// stored. When an ML_Data must store a variable that is more than an
// SLONG it just malloc()s the required memory and then stores a pointer
// to it.
//

//
// Each instruction...
//

#define ML_DO_PUSH_CONSTANT	          0		// Followed by an 8-byte constant.
#define ML_DO_ADD			          1		// Adds the last two items on the stack and replaces them with the answer.
#define ML_DO_PRINT			          2		// Pops the last item of the stack and prints it.
#define ML_DO_EXIT			          3		// Exits.
#define ML_DO_GOTO			          4		// Followed by the instruction to go to. In SLONGs from the beginning of the program.
#define ML_DO_UMINUS    	          5		// 'Minuses' the last item on the stack.
#define ML_DO_PUSH_GLOBAL_VALUE       6		// Followed by the number of the global to push.
#define ML_DO_MINUS      	          7		// Does stack[-2] = stack[-2] - stack[-1]; stack -= 1...
#define ML_DO_TIMES      	          8		// Does stack[-2] = stack[-2] * stack[-1]; stack -= 1...
#define ML_DO_DIVIDE     	          9		// Does stack[-2] = stack[-2] / stack[-1]; stack -= 1...
#define ML_DO_MOD        	         10		// Does stack[-2] = stack[-2] % stack[-1]; stack -= 1...
#define ML_DO_IF_FALSE_GOTO	         11		// Pops the stack. If it is FALSE then it does a GOTO (next instruction holds address)
#define ML_DO_AND		 	         12		// etc...!
#define ML_DO_OR         	         13
#define ML_DO_EQUALS     	         14
#define ML_DO_GT         	         15
#define ML_DO_LT		 	         16
#define ML_DO_GTEQ		 	         17
#define ML_DO_LTEQ		 	         18
#define ML_DO_NOT        	         19
#define ML_DO_SQRT       	         20
#define ML_DO_NEWLINE    	         21		// A PRINT without anything produces a NEWLINE instruction...
#define ML_DO_ABS			         22
#define ML_DO_PUSH_GLOBAL_ADDRESS    23		// Pushes an 'ML_TYPE_POINTER' onto the stack containing the address of the given global.
#define ML_DO_ASSIGN                 24		// stack[-2] is a value, stack[-1] is a pointer. Assigns the value to the ML_Data pointed to by stack[-1]
#define ML_DO_PUSH_FIELD_ADDRESS	 25		// The next instruction contains the field_id. This function pops off a pointer to a data and pushes on the address of the given field of that bit of data.
#define ML_DO_PUSH_FIELD_VALUE		 26
#define ML_DO_PUSH_ARRAY_ADDRESS     27		// The next instruction contains the dimensions of the array and the indices will already be on the stack
#define ML_DO_PUSH_ARRAY_VALUE       28
#define ML_DO_PUSH_INPUT             29		// Gets user input as a string and pushes it onto the stack
#define ML_DO_GOSUB                  30
#define ML_DO_RETURN                 31
#define ML_DO_XOR                    32
#define ML_DO_PUSH_GLOBAL_QUICK      33
#define ML_DO_PUSH_FIELD_QUICK       34		// Pushes the actual field onto the stack- not a copy of it.
#define ML_DO_PUSH_ARRAY_QUICK       35		// Pushes the actual array onto the stack- not a copy of it.
#define ML_DO_NOTEQUAL               36
#define ML_DO_IF_TRUE_GOTO           37
#define ML_DO_PUSH_RANDOM_SLUMBER    38		// Pushes a random SLUMBER onto the stack.
#define ML_DO_SWAP                   39		// Swaps the contents of the last two ML_TYPE_POINTERs on the stack.
#define ML_DO_ENTERFUNC              40		// Followed by the number of arguments passed to the function.
#define ML_DO_ENDFUNC                41		// Followed by the number of arguments passed to the function.
#define ML_DO_POP                    42		// Throws away the value at the top of the stack.
#define ML_DO_PUSH_LOCAL_VALUE       43
#define ML_DO_PUSH_LOCAL_ADDRESS     44
#define ML_DO_PUSH_LOCAL_QUICK       45
#define ML_DO_JNEQ_POP_1             46		// For switch statements. Compares two values on the stack. If not equal it pops one value and jumps otherwise it pops both values. The next SLONG hold where to jump to.
#define ML_DO_TEXTURE                47
#define ML_DO_BUFFER                 48
#define ML_DO_DRAW                   49
#define ML_DO_CLS                    50
#define ML_DO_FLIP                   51
#define ML_DO_KEY_VALUE              52
#define ML_DO_KEY_ASSIGN             53
#define ML_DO_INKEY_VALUE            54
#define ML_DO_INKEY_ASSIGN           55
#define ML_DO_TIMER                  56
#define ML_DO_SIN					 57
#define ML_DO_COS					 58
#define ML_DO_TAN					 59
#define ML_DO_ASIN					 60
#define ML_DO_ACOS					 61
#define ML_DO_ATAN					 62
#define ML_DO_ATAN2					 63
#define ML_DO_NOP                    64
#define ML_DO_LEFT                   65
#define ML_DO_MID                    66
#define ML_DO_RIGHT                  67
#define ML_DO_LEN					 68
#define ML_DO_PUSH_IDENTITY_MATRIX   69		// Pushes an identity matrix onto the stack
#define ML_DO_PUSH_ZERO_VECTOR       70		// Pushes a zero vector onto the stack
#define ML_DO_MATRIX				 71		// Constructs a matrix from three vectors on the stack
#define ML_DO_VECTOR                 72		// Constructs a vector from three numbers on the stack
#define ML_DO_PUSH_UP				 73		// Push vector ( 0,+1, 0)
#define ML_DO_PUSH_DOWN				 74		// Push vector ( 0,-1, 0)
#define ML_DO_PUSH_LEFT				 75		// Push vector (-1, 0, 0)
#define ML_DO_PUSH_RIGHT			 76		// Push vector (+1, 0, 0)
#define ML_DO_PUSH_FORWARDS			 77		// Push vector ( 0, 0,+1)
#define ML_DO_PUSH_BACKWARDS		 78		// Push vector ( 0, 0,-1)
#define ML_DO_DOT                    79		// The dot product of two vectors on the stack
#define ML_DO_CROSS                  80		// The cross product of two vectors on the stack

//
// The basic types.
//

#define ML_TYPE_UNDEFINED    0
#define ML_TYPE_SLUMBER		 1
#define ML_TYPE_FLUMBER      2
#define ML_TYPE_STRCONST	 3  // A string constant located in the string data segment.
#define ML_TYPE_STRVAR		 4
#define ML_TYPE_BOOLEAN      5
#define ML_TYPE_POINTER      6  // Pointer to an ML_Data in the 'data' field.
#define ML_TYPE_STRUCTURE    7  // Pointer to an ML_Structure on the heap.
#define ML_TYPE_ARRAY        8  // Pointer to an ML_Array on the heap.
#define ML_TYPE_CODE_POINTER 9  // Instruction address
#define ML_TYPE_STACK_BASE   10 // The previous stack frame base pointer.
#define ML_TYPE_TEXTURE      11
#define ML_TYPE_BUFFER       12
#define ML_TYPE_NUM_ARGS     13 // The number of argument passed in a function call.
#define ML_TYPE_MATRIX       14
#define ML_TYPE_VECTOR       15
#define ML_TYPE_FLOINTER     16 // A pointer to a float inside a vector
#define ML_TYPE_VOINTER      17 // A pointer to a vector

//
// A memory location.
//

typedef struct ml_data      ML_Data;
typedef struct ml_structure ML_Structure;
typedef struct ml_array     ML_Array;
typedef struct ml_vector    ML_Vector;
typedef struct ml_matrix    ML_Matrix;

typedef	struct ml_data
{
	SLONG type;

	union
	{
		SLONG         value;
		SLONG         slumber;
		float         flumber;
		SLONG         strconst;		// Index into the data table.
		SLONG         local;		// Index into the current stackframe.
		CBYTE        *strvar;		// Pointer to some MEM_alloc()ed memory.
		SLONG         boolean;
		ML_Data      *data;			// Pointer to an ML_Data.
		ML_Structure *structure;	// Pointer to an ML_Structure on the heap.
		ML_Array     *array;
		SLONG        *code_pointer;	// Index into instruction memory.
		ML_Data      *stack_base;
		LL_Texture   *lt;
		LL_Buffer    *lb;
		CBYTE         character;
		SLONG         args;
		ML_Vector    *vector;
		ML_Matrix    *matrix;
		float        *flointer;
	};

} ML_Data;

typedef struct
{
	SLONG   field_id;
	ML_Data data;

} ML_Field;

typedef struct ml_structure
{
	SLONG    num_fields;
	ML_Field field[];

} ML_Structure;

typedef struct
{
	SLONG size;
	SLONG stride;	// How many ML_Datas between two array members with consecutive indices in this dimension.

} ML_Dimension;

typedef struct ml_array
{
	SLONG        length;		// Total number of ML_Datas in the data[] array.
	SLONG        num_dimensions;
	ML_Data     *data;			// The actual data on the heap.
	ML_Dimension dimension[];

} ML_Array;

typedef struct ml_vector
{
	float x;
	float y;
	float z;

} ML_Vector;

typedef struct ml_matrix
{
	ML_Vector vector[3];

} ML_Matrix;



//
// This is the header for the executable file.
//

#define ML_VERSION_NUMBER 1

typedef struct
{
	SLONG version;
	SLONG instructions_memory_in_bytes;
	SLONG data_table_length_in_bytes;
	SLONG num_globals;

	//
	// The instructions...
	//

	//
	// The data table...
	//

} ML_Header;





#endif
