//
// A lexical analyser
//

#include "always.h"
#include "lex.h"



//
// The input stream we are analysing.
// The cursor in the stream upto where we have done.
// The current line we are on.
//

CBYTE *LEX_stream_buffer;
CBYTE *LEX_stream_upto;
SLONG  LEX_stream_line;



//
// We can push upto one token onto the stack!
//

SLONG     LEX_stack_valid;
LEX_Token LEX_stack;



//
// The last token we read.
//

SLONG     LEX_top_valid;
LEX_Token LEX_top;


//
// TRUE if the last token found was a NEWLINE
//

SLONG LEX_last_token_newline;



//
// A buffer for returning string constants in.
//

CBYTE LEX_string_buffer[LEX_MAX_STRING_LENGTH + 32];



//
// Eats up the input stream and finds the next token.
//

void LEX_find_next_token(void)
{
	SLONG  i;
	CBYTE *dest;

	//
	// Initailise the answer.
	//

	LEX_top_valid = TRUE;
	LEX_top.line  = LEX_stream_line;

	//
	// Skip whitespace.
	//

	while(isspace(*LEX_stream_upto))
	{
		if (*LEX_stream_upto == '\n')
		{
			//
			// A new line character.
			//

			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_NEWLINE;

			LEX_last_token_newline = TRUE;

			return;
		}

		LEX_stream_upto++;
	}

	//
	// End of the input stream?
	//

	if (*LEX_stream_upto == '\000')
	{
		//
		// Only return an EOF after a NEWLINE
		//

		if (LEX_last_token_newline)
		{
			LEX_top.type = LEX_TOKEN_TYPE_EOF;
		}
		else
		{
			LEX_top.type = LEX_TOKEN_TYPE_NEWLINE;

			LEX_last_token_newline = TRUE;
		}

		return;
	}

	LEX_last_token_newline = FALSE;

	//
	// Arithmetic characters..
	//

	switch(*LEX_stream_upto)
	{
		case '=':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_EQUALS;
			return;

		case '-':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_MINUS;
			return;

		case '+':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_PLUS;
			return;

		case '*':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_TIMES;
			return;

		case '%':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_MOD;
			return;

		case '/':

			if (LEX_stream_upto[1] == '/')
			{
				//
				// This is the C++ comment system- bin the rest of the line.
				//

				while(1)
				{
					if (*LEX_stream_upto == '\n')
					{
						LEX_stream_upto++;
						LEX_top.type = LEX_TOKEN_TYPE_NEWLINE;

						LEX_last_token_newline = TRUE;

						return;
					}
					
					if (*LEX_stream_upto == '\000')
					{
						LEX_top.type = LEX_TOKEN_TYPE_NEWLINE;

						LEX_last_token_newline = TRUE;

						return;
					}

					LEX_stream_upto++;
				}
				
				//
				// Never gets here
				//

				ASSERT(0);
			}

			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_DIVIDE;
			return;

		case ':':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_COLON;
			return;

		case '(':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_OPEN;
			return;

		case ')':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_CLOSE;
			return;
		
		case '[':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_OSQUARE;
			return;

		case ']':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_CSQUARE;
			return;
		
		case ',':
			LEX_stream_upto++;
			LEX_top.type = LEX_TOKEN_TYPE_COMMA;
			return;
		
		case '>':

			if (LEX_stream_upto[1] == '=')
			{
				LEX_stream_upto += 2;
				LEX_top.type     = LEX_TOKEN_TYPE_GTEQ;
			}
			else
			{
				LEX_stream_upto += 1;
				LEX_top.type     = LEX_TOKEN_TYPE_GT;
			}

			return;

		case '<':

			if (LEX_stream_upto[1] == '=')
			{
				LEX_stream_upto += 2;
				LEX_top.type     = LEX_TOKEN_TYPE_LTEQ;
			}
			else
			if (LEX_stream_upto[1] == '>')
			{
				LEX_stream_upto += 2;
				LEX_top.type     = LEX_TOKEN_TYPE_NOTEQUAL;
			}
			else
			{
				LEX_stream_upto += 1;
				LEX_top.type     = LEX_TOKEN_TYPE_LT;
			}

			return;

		case '!':

			if (LEX_stream_upto[1] == '=')
			{
				LEX_stream_upto += 2;
				LEX_top.type     = LEX_TOKEN_TYPE_NOTEQUAL;

				return;
			}

			break;

		case '.':

			if (isdigit(LEX_stream_upto[1]))
			{
				//
				// This dot is part of a number.
				//

				break;
			}
			else
			{
				LEX_stream_upto += 1;
				LEX_top.type     = LEX_TOKEN_TYPE_DOT;

				return;
			}

		case '"':

			LEX_stream_upto += 1;
			dest             = LEX_string_buffer;

			while(1)
			{
				if (!WITHIN(dest, LEX_string_buffer, LEX_string_buffer + LEX_MAX_STRING_LENGTH - 1))
				{
					LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
					LEX_top.error = "String constant is too long";

					return;
				}

				if (*LEX_stream_upto == '"')
				{
					*dest            = '\000';
					LEX_stream_upto += 1;
					LEX_top.type     = LEX_TOKEN_TYPE_STRING;
					LEX_top.string   = LEX_string_buffer;

					return;
				}
				else
				if (*LEX_stream_upto == '\n')
				{
					LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
					LEX_top.error = "Newline in string constant (did you miss out a close quote on a string!)";

					return;
				}
				else
				if (*LEX_stream_upto == '\000')
				{
					LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
					LEX_top.error = "End of file found during string constant (did you miss out a close quote on a string!)";

					return;
				}
				else
				{
					*dest++ = *LEX_stream_upto++;
				}
			}

			//
			// Never gets here...
			//

			ASSERT(0);

		case '\'':
		
			//
			// Character constant?
			//

			LEX_stream_upto += 1;

			if (iscntrl(*LEX_stream_upto))
			{
				LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
				LEX_top.error = "Bad character in character constant";

				return;
			}

			LEX_top.type    =  LEX_TOKEN_TYPE_SLUMBER;
			LEX_top.slumber = *LEX_stream_upto++;

			if (*LEX_stream_upto != '\'')
			{
				LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
				LEX_top.error = "Character constant isn't terminated with an end quote";

				return;
			}

			LEX_stream_upto++;

			break;

		default:
			break;
	}

	//
	// Number constant?
	//

	if (LEX_stream_upto[0] == '0' && (LEX_stream_upto[1] == 'x' || LEX_stream_upto[1] == 'X'))
	{
		//
		// This is a HEX number.
		//

		SLONG number     = 0;
		SLONG num_digits = 0;

		LEX_stream_upto += 2;

		if (!isxdigit(*LEX_stream_upto))
		{
			LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
			LEX_top.error = "Unfinished hexadecimal constant";

			return;
		}

		while(1)
		{
			if (isxdigit(*LEX_stream_upto))
			{
				number <<= 4;
				
				if (isdigit(*LEX_stream_upto))
				{
					number |= *LEX_stream_upto - '0';
				}
				else
				{
					if (isupper(*LEX_stream_upto))
					{
						number |= *LEX_stream_upto - 'A' + 10;
					}
					else
					{
						number |= *LEX_stream_upto - 'a' + 10;
					}
				}

				num_digits      += 1;
				LEX_stream_upto += 1;
			}
			else
			{
				break;
			}
		}

		if (num_digits > 8)
		{
			LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
			LEX_top.error = "Too many digits in hexadecimal constant";

			return;
		}

		LEX_top.type    = LEX_TOKEN_TYPE_SLUMBER;
		LEX_top.slumber = number;

		return;
	}
	else
	if (isdigit(*LEX_stream_upto) || *LEX_stream_upto == '.')
	{
		SLONG doing_fraction = FALSE;

		double number = 0.0F;
		double frac   = 0.1F;

		while(1)
		{
			if (isdigit(*LEX_stream_upto))
			{
				if (doing_fraction)
				{
					//
					// We are doing the fractional part of a floating point number.
					//

					number += float(*LEX_stream_upto - '0') * frac;
					frac   *= 0.1F;
				}
				else
				{
					//
					// We are doing the integer part.
					//

					number *= 10.0F;
					number += float(*LEX_stream_upto - '0');
				}
			}
			else
			if (*LEX_stream_upto == '.')
			{
				//
				// A floating point number.
				//

				if (doing_fraction)
				{
					//
					// We've already come across one decimal point!
					//

					LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
					LEX_top.error = "Found two decimal points in a floating point number!";

					return;
				}
				else
				{
					doing_fraction = TRUE;
				}
			}
			else
			{
				if (doing_fraction)
				{
					LEX_top.type    = LEX_TOKEN_TYPE_FLUMBER;
					LEX_top.flumber = (float) number;
				}
				else
				{
					LEX_top.type   = LEX_TOKEN_TYPE_SLUMBER;
					LEX_top.slumber = (SLONG) number;
				}

				return;
			}

			LEX_stream_upto += 1;
		}
	}

	//
	// A word of some sort?
	//

	if (isalpha(*LEX_stream_upto))
	{
		//
		// Copy the variable into the string buffer.
		//

		dest = LEX_string_buffer;

		while(1)
		{
			if (!WITHIN(dest, LEX_string_buffer, LEX_string_buffer + LEX_MAX_STRING_LENGTH - 1))
			{
				LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
				LEX_top.error = "Variable name or label is too long";

				return;
			}

			if (isalnum(*LEX_stream_upto) || *LEX_stream_upto == '_')
			{
				*dest++ = *LEX_stream_upto++;
			}
			else
			{
				*dest = '\000';

				break;
			}
		}

		//
		// Have we found a keyword?
		//

		struct
		{
			CBYTE *keyword;
			SLONG  token;

		} keyword[] =
		{
			{"IF",        LEX_TOKEN_TYPE_IF       },
			{"THEN",      LEX_TOKEN_TYPE_THEN     },
			{"GOTO",      LEX_TOKEN_TYPE_GOTO     },
			{"AND",	      LEX_TOKEN_TYPE_AND      },
			{"OR",	      LEX_TOKEN_TYPE_OR       },
			{"NOT",	      LEX_TOKEN_TYPE_NOT      },
			{"REM",	      NULL                    },	// This is a special case, LEX removes the rest of the line
			{"CALL",      LEX_TOKEN_TYPE_CALL     },
			{"FUNC",      LEX_TOKEN_TYPE_FUNC     },
			{"LOCAL",     LEX_TOKEN_TYPE_LOCAL    },
			{"PRINT",     LEX_TOKEN_TYPE_PRINT    },
			{"ELSE",      LEX_TOKEN_TYPE_ELSE     },
			{"TRUE",      LEX_TOKEN_TYPE_TRUE     },
			{"FALSE",     LEX_TOKEN_TYPE_FALSE    },
			{"SQRT",      LEX_TOKEN_TYPE_SQRT     },
			{"ABS",       LEX_TOKEN_TYPE_ABS      },
			{"INPUT",     LEX_TOKEN_TYPE_INPUT    },
			{"UNDEFINED", LEX_TOKEN_TYPE_UNDEFINED},
			{"EXIT",      LEX_TOKEN_TYPE_EXIT     },
			{"GOSUB",     LEX_TOKEN_TYPE_GOSUB    },
			{"RETURN",    LEX_TOKEN_TYPE_RETURN   },
			{"XOR",       LEX_TOKEN_TYPE_XOR      },
			{"FOR",       LEX_TOKEN_TYPE_FOR      },
			{"TO",        LEX_TOKEN_TYPE_TO       },
			{"STEP",      LEX_TOKEN_TYPE_STEP     },
			{"NEXT",      LEX_TOKEN_TYPE_NEXT     },
			{"RANDOM",    LEX_TOKEN_TYPE_RANDOM   },
			{"SWAP",      LEX_TOKEN_TYPE_SWAP     },
			{"MOD",       LEX_TOKEN_TYPE_MOD      },
			{"ENDIF",     LEX_TOKEN_TYPE_ENDIF    },
			{"WHILE",     LEX_TOKEN_TYPE_WHILE    },
			{"LOOP",      LEX_TOKEN_TYPE_LOOP     },
			{"FUNCTION",  LEX_TOKEN_TYPE_FUNCTION },
			{"ENDFUNC",   LEX_TOKEN_TYPE_ENDFUNC  },
			{"TEXTURE",   LEX_TOKEN_TYPE_TEXTURE  },
			{"BUFFER",    LEX_TOKEN_TYPE_BUFFER   },
			{"DRAW",      LEX_TOKEN_TYPE_DRAW     },
			{"CLS",       LEX_TOKEN_TYPE_CLS      },
			{"FLIP",      LEX_TOKEN_TYPE_FLIP     },
			{"KEY",       LEX_TOKEN_TYPE_KEY      },
			{"INKEY",     LEX_TOKEN_TYPE_INKEY    },
			{"TIMER",     LEX_TOKEN_TYPE_TIMER    },
			{"SIN",       LEX_TOKEN_TYPE_SIN      },
			{"COS",       LEX_TOKEN_TYPE_COS      },
			{"TAN",       LEX_TOKEN_TYPE_TAN      },
			{"ASIN",      LEX_TOKEN_TYPE_ASIN     },
			{"ACOS",      LEX_TOKEN_TYPE_ACOS     },
			{"ATAN",      LEX_TOKEN_TYPE_ATAN     },
			{"ATAN2",     LEX_TOKEN_TYPE_ATAN2    },
			{"EXPORT",    LEX_TOKEN_TYPE_EXPORT   },
			{"LEFT",      LEX_TOKEN_TYPE_LEFT     },
			{"MID",       LEX_TOKEN_TYPE_MID      },
			{"RIGHT",     LEX_TOKEN_TYPE_RIGHT    },
			{"LEN",       LEX_TOKEN_TYPE_LEN      },
			{"!"}
		};

		for (i = 0; keyword[i].keyword[0] != '!'; i++)
		{
			if (strcmp(keyword[i].keyword, LEX_string_buffer) == 0)
			{
				if (strcmp(keyword[i].keyword, "REM") == 0)
				{
					//
					// This is a REM statement. Skip to the end of the line
					// and return a NEWLINE.
					//

					while(1)
					{
						if (*LEX_stream_upto == '\n')
						{
							LEX_stream_upto++;
							LEX_top.type = LEX_TOKEN_TYPE_NEWLINE;

							LEX_last_token_newline = TRUE;

							return;
						}
						
						if (*LEX_stream_upto == '\000')
						{
							LEX_top.type = LEX_TOKEN_TYPE_NEWLINE;

							LEX_last_token_newline = TRUE;

							return;
						}

						LEX_stream_upto++;
					}
					
					//
					// Never gets here
					//

					ASSERT(0);
				}

				if (LEX_last_token_newline)
				{
					//
					// If the next character is a ':', then it's an error because
					// you can't have a keyword as a label.
					//

					if (*LEX_stream_upto == ':')
					{
						LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
						LEX_top.error = "You can't have a keyword as a label";

						return;
					}
				}

				LEX_top.type = keyword[i].token;

				return;
			}
		}

		//
		// Is this a label? Is the next character a ':'?
		// 

		if (*LEX_stream_upto == ':')
		{
			if (LEX_last_token_newline)
			{
				//
				// Labels only at the beginning of a line.
				//

				strcat(LEX_string_buffer, ":");

				LEX_stream_upto += 1;
				LEX_top.type     = LEX_TOKEN_TYPE_LABEL;
				LEX_top.label    = LEX_string_buffer;
	
				return;
			}
			else
			{
				//
				// Otherwise it's a variable followed by a COLON separator.
				// 
			}
		}

		//
		// Must be a variable.
		//

		LEX_top.type     = LEX_TOKEN_TYPE_VARIABLE;
		LEX_top.variable = LEX_string_buffer;

		return;
	}

	//
	// Strange character.
	//

	sprintf(LEX_string_buffer, "Found a strange character: '%c'", *LEX_stream_upto);

	LEX_top.type  = LEX_TOKEN_TYPE_ERROR;
	LEX_top.error = LEX_string_buffer;

	return;
}




