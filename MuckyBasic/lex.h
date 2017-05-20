//
// A lexical analyser
//

#ifndef _LEX_
#define _LEX_


#define	LEX_TOKEN_TYPE_EQUALS	   0
#define	LEX_TOKEN_TYPE_PLUS		   1
#define	LEX_TOKEN_TYPE_MINUS	   2
#define	LEX_TOKEN_TYPE_TIMES	   3
#define	LEX_TOKEN_TYPE_DIVIDE	   4
#define	LEX_TOKEN_TYPE_SLUMBER	   5
#define	LEX_TOKEN_TYPE_FLUMBER	   6
#define	LEX_TOKEN_TYPE_STRING	   7
#define	LEX_TOKEN_TYPE_VARIABLE	   8
#define	LEX_TOKEN_TYPE_IF		   9
#define	LEX_TOKEN_TYPE_THEN		  10
#define	LEX_TOKEN_TYPE_GOTO		  11
#define	LEX_TOKEN_TYPE_LABEL	  12
#define	LEX_TOKEN_TYPE_ERROR	  13
#define	LEX_TOKEN_TYPE_NEWLINE	  14
#define	LEX_TOKEN_TYPE_EOF		  15
#define	LEX_TOKEN_TYPE_GT		  16
#define	LEX_TOKEN_TYPE_LT		  17
#define	LEX_TOKEN_TYPE_GTEQ		  18
#define	LEX_TOKEN_TYPE_LTEQ		  19
#define	LEX_TOKEN_TYPE_AND		  20
#define	LEX_TOKEN_TYPE_OR		  21
#define	LEX_TOKEN_TYPE_NOT		  22
#define	LEX_TOKEN_TYPE_DOT		  23
#define	LEX_TOKEN_TYPE_OPEN		  24
#define	LEX_TOKEN_TYPE_CLOSE	  25
#define	LEX_TOKEN_TYPE_CALL		  26
#define	LEX_TOKEN_TYPE_FUNC		  27
#define	LEX_TOKEN_TYPE_LOCAL	  28
#define	LEX_TOKEN_TYPE_OSQUARE	  29	// Open square bracket [
#define	LEX_TOKEN_TYPE_CSQUARE	  30	// Close square bracket ]
#define	LEX_TOKEN_TYPE_PRINT	  31
#define LEX_TOKEN_TYPE_MOD        32
#define LEX_TOKEN_TYPE_ELSE       33
#define LEX_TOKEN_TYPE_TRUE       34
#define LEX_TOKEN_TYPE_FALSE      35
#define LEX_TOKEN_TYPE_SQRT       36
#define LEX_TOKEN_TYPE_ABS        37
#define LEX_TOKEN_TYPE_COMMA      38
#define LEX_TOKEN_TYPE_INPUT      39
#define LEX_TOKEN_TYPE_UNDEFINED  40	// The actual word "UNDEFINED"
#define LEX_TOKEN_TYPE_COLON      41
#define LEX_TOKEN_TYPE_EXIT       42
#define LEX_TOKEN_TYPE_GOSUB      43
#define LEX_TOKEN_TYPE_RETURN     44
#define LEX_TOKEN_TYPE_XOR        45
#define LEX_TOKEN_TYPE_FOR        46
#define LEX_TOKEN_TYPE_TO		  47
#define LEX_TOKEN_TYPE_STEP		  48
#define LEX_TOKEN_TYPE_NEXT       49
#define LEX_TOKEN_TYPE_NOTEQUAL   50
#define LEX_TOKEN_TYPE_RANDOM     51
#define LEX_TOKEN_TYPE_SWAP       52
#define LEX_TOKEN_TYPE_ENDIF      53
#define LEX_TOKEN_TYPE_WHILE      54
#define LEX_TOKEN_TYPE_LOOP       55
#define LEX_TOKEN_TYPE_FUNCTION   56
#define LEX_TOKEN_TYPE_ENDFUNC    57
#define LEX_TOKEN_TYPE_TEXTURE    58
#define LEX_TOKEN_TYPE_BUFFER     59
#define LEX_TOKEN_TYPE_DRAW       60
#define LEX_TOKEN_TYPE_CLS        61
#define LEX_TOKEN_TYPE_FLIP       62
#define LEX_TOKEN_TYPE_KEY        63
#define LEX_TOKEN_TYPE_INKEY      64
#define LEX_TOKEN_TYPE_TIMER      65
#define LEX_TOKEN_TYPE_SIN        66
#define LEX_TOKEN_TYPE_COS        67
#define LEX_TOKEN_TYPE_TAN        68
#define LEX_TOKEN_TYPE_ASIN       69
#define LEX_TOKEN_TYPE_ACOS       70
#define LEX_TOKEN_TYPE_ATAN       71
#define LEX_TOKEN_TYPE_ATAN2      72
#define LEX_TOKEN_TYPE_EXPORT     73
#define LEX_TOKEN_TYPE_LEFT       74
#define LEX_TOKEN_TYPE_MID        75
#define LEX_TOKEN_TYPE_RIGHT      76
#define LEX_TOKEN_TYPE_LEN        77
#define LEX_TOKEN_TYPE_MATRIX     78
#define LEX_TOKEN_TYPE_VECTOR     79
#define LEX_TOKEN_TYPE_DPROD      80
#define LEX_TOKEN_TYPE_CPROD      81
#define LEX_TOKEN_TYPE_NORMALISE  82
#define LEX_TOKEN_TYPE_TRANSPOSE  83



typedef struct
{
	SLONG type;
	SLONG line;

	union
	{
		SLONG  slumber;
		float  flumber;
		CBYTE *string;
		CBYTE *variable;
		CBYTE *label;
		CBYTE *error;
	};

} LEX_Token;


//
// The maximum length of a string constant, variable name or label.
//

#define LEX_MAX_STRING_LENGTH 1024




//
// Starts reading token from the given string.
//

void LEX_start(CBYTE *string);


//
// Returns the next token in the input stream without
// taking it out of the stream.
//

LEX_Token LEX_get(void);

//
// Pops the next token from the input stream.
//

LEX_Token LEX_pop(void);


//
// Pushes the given token back onto the input stream.
//

void LEX_push(LEX_Token lt);


//
// Skips to the next line and eats up all the input. Helpful
// for error recovery.
//

void LEX_next_line(void);




#endif
