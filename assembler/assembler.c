/* assembler.c - 'Assembler Program'
@ written by Nadav Kahlon, June 2020.

This program is an assembler program.
To use it, enter (as the command line arguments) different file names (1 or more) in the command line - each of them should be a name of an assembly source file (without a '.as' suffix). The assembler will process each one of them, and in case errors were found in the source code it will print proper error messages.

If no errors occured, the program will produce the corresponding output files:
+	An "object" file (with a '.ob' suffix) - which will hold the resulting machine code.
+	An "externals" file (with a '.ext' suffix) - which will hold the addresses of each external symbol in the machine code (this file will be produced only if there are any external symbols in the code image).
+	An "entries" file (with a '.ent' suffix) - which will hold information about every "entry" specified symbol in the code (this file will be produced only if there are any "entry" symbols in the code).

In case a technical error occured, the program will shut down after printing a proper error message to the standard error file. In such case the program will return one of the following indication constants (defined in ../indicators/indicators.h):
*	FILE_OPERATION_ERR - to indicate that an error occured while trying to open / create / close a file.
*	PNT_ERR - to indicate that an error occured while trying to print to a file.
*	ALLOC_ERR - to indicate that an error occured in the process of memory allocation.


Good Luck !

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../indicators/indicators.h"
#include "../as_core_design/as_core_design.h"
#include "../as_mem_words/as_mem_words.h"
#include "../as_symb_table/as_symb_table.h"
#include "../assembler_io/assembler_io.h"
#include "../as_1st_scan/as_1st_scan.h"
#include "../as_2nd_scan/as_2nd_scan.h"



/* The following functions declarations are declarations for functions which are defined in the rest of the file (after the main function). For more detail about each one of them read the corresponding definitions below. */

static short int scan_as_src_file(const char *fname, WordList *code_img_ptr, WordList *data_img_ptr, ExtList *ext_list_ptr, SymbolTable symb_table);

static void as_proc_file(const char *origin_fname);



/* main - the main function of the assembler program - runs the assembler program explained in the top of this file.
'argc' - the number of command line arguments that the program was invoked with (the "argument counter").
'argv' - a pointer to an array of character strings that contain the arguments, one per string. */
int main(int argc, char *argv[])
{
	int i; /* the index of the current argument in 'argv' */
	
	if(argc == 1) /* in case the only argument is the command name - no file names were entered at all */
		fprintf(stderr, "assembler: expected at least 1 assembly source file name (excluding a finishing \".as\" suffix.)\n");
	else
	{
		for(i = 1; i < argc; i++) /* we scan 'argv'  */
			as_proc_file(argv[i]); /* and process each file name in it using the 'as_proc_file' function (defined below) */
	}
	return 0; /* if we reached here, no errors occured and we can return 0 (which is unequal to the other indicators - see description in ../indicators/indicators.h) */
}



/* scan_as_src_file (static function) - scans the assembler source file named 'fname' completely (twice) and collects all the information about the resulting machine code that is needed by the assembler:
The code image will be stored in '*code_img_ptr';
The data image will be stored in '*data_img_ptr';
The external symbols' addresses list will be stored in '*ext_list_ptr';
And the symbol table of the code will be stored in 'symb_table'.
Returns the status of the process.
In case an error was found in the source code it will print a proper error message to the standard error file.
In case a technical error occured, the program will shut down after printing a proper error message to the standard error file. In such case the program will return an indication value that will indicate the error (as detailed in the top of this file).
* Note: assembly error messages will be printed using the 'printerr' function (declared in ../assembler_io/assembler_io.h). This function updates the file name that is used by that function to be 'fname', and in the end it clears that file name - so the file name that is used by 'printerr' will be defaultly: "unknown-file". */
static short int scan_as_src_file(const char *fname, WordList *code_img_ptr, WordList *data_img_ptr, ExtList *ext_list_ptr, SymbolTable symb_table)
{
	short int status; /* to hold the status of the process */
	FILE *source_file; /* a FILE pointer to represent the source file */
	
	if((source_file = fopen(fname, "r")) == NULL) /* we open the source file for reading and check the process */
	{
		shut_down_err(FILE_OERATION_ERR, "An error occured while trying to open the assembly source file named \"%s\"", fname); /* in case an error occured while trying to open the file - we shut down the program using 'shut_down_err' */
	}
	
	set_err_fname(fname); /* after opening the file, we set the character string pointer that is used by 'printerr' to represent the file name in which errors and warning are found (if so) to be the name of the current source file */
	
	status = run_1st_scan(source_file, code_img_ptr, data_img_ptr, symb_table); /* we run the first scan */
	(void)run_2nd_scan(source_file, code_img_ptr, symb_table, &status, ext_list_ptr); /* we run the second scan (even if there were errors in the first scan, in order to prevent us from missing another errors that we couldn't find in the first scan) */
	
	if(fclose(source_file) != 0) /* after scanning the file completely twice, we close it and check the status of the process */
	{
		shut_down_err(FILE_OERATION_ERR, "An error occured while trying to close the file named: \"%s\"", fname); /* in this case an error occured - we shut down the program using 'shut_down_err' */
	}
	
	set_err_fname(NULL); /* to finish, we clear the file name used by 'printerr' - to prevent the char pointer 'fname' to stay there even when it will be discarded */
	
	return status;
}


