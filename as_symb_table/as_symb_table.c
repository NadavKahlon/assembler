/* as_symb_table.h - 'Assembler Symbol Table' - functions implementation
@ written by Nadav Kahlon, May 2020.

This file holds the definition of the different functions declared in as_symb_table.h, which offer an interface to manage symbol tables in an assembler program.

About the implementation:
The symbol table is implemented as an "hash table" - an array of pointers. Each one of them points at the first node in a dynamic linked list of symbol - or NULL if this list is empty. We can match a unique index in this array to every possible symbol, based on its name (in other words - we can match an index in the symbol table array to every possible string of characters). This index is called its "hash value". Every symbol that is going to be installed into the table, will be installed into the list which is indexed in the array by the symbol's hash value.
That way, if we are looking for a symbol  named "abcd" in the table, for example, we can just calculate the hash value for "abcd" and look for such a symbol in the corresponding linked list in the table's array.
The size of an hash table array is HASH_SIZE - a constant which is defined in as_symbols.h. Therefore, hash values are ranged between 0 and HASH_SIZE-1

The functions defined here are:
- intlz_symbt - to initialize a symbol table to a new empty table.
** symb_hash - a static function to form an hash value for a given string of characters.
- symb_lookup - to look for a symbol in a symbol table.
- symb_inst - to install a symbol into a symbol table.
- clear_symb_table - to clear a symbol table.
- inc_data - to increase the address of "data" symbols in a symbol table by a given value.
- form_ent_file - to creat a ".ent" file including information about every "entry" specified symbol in a symbol table.

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../indicators/indicators.h"
#include "../as_core_design/as_core_design.h"
#include "../as_mem_words/as_mem_words.h"
#include "as_symb_table.h"
#include "../assembler_io/assembler_io.h"

/* intlz_symbt - sets the symbol table entered as the parameter 'table' to a new and empty table.
We scan the table array, and initialized each list in it to an empty list by setting the first node of the list to NULL. */
void intlz_symbt(SymbolTable table)
{
	int i;
	for(i = 0; i < SYMBT_HSIZE; i++) /* we scan the array and set the lists in it to be empty lists */
		table[i] = NULL;
}

/* symb_hash - a static function to form an hash value for the character string 'str'.
The hashing function adds the numeric value of each character in the string to a scrambled combination of the previous ones and returns the remainder modulo the hash table array size (to get a proper index in the hash table array). */
static unsigned int symb_hash(const char *str)
{
	unsigned hashval; /* a variable to hold the resulting hash value */
	
	for(hashval = 0; *str != '\0'; str++) /* scans the character string until reaching its end */
		hashval = *str  + 31 * hashval; /* and adds each character into a scrambled combination of the previous ones */
	return hashval % SYMBT_HSIZE;
}

/* symb_lookup - looks for a symbol named 'str' in 'table' and returns a poiner to it, or NULL if the symbol was not found.
We check the hash value of the character string and scan the linked list that is indexed at that hash value in the symbol table array, looking for a symbol with 'str' as its name. */
Symbol *symb_lookup(SymbolTable table, const char *str)
{
	struct symb_lnode *curr; /* a pointer to the current node in the linked list that we are scanning */
	
	/* we scan the list that corresponds to the hash value of 'str' */
	for(curr = table[symb_hash(str)]; curr != NULL; curr = curr->next)
	{
		if(strcmp(str, (curr->symb).name) == 0) /* in case we found a symbol with 'str' as its name in the list */
			return &(curr->symb); /* we return a pointer to this symbol in the list */
	}
	return NULL; /* if we reached here, it means that such symbol was not found in the list, and therefore it is not in the table. */
}