void LEX_start(CBYTE *string)
{
	LEX_stream_buffer = string;
	LEX_stream_upto   = string;

	LEX_top_valid          = FALSE;
	LEX_last_token_newline = FALSE;
}

LEX_Token LEX_get()
{
	if (!LEX_top_valid)
	{
		LEX_find_next_token();
	}

	return LEX_top;
}

LEX_Token LEX_pop()
{
	LEX_Token ans;

	if (!LEX_top_valid)
	{
		LEX_find_next_token();
	}

	LEX_top_valid = FALSE;
	ans           = LEX_top;

	if (LEX_stack_valid)
	{	
		LEX_stack_valid = FALSE;
		LEX_top_valid   = TRUE;
		LEX_top         = LEX_stack;
	}

	return ans;
}

void LEX_push(LEX_Token lt)
{
	if (LEX_top_valid)
	{
		ASSERT(!LEX_stack_valid);

		LEX_stack_valid = TRUE;
		LEX_stack       = LEX_top;
		LEX_top         = lt;
	}
	else
	{
		LEX_top_valid = TRUE;
		LEX_top       = lt;
	}
}


void LEX_next_line()
{
	LEX_Token lt;

	while(1)
	{
		lt = LEX_pop();

		if (lt.type == LEX_TOKEN_TYPE_NEWLINE)
		{
			return;
		}

		if (lt.type == LEX_TOKEN_TYPE_EOF)
		{
			//
			// Push the token back on the stack...
			//

			LEX_top_valid = TRUE;

			return;
		}	
	}
}