/* as_proc_file (static function) - this function processes an assembly source file and produces the correspondng output file. 'origin_fname' represents the origin file name - which is the name of the assembly source file, excluding a finishing ".ax" suffix.
In case an error was found in the source code it will print a proper error message to the standard error file.
In case a technical error occured, the program will shut down after printing a proper error message to the standard error file. In such case the program will return an indication value that will indicate the error (as detailed in the top of this file).
* Note: assembly error messages will be printed using the 'printerr' function (declared in ../assembler_io/assembler_io.h). This function updates the file name that is used by that function to be 'fname', and in the end it clears that file name - so then the file name that is used by 'printerr' will be set defaultly: "unknown-file". */
static void as_proc_file(const char *origin_fname)
{
	const unsigned int origin_fname_len = strlen(origin_fname); /* a constant variable to hold the size of the origin file name (initialized using the standard library function 'strlen) */
	char *curr_fname; /* a pointer to the name of the current file that we process - we use a single pointer to hold the name of each one of them in turn in order to save copy time and memory space */
	
	/* The following are variables that will hold different data structures with information about the code */
	SymbolTable symb_table; /* the symbol table */
	WordList code_img = NEW_WORD_LIST; /* the code image (initialized to an empty word list) */
	WordList data_img = NEW_WORD_LIST; /* the data image (initialized to an empty word list) */
	ExtList ext_list = NEW_EXT_LIST; /* the external symbols' addresses list (initialized to an empty list) */
	
	intlz_symbt(symb_table); /* we initialize the symbol table to be empty table */
	curr_fname = malloc(origin_fname_len + 5); /* we allocate enough space to hold the longest file name from the files that we process: the length of the original file name (with no suffix), plus 4 more bytes to hold the longest suffix of the 3 possible suffixes: ".ext"/".ent"/".as", plus 1 more character for a finishing '\0' characer */
	strcpy(curr_fname, origin_fname); /* and we copy the original file name onto 'curr_fname' */
	
	/* we process the assembly source file: */
	strcat(curr_fname, ".as"); /* we first get the file name by concatenate the suffix ".as" to the original file name */
	if(scan_as_src_file(curr_fname, &code_img, &data_img, &ext_list, symb_table) == ALL_GOOD) /* we scan the file using 'scan_as_src_file' (this function will also take care of the error checking and printing) and check the status of the process: */
	{
		/* in this case the process went with no errors, so we produce the output files. Before producing each of them, we copy the corresponding suffix to 'curr_fname' - starting in the 'origin_fname_len' character, so the suffix will be added to the end of the origin file name stored in 'curr_fname'. */
		
		strcpy(curr_fname + origin_fname_len, ".ob"); /* we set the suffix of 'curr_fname' to ".ob" */
		create_ob_file(curr_fname, code_img, data_img); /* and produce the "object" file */
		
		strcpy(curr_fname + origin_fname_len, ".ext"); /* we set the suffix of 'curr_fname' to ".ext" */
		form_ext_file(curr_fname, ext_list); /* and produce the "externals" file (in case it should be produced) */
		
		strcpy(curr_fname + origin_fname_len, ".ent"); /* we set the suffix of 'curr_fname' to ".ent" */
		form_ent_file(curr_fname, symb_table); /* and produce the "entries" file (in case it should be produced) */
	}
	else /* otherwise an error occured - we tell the user that the output files will not be produced */
		fprintf(stderr, "assembler: an error was found in assembly source file \"%s\" - the assembler is not able to produce output files.\n\n\n", curr_fname);
	
	free(curr_fname); /* we free the memory space we allocated for curr_fname */
	clear_symb_table(symb_table); /* clear the symbol table (and free any allocated space) */
	clear_word_list(&code_img); /* clear the code image (and free any allocated space) */
	clear_word_list(&data_img); /* clear the data image (and free any allocated space) */
	clear_ext_list(&ext_list); /* and clear the external symbols' addresses list (and free any allocated space) */
}