/* symb_inst - installs a symbol in the symbol-table 'table'. The symbol's name is represented by 'name' (we assume that it's not NULL), its replacement word is 'mem_word', and the rest of the parameters are the 3 flag values of the symbol (they are EXPECTED to be 1 bit long). Allocates new memory space for it and for the copy of the name using the standard library function malloc.
Returns a status value: ALLOC_ERR if an error occurred while trying to allocate space, DUP_ERR if a symbol with this name already exist in 'table', or ALL_GOOD if the process went with no errors (this values are defined in ../indicators/indicators.h). In case of an error nothing happens to the table.
Note: this function does not check the validitiy of the symbol name. You have to enter a valid symbol information. */
short int symb_inst(SymbolTable table, const char *name, Word rep_word, short int is_extern, short int is_entry, short int is_data)
{
	struct symb_lnode *new_node; /* a pointer to the node that we want to add to the list */
	unsigned int hash_val; /* to store the hash value of the new node */
	
	if(symb_lookup(table, name) != NULL) /* in such case a symbol with this name already exists in the table */
		return DUP_ERR;
	
	/* allocates space for the new node and its name and check the status of the allocation process: */
	if((new_node = malloc(sizeof(struct symb_lnode))) == NULL || ((new_node->symb).name = malloc(strlen(name)+1)) == NULL)
		return ALLOC_ERR; /* in such case malloc couldn't allocate new space */
	/* note that the space we allocated for the name is strlen(name) bytes long + space for a a finishing '\0' character - to fit a copy of 'name' */
	
	/* we copy the fields of the new symbol: */
	strcpy((new_node->symb).name, name);
	(new_node->symb).rep_word = rep_word;
	(new_node->symb).is_extern = is_extern;
	(new_node->symb).is_entry = is_entry;
	(new_node->symb).is_data = is_data;
	
	/* and add it to the beginning of the list corresponding to its hash value */
	hash_val = symb_hash((new_node->symb).name);
	new_node->next = table[hash_val];
	table[hash_val] = new_node;
	return ALL_GOOD;
}


/* clear_symb_table - frees any allocated space that the symbol table 'table' is holding - "clearing" every memory space that it takes. It also initializes the table back into an empty form - so no illegal pointers will be left.
Assumes that the table was managed ONLY using the funtions in this interface. */
void clear_symb_table(SymbolTable table)
{
	int i; /* the index in the current list in the table */
	struct symb_lnode *curr, *next_node; /* 'curr' - a pointer to the current node in a symbol list, 'next_node' - a pointer to the next node in a symbol list */
	
	for(i = 0; i < SYMBT_HSIZE; i++) /* we scan each list in the table */
	{
		curr = table[i]; /* we store a pointer to the first node in 'curr', beforw scanning the list */
		table[i] = NULL; /* we set table[i] to NULL to indicate that the current list will be wmpty */
		
		while(curr != NULL) /* we scan the current list and free each node in it separately, until reaching the end of it (a NULL pointer) */
		{
			next_node = curr->next; /* before clearing the current node, we store a pointer to the next one in 'next_node' */
			
			free((curr->symb).name); /* we free the space we allocated for the symbol name */
			free(curr); /* we free the space we allocated for the whole symb_lnode structure of the node */
			
			curr = next_node; /* and now 'curr' will point the next node that we stored earlier in 'next_node' */
		}
		
	} 
}


/* inc_data_symbs - increase the address that is represented in the replacement memory word of each symbol in the symbol-table 'table' that represents an adress in the data image (its 'is_data' field is TRUE) by 'inc_val'. */
/*** Algorithm explanation: we scan each linked list in the table (in each possible index), and if we encounter a 'data' symbol, we increase its address (which is represented in its memory word in bits 3-24) by inc_val. */
void inc_data(SymbolTable table, unsigned long int inc_val)
{
	int i; /* the index of the current list in the table */
	struct symb_lnode *curr; /* the current node in the list we are currently scanning */
	
	for(i = 0; i < SYMBT_HSIZE; i++) /* we scan each list in the table */
	{
		for(curr = table[i]; curr != NULL; curr = curr->next) /* we check each each node in the current list */
		{
			if((curr->symb).is_data == TRUE) /* in case the current node represents an adress in the data imgae */
			{
				set_word_field(NON_ARE, (curr->symb).rep_word, get_symb_adss(curr->symb) + inc_val, 3); /* we take the address that the symbol represents (using the 'get_symb_adss' macro defined in ../as_core_design/as_core_design.h) and increase it by 'inc_val', then we set this new value to the non-ARE field of the replacement word (the address field) using the set_word_field macro defined in ../as_mem_words/as_mem_words.h */
			}
		}
	}
}



