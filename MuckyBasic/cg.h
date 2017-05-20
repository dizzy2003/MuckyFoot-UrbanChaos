//
// The code generator and linker. Converts the output of the parse into
// an executable.
//

#ifndef _CG_
#define _CG_



//
// Takes the output of the PARSE module and spews out
// code to the given filename.  Returns FALSE on failure.
// CG_output[] hold the errors/warnings/status of the
// code generation phase.
//

extern CBYTE *CG_output;
extern SLONG  CG_num_errors;
extern SLONG  CG_num_warnings;

#define CG_OUTPUT_EXECUTABLE  (1 << 0)
#define CG_OUTPUT_OBJECT_FILE (1 << 1)
#define CG_OUTPUT_DEBUG_INFO  (1 << 2)

SLONG CG_do(CBYTE *fname, SLONG output = CG_OUTPUT_EXECUTABLE);





#endif
