//
// A symbol table.
//

#include "always.h"
#include "st.h"




//
// A symbol.
//

typedef struct
{
	ULONG hash;
	SLONG value;
	SLONG flag;
	UWORD string;	// Offset into the buffer
	UWORD next;
 
} ST_Symbol;


//
// A symbol table.
//

typedef struct
{
	//
	// Where we store strings.
	//

	CBYTE *buffer;
	SLONG  buffer_upto;
	SLONG  buffer_max;

	//
	// Where we store symbols.
	//

	ST_Symbol *symbol;
	SLONG      symbol_upto;
	SLONG      symbol_max;

	//
	// A hash table. Each entry is the start of a linked list.
	//

	#define ST_HASH_SIZE 512

	UWORD hash[ST_HASH_SIZE];

} ST_Table;

ST_Table ST_table[ST_TABLE_NUMBER];



//
// Computes the hash value of a string.
//

ULONG ST_hash_value(CBYTE *string)
{
	ULONG ans = 0;
	SLONG rot = 0;

	CBYTE *ch;

	for (ch = string; *ch; ch++)
	{
		ans ^= _lrotl(*ch, rot);

		rot += 7;
	}

	ans ^= ans >> 16;	// So more letters count...

	return ans;
}

void ST_init()
{
	SLONG i;

	ST_Table *st;

	//
	// Clear all data.
	//

	memset(ST_table, 0, sizeof(ST_table));

	//
	// Initialise each table.
	//

	for (i = 0; i < ST_TABLE_NUMBER; i++)
	{
		st = &ST_table[i];

		st->buffer_max  = 8192;
		st->buffer_upto = 0;
		st->buffer      = (CBYTE *) malloc(sizeof(CBYTE) * st->buffer_max);

		st->symbol_max  = 1024;
		st->symbol_upto = 1;	// The 0th element is the NULL index...
		st->symbol      = (ST_Symbol *) malloc(sizeof(ST_Symbol) * st->symbol_max);
	}
}



void ST_add(SLONG table, CBYTE *string, SLONG value, SLONG flag)
{
	ST_Table  *st;
	ST_Symbol *ss;
	
	SLONG symbol;

	ASSERT(WITHIN(table, 0, ST_TABLE_NUMBER - 1));

	st = &ST_table[table];

	//
	// Compute the hash value.
	//

	ULONG hash = ST_hash_value(string);

	#ifdef _DEBUG

	//
	// Do we already have this symbol?
	//

	#endif

	//
	// Get a new symbol. Is there enough room?
	//

	if (st->symbol_upto >= st->symbol_max)
	{
		st->symbol_max *= 2;
		st->symbol      = (ST_Symbol *) realloc(st->symbol, sizeof(ST_Symbol) * st->symbol_max);
	}

	//
	// The index of our new symbol.
	//

	symbol = st->symbol_upto++;

	//
	// Initialise the symbol.
	//

	ss = &st->symbol[symbol];

	ss->hash   = hash;
	ss->value  = value;
	ss->flag   = flag;
	ss->string = (UWORD) st->buffer_upto;
	ss->next   = NULL;

	//
	// Enough room to add the string?
	//

	SLONG length = strlen(string) + 1;	// + 1 to include the terminating NULL.

	if (st->buffer_upto + length > st->buffer_max)
	{
		//
		// Must allocate a bigger string buffer.
		//

		st->buffer_max *= 2;
		st->buffer      = (CBYTE *) realloc(st->buffer, sizeof(CBYTE) * st->buffer_max);
	}

	//
	// Add the string to the buffer.
	//

	strcpy(st->buffer + st->buffer_upto, string);

	st->buffer_upto += length;

	//
	// Insert the symbol into the hash table.
	//

	ss->next                                = st->hash[ss->hash & (ST_HASH_SIZE - 1)];
	st->hash[ss->hash & (ST_HASH_SIZE - 1)] = (UWORD) symbol;
}



