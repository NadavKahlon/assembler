/* as_1st_scan.h - 'The Assembler's First Scan' - header file
@ written by Nadav Kahlon, June 2020.

The assembler program scans the source code twice, and each scan takes care of another part of the compiling procedure.
This file takes care of the first scan - you can find here the declaration of the 'run_1st_scan' function (defined in the adjacent file as_1st_scan.c) to run the first scan and produce the data image and the symbol table, alongside a rudimentary version of the code image.

The purpose of the first scan is to create the foundation for the information collected by the assembler. Mainly, the first scan is about creating a symbol table to hold the symbols defined in the code (note that entry symbols are marked in the SECOND scan), build the data image, and create the basic structure of the code image (the code image itself - but if an instruction operand relies on a symbol - its additional memory word is set to an arbitrary 0, becasue we cannot know it for sure in the first scan) - these are then used in the second scan which finishes the code and the entry symbol marking procedure, while producing an external symbols' appearances list. This function also produces error messages and prints them to the standard error file in error cases. 
More about it can be found below.

* Note that we use the 'printerr' and the 'shut_down_err' functions declared in ../assembler_io/assembler_io.h for printing errors/warnings, but we do not change the file name specified in the 'printerr' function - so make sure you enter it a proper file name before using 'run_1st_scan'.

*/


/* AS_1ST_SCAN_IN - an arbitrary constant that was meant to prevent multiple inclusions of "as_1st_scan.h". If defined already - it indicates that "as_1st_scan.h" is already included in a file. */
#ifndef AS_1ST_SCAN_IN
#define AS_1ST_SCAN_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "as_1st_scan.h" for the first time. */


#include <stdio.h>
#include "../as_mem_words/as_mem_words.h"
#include "../as_symb_table/as_symb_table.h"


/* run_1st_scan - runs the first scan of the assembly source code found in 'source_file' (which is assumed to be opened for reading).
The code image of the machine code will be stored in '*code_img_ptr' (which is assumed to be initialized to an empty list);
The data image of the machine code will be stored in '*data_img_ptr' (which is assumed to be initialized to an empty list);
The symbol table of the assembly source code will be stored in 'symb_table' (which is assumed to be initialized to an empty symbol table).
In case an error/warning in the code was found, an error/warning message is printed using 'printerr' (declared in ../assembler_io/assembler_io.h) - and the current file name specified there, and INP_ERROR is returned (this constant is defined in ../indicators/indicators.h). In case of error in memory allocation the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err' - declared in ../assembler_io/assembler_io.h). Otherwise, ALL_GOOD is returned.
* Note that the memory word of every operand (in case it is an instruction statement) that relies on a symbol is set arbitrarily to 0 - because in the first scan no information about every symbol is known yet.
Assumes every pointer is not NULL. */
short int run_1st_scan(FILE *source_file, WordList *code_img_ptr, WordList *data_img_ptr, SymbolTable symb_table);

#endif /* end of the "#ifndef AS_1ST_SCAN_IN" conditional inclusion statement */

