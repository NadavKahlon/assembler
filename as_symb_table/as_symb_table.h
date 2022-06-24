/* as_symb_table.h - 'Assembler Symbol Table' - header file
@ written by Nadav Kahlon, May 2020.

This file presents an interface to handle symbol tables in an assembler program.

A symbol is a string of characters (called the "symbol name") which appears in the input and represents a numeric address and an ARE field, which create a memory word in machine language (represented by a Word variable) that should replace the symbol in the final output. A symbol is represented by the Symbol data type which is defined is ../as_core_design/as_core_design.h.

This interface allows you to handle a "symbol table" conveniently. A symbol table is a data structure that holds symbols, here it is represented by the "SymbolTable" data type.
We implement the symbol table using the hash table data structure.

* Note: in order to make sure you use this interface properly, we recommend you to intialize new SymbolTable variables only using the "intlz_symbt" function (which is declared below), and manage it using only this interface's routines.


*/


/* AS_SYMB_TABLE_IN - an arbitrary constant that was meant to prevent multiple inclusions of "as_symb_table.h". If defined already - it indicates that "as_symb_table.h" is already included in a file. */
#ifndef AS_SYMB_TABLE_IN
#define AS_SYMB_TABLE_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "as_symb_table.h" for the first time. */


#include "../as_core_design/as_core_design.h"



/* struct symb_lnode - represents a single node in a linked list of character strings */
struct symb_lnode {
	struct symb_lnode *next; /* a pointer to the next node in the list */
	Symbol symb; /* the symbol that this node is holding */
};

/* SYMBT_HSIZE - the size of the symbol hash table array (therefore hash values are ranged between 0 and SYMBT_HSIZE-1). */
#define SYMBT_HSIZE 58


/* SymbolTable - A data structure that holds assembly symbols represented by the Symbol data type (the table is implemented as an "hash table"). */
typedef struct symb_lnode *SymbolTable[SYMBT_HSIZE]; /* (the table is an array of pointers to the first node of linked lists of symbols) */


/* intlz_symbt - sets the symbol table entered as the parameter 'table' to a new and empty table.
Note that this is how symbol tables should be initialized using this interface. */
void intlz_symbt(SymbolTable table);


/* symb_lookup - looks for a symbol named 'str' in 'table' and returns a poiner to it, or NULL if the symbol was not found. We assume that 'str' is not NULL. */
Symbol *symb_lookup(SymbolTable table, const char *str);


/* symb_inst - installs a symbol in the symbol-table 'table'. The symbol's name is represented by 'name' (we assume that it's not NULL), its replacement word is 'mem_word', and the rest of the parameters are the 3 flag values of the symbol (they are EXPECTED to be 1 bit long). Allocates new memory space for it and for the copy of the name using the standard library function malloc.
Returns a status value: ALLOC_ERR if an error occurred while trying to allocate space, DUP_ERR if a symbol with this name already exist in 'table', or ALL_GOOD if the process went with no errors (this values are defined in ../indicators/indicators.h). In case of an error nothing happens to the table.
Note: this function does not check the validitiy of the symbol name. You have to enter a valid symbol information. */
short int symb_inst(SymbolTable table, const char *name, Word rep_word, short int is_extern, short int is_entry, short int is_data);


/* clear_symb_table - frees any allocated space that the symbol table 'table' is holding - "clearing" every memory space that it takes. It also initializes the table back into an empty form - so no illegal pointers will be left.
Assumes that the table was managed ONLY using the funtions in this interface. */
void clear_symb_table(SymbolTable table);


/* inc_data_symbs - increase the address that is represented in the replacement memory word of each symbol in the symbol-table 'table' that represents an adress in the data image (its 'is_data' field is TRUE) by 'inc_val'. */
void inc_data(SymbolTable table, unsigned long int inc_val);


/* form_ent_file - in case ths symbol table 'table' includes symbols with the "entry" specification, this function will creat a ".ent" file which will hold the required information about each "entry" symbol. The file name (including a ".ent" suffix) is specified by 'fname'. Assumes every pointer is not NULL
In case an error occured while trying to open / creat / close the file, the program shuts down - returning FILE_OERATION_ERR, after printing a proper error message (using 'shut_down_err').
In case an error occured while writing to the file the program shuts down - returning PNT_ERR, after printing a proper error message (using 'shut_down_err').
(* These 2 constants are defined in ../indicators/indicators.h.  * 'shut_doen_err' is declared in ../assembler_io/assembler_io.h). */
void form_ent_file(char *fname, SymbolTable table);


#endif /* end of the "#ifndef AS_SYMB_TABLE_IN" conditional inclusion statement */
 