/* form_ent_file - in case ths symbol table 'table' includes symbols with the "entry" specification, this function will creat a ".ent" file which will hold the required information about each "entry" symbol. The file name (including a ".ent" suffix) is specified by 'fname'. Assumes every pointer is not NULL
In case an error occured while trying to open / creat / close the file, the program shuts down - returning FILE_OERATION_ERR, after printing a proper error message (using 'shut_down_err').
In case an error occured while writing to the file the program shuts down - returning PNT_ERR, after printing a proper error message (using 'shut_down_err').
(* These 2 constants are defined in ../indicators/indicators.h.  * 'shut_doen_err' is declared in ../assembler_io/assembler_io.h). */

/*** Algorithm explanation: we scan each linked list in the table's array - when finding the first "entry" symbol we creat the file. We keep scanning the table while printing the entry symbols in it. The address of each symbol is the non-ARE field of its replacement word. */
void form_ent_file(char *fname, SymbolTable table)
{
	FILE *out_stream = NULL; /* a FILE pointer to the output file (initialized to NULL in order to indicate that it was not opened yet) */
	unsigned int curr_list_i; /* the index of the current list that we scan */
	struct symb_lnode *curr_node; /* a pointer to the current node in the current list that we scan */
	Address symb_adss; /* to hold the address that each entry symbol represents */
	char adss_str[ADSS_DEC_LEN+1]; /* a character string to hold the address of each symbol in decimal form */
	short int first = TRUE; /* a flag value - FALSE if an entry symbol was already printed and TRUE if the next entry symbol is the first one found */
	
	for(curr_list_i = 0; curr_list_i < SYMBT_HSIZE; curr_list_i++) /* scans through the table's linked lists */
	{
		for(curr_node = table[curr_list_i]; curr_node != NULL; curr_node = curr_node->next) /* scans through the current linked lists */
		{
			if((curr_node->symb).is_entry == TRUE) /* in case we encountered an entry symbol */
			{
				/* we take the non-ARE field of the symbol - which is the address that it represents (we do it shifting the replacement word of the symbol in unsigned form - to prevent sign extension) */
				symb_adss = (Address)(((unsigned long int)((curr_node->symb).rep_word)) >> 3);
				adss_to_str(symb_adss, adss_str); /* and convert it to a character string format */
				
				if(first) /* in case that is the first symbol we print */
				{
					if((out_stream = fopen(fname, "w")) == NULL) /* we create the ".ent" file and check the process */
					{
						shut_down_err(FILE_OERATION_ERR, "An error occured while trying to open/create a file named: \"%s\"", fname); /* in case an error occured while trying to open the file - we shut down the program using 'shut_down_err' */
					}
					if(fprintf(out_stream, "%s %s", (curr_node->symb).name, adss_str) < 0) /* then we print the symbol and it address, and check the status of the process: */
					{
						shut_down_err(PNT_ERR, "An error occured while trying to print to file \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
					}
					first = FALSE; /* we set 'first' to false, because the first symbol was now printed */
				}
				else /* unless - this symbol is not the first one - we get to a new line and then print the symbol */
				{
					if(fprintf(out_stream, "\n%s %s", (curr_node->symb).name, adss_str) < 0) /* we get to a new line, print the symbol and its address, and check the status of the process: */
					{
						shut_down_err(PNT_ERR, "An error occured while trying to print to file \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
					}
				}
			} /* end of "if" statement, checking "entry" sepecification of the current symbol */
		} /* end of linked list scanning loop */
	} /* end of array scanning loop */
	
	if(out_stream != NULL) /* in case the file was opened during the process */
	{
		if(fclose(out_stream) != 0) /* we close the file and check the status of the process */
			shut_down_err(FILE_OERATION_ERR, "An error occured while trying to close the file named: \"%s\"", fname); /* in this case an error occured - we shut down the program using 'shut_down_err' */
	}
}
