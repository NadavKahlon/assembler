/* as_2nd_scan.h - 'The Assembler's Second Scan' - header file
@ written by Nadav Kahlon, June 2020.

The assembler program scans the source code twice, and each scan takes care of another part of the compiling procedure.
This file takes care of the second scan - you can find here the declaration of the 'run_2nd_scan' function (defined in the adjacent file as_2nd_scan.c) to run the second scan and complete the code image of the program, while creating a list of external symbols' appearances in the code image (using the 'ExtList' data type defined in ../as_mem_word/as_mem_words.h).

The purpose of the second scan is mainly to complete the code image, after the symbol table was finally completed, and every memory word that relied on a symbol can now be determined. Another purpose of the second scan is to process the ".entry" statements - check the syntax and set the 'entry' symbols in the symbol table.
'run_2nd_scan' also produces error messages and prints them to the standard error file in error cases. 
More about the different functions can be found below.

* Note that we use the 'printerr' and the 'shut_down_err' functions declared in ../assembler_io/assembler_io.h for printing errors/warnings, but we do not change the file name specified in the 'printerr' function - so make sure you enter it a proper file name before using 'run_2nd_scan'.

*/


/* AS_2ND_SCAN_IN - an arbitrary constant that was meant to prevent multiple inclusions of "as_2nd_scan.h". If defined already - it indicates that "as_2nd_scan.h" is already included in a file. */
#ifndef AS_2ND_SCAN_IN
#define AS_2ND_SCAN_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "as_2nd_scan.h" for the first time. */


#include <stdio.h>
#include "../as_mem_words/as_mem_words.h"
#include "../as_symb_table/as_symb_table.h"



/* run_2nd_scan - runs the second scan of the assembly source code found in 'source_file' (which is assumed to be opened for reading).
The rudimentary version of the code image that was created in the first scan is specified by '*code_img_ptr';
The symbol table of the assembly source code that was created in the first scan is specified by 'symb_table';
the current status of the assembler process (that was collected from the first scan) is stored in '*status_ptr', and in case of an error it will be updated to INP_ERROR (defined in ../indicators/indicators.h).
The external-symbol-appearances list of the machine code will be stored in '*ext_list_ptr' (which is assumed to be initialized to an empty list).
Returns '*status_ptr'. In case an error/warning in the code was found, an error/warning message is printed using 'printerr' (declared in ../assembler_io/assembler_io.h). In case of error in memory allocation the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err' - declared in ../assembler_io/assembler_io.h).
Assumes every pointer is not NULL. */
short int run_2nd_scan(FILE *source_file, WordList *code_img_ptr, SymbolTable symb_table, short int *status_ptr, ExtList *ext_list_ptr);


#endif /* end of the "#ifndef AS_2ND_SCAN_IN" conditional inclusion statement */