//
// Looks for the string in the given table. Returns TRUE if it
// finds one. Sets the ST_found_table and ST_found_value variables
// if it finds the string.
//

SLONG      ST_found_table;
SLONG      ST_found_value;
SLONG      ST_found_flag;
CBYTE     *ST_found_string;
ST_Symbol *ST_found_ss;

SLONG ST_find_in_table(SLONG table, CBYTE *string)
{
	ST_Table  *st;
	ST_Symbol *ss;
	
	SLONG symbol;

	ASSERT(WITHIN(table, 0, ST_TABLE_NUMBER - 1));

	st = &ST_table[table];

	//
	// Compute the hash value.
	//

	ULONG hash = ST_hash_value(string);

	for (symbol = st->hash[hash & (ST_HASH_SIZE - 1)]; symbol; symbol = ss->next)
	{
		ASSERT(WITHIN(symbol, 1, st->symbol_upto - 1));

		ss = &st->symbol[symbol];

		if (ss->hash == hash)
		{
			//
			// This is probably our symbol. We have to check though.
			//

			if (strcmp(st->buffer + ss->string, string) == 0)
			{
				//
				// This is our symbol.
				//

				ST_found_table  = table;
				ST_found_value  = ss->value;
				ST_found_flag   = ss->flag;
				ST_found_ss     = ss;
				ST_found_string = string;

				return TRUE;
			}
		}
	}

	return FALSE;
}




SLONG ST_find(CBYTE *string)
{
	SLONG table;

	//
	// Look in the tables.
	//

	for (table = ST_TABLE_NUMBER - 1; table >= 0; table--)
	{
		if (ST_find_in_table(
				table,
				string))
		{
			return TRUE;
		}
	}

	return FALSE;
}


void ST_update_flag(CBYTE *string, SLONG new_flag)
{
	SLONG table;

	//
	// Look in the tables.
	//

	for (table = ST_TABLE_NUMBER - 1; table >= 0; table--)
	{
		if (ST_find_in_table(
				table,
				string))
		{
			ST_found_ss->flag = new_flag;

			return;
		}
	}

	//
	// Symbol not found!
	//

	ASSERT(0);

	return;
}



SLONG ST_find_all_table;
SLONG ST_find_all_symbol;


void ST_find_all_start()
{
	ST_find_all_table  = 0;
	ST_find_all_symbol = 1;
}

SLONG ST_find_all_next()
{
	ST_Table  *st;
	ST_Symbol *ss;

	while(1)
	{
		ASSERT(WITHIN(ST_find_all_table, 0, ST_TABLE_NUMBER - 1));

		st = &ST_table[ST_find_all_table];

		if (ST_find_all_symbol >= st->symbol_upto)
		{
			ST_find_all_table  += 1;
			ST_find_all_symbol  = 1;

			if (ST_find_all_table >= ST_TABLE_NUMBER)
			{
				return FALSE;
			}
		}
		else
		{
			ASSERT(WITHIN(ST_find_all_symbol, 0, st->symbol_upto - 1));

			ss = &st->symbol[ST_find_all_symbol++];

			ASSERT(WITHIN(ss->string, 0, st->buffer_upto - 2));

			ST_found_string = &st->buffer[ss->string];
			ST_found_table  =  ST_find_all_table;
			ST_found_value  =  ss->value;
			ST_found_flag   =  ss->flag;

			return TRUE;
		}
	}
}



void ST_clear(SLONG table)
{
	ST_Table  *st;

	ASSERT(WITHIN(table, 0, ST_TABLE_NUMBER - 1));

	st = &ST_table[table];

	//
	// Zero length the arrays and clear the hash table.
	//

	st->symbol_upto = 1;	// 1 because the 0'th index is reserved for NULL.
	st->buffer_upto = 0;

	memset(st->hash, 0, sizeof(st->hash));
}


void ST_clear_all()
{
	SLONG i;

	for (i = 0; i < ST_TABLE_NUMBER; i++)
	{
		ST_clear(i);
	}
}

