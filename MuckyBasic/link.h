//
// The linker
//

#ifndef _LINK_
#define _LINK_


// ========================================================
//
// STRUCTURES IN THE OBJECT FILE.
//
// ========================================================

typedef struct
{
	UWORD index;	// This is the number of the global.
	UBYTE export;	// TRUE => This global is exported
	UBYTE local;	// TRUE => This global is local to the file.
	SLONG name;		// Index into the debug data

} LINK_Global;

typedef struct
{
	SLONG name;
	SLONG export;		// TRUE => this function is exported.
	SLONG line_start;	// The first and last line of the function body
	SLONG line_end;
	SLONG num_args;		// The number of arguments to the function

} LINK_Function;

typedef struct
{
	SLONG instruction;	// The first instruction generated for this line of code.

} LINK_Line;

typedef struct
{
	SLONG instruction;	// Index into instruction memory (SLONG *..) that contains a jump address.

} LINK_Jump;

typedef struct
{
	SLONG index;		// This is the number of the field.
	SLONG name;			// Index into the debug data.

} LINK_Field;

typedef struct
{
	SLONG instruction;	// Index into instruction memory that contains a global index.

} LINK_Globalref;

typedef struct
{
	SLONG name;			// Index into debug data for the name of the undefined function.
	SLONG instruction;	// Index into instruction memory that should contain the GOSUB jump to the function.

} LINK_Undefref;

typedef struct
{
	SLONG instruction;	// Index into the instruction memory for which field.

} LINK_Fieldref;

typedef struct
{
	SLONG instruction;

} LINK_Datatableref;

typedef struct
{
	SLONG version;
	SLONG num_instructions;		// Each instruction is a SLONG
	SLONG data_table_length_in_bytes;
	SLONG num_globals;
	SLONG num_functions;
	SLONG num_lines;
	SLONG num_jumps;
	SLONG num_fields;
	SLONG num_global_refs;
	SLONG num_undef_refs;
	SLONG num_field_refs;
	SLONG num_data_table_refs;
	
} LINK_Header;


// ========================================================
//
// THE FORMAT OF THE OBJECT FILE.
//
// ========================================================

//
// LINK_Header    lh
//
// SLONG             Instructions           [lh.num_instructions          ]  Followed by MAGIC SLONG(12345678)
// CBYTE             Data table             [lh.data_table_length_in_bytes]	 Followed by MAGIC SLONG(12345678)
// LINK_Global       Globals                [lh.num_globals               ]	 Followed by MAGIC SLONG(12345678)
// LINK_Function     Functions              [lh.num_functions             ]	 Followed by MAGIC SLONG(12345678)
// LINK_Line         Lines                  [lh.num_lines                 ]	 Followed by MAGIC SLONG(12345678)
// LINK_Jump         Jumps                  [lh.num_jumps                 ]	 Followed by MAGIC SLONG(12345678)
// LINK_Field        Fields                 [lh.num_fields                ]	 Followed by MAGIC SLONG(12345678)
// LINK_Globalref    Gobal refs             [lh.num_global_refs           ]	 Followed by MAGIC SLONG(12345678)
// LINK_Undefref     Undefined function refs[lh.num_undefined_funcion_refs]	 Followed by MAGIC SLONG(12345678)
// LINK_Fieldref     Field refs             [lh.num_field_refs            ]	 Followed by MAGIC SLONG(12345678)
// LINK_Datatableref Data table refs        [lh.num_data_table_refs       ]	 Followed by MAGIC SLONG(12345678)
//
// SLONG          debug_data_length_in_bytes
// CBYTE          debug data[debug_data_length_in_bytes]
//




// ========================================================
//
// THE LINKER
//
// ========================================================

//
// Links all the object files into an executable. Returns FALSE on failure.
//

SLONG LINK_do(CBYTE *object_fname[], SLONG num_object_files, CBYTE *exec_fname);




#endif
