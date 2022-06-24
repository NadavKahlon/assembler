/* as_2nd_scan.c - 'The Assembler's Second Scan' - functions' implementation
@ written by Nadav Kahlon, June 2020.

This file contains definitions for different functions to handle the second scan of an assembly source code, as well as the definition of the 'run_2nd_scan' routine declared in the adjacent file 'as_2nd_scan.h'.

* Note that we use the 'printerr' function declared in ../assembler_io/assembler_io.h for printing errors/warnings in almost every function. We also use 'get_token' and 'get_line' for input reading (these functions are declared in ../assembler_io/assembler_io.h as well). So if you see them, note that we refer to the functions declared in that file.
* Most of the functions here return indicator constants - which are defined in ../indicator/indicators.h.


The functions defined here are:

** get_rltv_opnd_word (static) - to get the memory word that a relatively addressed operand represents.
** str_to_opnd_2 (static) - to convert a character string that represents an operand into an Operand variable, as part f the second scan of the source code.
** proc_opnd_2 (static function) - to completely process an operand, as part of the second scan of the assembly source code.
** proc_inst_2 (static) - to processe a whole instrucion statement as part of the first scan of an assembly source code.
** proc_entry_guide (static function) - to processe a ".entry" guidance statement.
** proc_line_2 (static function) - to process a whole line in an assembly source code, as part of the second scan of it.
- run_2nd_scan - to run the second scan of an assebmly source code on a given source file.

*/


#include <stdio.h>
#include <string.h>
#include "../indicators/indicators.h"
#include "../as_core_design/as_core_design.h"
#include "../as_mem_words/as_mem_words.h"
#include "../as_symb_table/as_symb_table.h"
#include "../assembler_io/assembler_io.h"


/* get_rltv_opnd_word (static function) - 'symb_name' represents a symbol name that follows a '&' character in an operand which is addressed using the relative addressing method. This function creates the additional memory word of the operand and stores it in '*res_ptr'. 'curr_inst_adss' is the address of the beginning of the current instruction in the code image (the address that we compare to the address of the symbol), and 'symb_Table' represents the symbol table of the program.
In case no errors occured, ALL_GOOD is returned. Otherwise INP_ERROR is returned and a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes every pointer is not NULL. */

/* Algorithm explanation: we first look for 'symb_name' in 'symb_table' using 'symb_lookup' (declared in ../as_symb_table/as_symb_table.h) - and make sure that it was found and that it is not external (otherwise - we print an error message). Then, we calculate the distance between the current address - 'curr_inst_adss', and the address that the symbol represents, set it to the non-ARE field of the result, and set the ARE field to A ("absolute")/ */
static short int get_rltv_opnd_word(char *symb_name, Word *res_ptr, SymbolTable symb_table, Address curr_inst_adss, unsigned long int line_num)
{
	Symbol *symb; /* to hold a pointer to the information about the symbol in the symbol table */
	Address symb_adss; /* to hold the address that the symbol represents */
	short int status; /* to hold the status of the process */
	
	if((symb = symb_lookup(symb_table, symb_name)) == NULL) /* we look for the symbol and make sure that it was found */
	{ /* in this case it was not found */
		printerr(NULL, line_num, FALSE, "Unknown symbol named \"%s\" was found after a '&' character, using the relative operand addressing method.", symb_name);
		status = INP_ERROR;
	}
	else if(symb->is_extern) /* in case the symbol is external*/
	{
		printerr(NULL, line_num, FALSE, "External symbol \"%s\" was found after a '&' character, using the relative operand addressing method.\nThe relative addressing method can be applied to internal symbols only.", symb_name);
		status = INP_ERROR;
	}
	else /* in case the symbol was found and it is internal - so we should set the opernad */
	{
		/* we specify the address of the symbol using the 'get_symb_adss' macro defined in ../as_core_design/as_core_design.h */
		symb_adss = get_symb_adss(*symb);
		/* then we can set the non-ARE field of the result to be the distance between that address and 'curr_inst_adss' (in signed form). Note that we convert it first to 21 bit form using the 'long_to_s21b' macro defined in ../as_mem_words/as_mem_words.h: */
		set_word_field(NON_ARE, *res_ptr, long_to_s21b((signed long int)symb_adss - (signed long int)curr_inst_adss), 3);
		set_word_field(ARE, *res_ptr, ARE_A_set, 0); /* and set the ARE field to A ("absolute") */
		status = ALL_GOOD;
	}
	
	return status;
}



/* str_to_opnd_2 (static function) - converts the charcater string 'opnd_str', which represents an instruction operand (and is not empty), into an Operand variable - and store the result in 'res_ptr', as part of the second scan of the assembly source code.
The 'adss_method' field of *res_ptr' is set, but the 'info' field is set only in case of relative / direct addressing method - because every operand in imediate / direct register addressing method was already checked and determined in the first scan. In addition, we do not check the validity of the operands if they are not addressed in relative / direct addressing method, for the same reason.
'symb_table' represents the symbol table of the program, 'curr_inst_adss' represents the address of the beginning of the current instrucion in the code image, and 'line_num' represents the number of the current line in the source code. In case no errors occured, ALL_GOOD is returned. Otherwise INP_ERROR is returned and a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes that all pointers are not NULL. */

/*** Algorithm explanation: we split the task into 4 cases - each corresponds to one addressing method. In imediat / direct register addressing method we just set the 'adss_method' field of the result. In relative addressing method we use the 'get_rltv_opnd_word' funciton defined above to get the memory word that represents the operand and store it in the result. In direct addressing method we use 'symb_lookup' (declared in ../as_symb_table/as_symb_table.h) to look for the symbol in the table, and store it in the result. AT each part we keep track of the status of the process. */
static short int str_to_opnd_2(char *opnd_str, Operand *res_ptr, SymbolTable symb_table, Address curr_inst_adss, unsigned long int line_num)
{
	Symbol *new_symb_ptr; /* in case of direct addressing method, it will point to the corresponding symbol in the table */
	Word word; /* in case of relative addressing method, it will hold the memory word of the operand */
	short int status; /* to hold the status of the process */
	
	if(*opnd_str == '#') /* in case the first character is '#' - it is addressed using imediate addressing method */
	{
		res_ptr->adss_method = imediate_a; /* set the addressing method */
		status = ALL_GOOD;
	}
	else if(reg_check(opnd_str) != -1) /* in case the operand is a register */
	{
		res_ptr->adss_method = direct_reg_a;
		status = ALL_GOOD;
	}
	else if(*opnd_str == '&') /* in case the first character is '&' - is is addressed using relative addressing method */
	{
		res_ptr->adss_method = relative_a; /* set the addressing method */
		if((status = get_rltv_opnd_word(opnd_str+1, &word, symb_table, curr_inst_adss, line_num)) == ALL_GOOD) /* we get the memory word that represents the operand and make sure that the operand was valid */
			res_ptr->info.mem_word = word; /* in case it is valid, we can set it to the result */
	}
	else /* in this case it has to be a symbol name - addressed using the direct addressing method */
	{
		res_ptr->adss_method = direct_a; /* set the addressing method */
		if((new_symb_ptr = symb_lookup(symb_table, opnd_str)) == NULL) /* we look for the symbol in the table and make sure it was found */
		{ /* in this case it was not found - we print an error message */
			printerr(NULL, line_num, FALSE, "Unknown symbol named \"%s\" was found using the direct operand addressing method.", opnd_str);
			status = INP_ERROR;
		}
		else /* in case it was found */
		{
			res_ptr->info.symb_ptr = new_symb_ptr; /* we set the operand to represent it */
			status = ALL_GOOD;
		}
	}
	
	return status;
}



/* proc_opnd_2 (static function) - this function processes an operand as part of the second scan of the assembly source code.
'opnd_str' is a (non empty) token that represents an instruction operand; 'curr_word_ptr' points to a pointer to the current node in the code image (or NULL); 'IC_ptr' points to an Address variable that holds the current address in the code image (the instruction counter); 'curr_inst_adss' represents the address of the beginning of the current instruction in the code image; 'ext_list_ptr' is a pointer to the list of external symbols' appearances in the code image (or NULL); and 'symb_table' is the symbol table of the assembly program.
We determine the characteristics of the operand and if it is addressed using direct / relative addressing method - we set the current memory word in the code image to represent it (only if 'curr_word_ptr' and '*curr_word_ptr' are not NULL) - completing the representation of the code image. In the end, in any case that the operand required an additional memory word to the instruction (and only if 'curr_word_ptr' and '*curr_word_ptr' are not NULL, of course), '*curr_word_ptr' will point the next unprocessed memory word in the code image (or NULL in the end of it) and '*IC_ptr' will be updated accordingly.
In case an external symbol appeared, '*ext_list_ptr' is updated accordingly - only if 'ext_list_ptr' is not NULL.

In case no errors occured, ALL_GOOD is returned. Otherwise INP_ERROR is returned and a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
In case of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err').
Assumes every pointer (except for 'curr_word_ptr', '*curr_word_ptr', and 'ext_list_ptr') isn't NULL. */

/*** Algorithm explanation: we use 'str_to_opnd; defined above to convert the character string into Operand form - and look for errors. Then, we update the code image and the external symbol list according to the addressing method of the operand (using a switch statement) - more details can be found below. */
static short int proc_opnd_2(char *opnd_str, struct word_node **curr_word_ptr, Address *IC_ptr, Address curr_inst_adss, ExtList *ext_list_ptr, SymbolTable symb_table, unsigned long int line_num)
{
	Operand opnd; /* to represent the operand in form of the Operand data type */
	short int status; /*to hold the status of the process */
	
	if((status = str_to_opnd_2(opnd_str, &opnd, symb_table, curr_inst_adss, line_num)) == ALL_GOOD) /* we convert the character string 'opnd_str' to Operand form and check the process */
	{ /* in this case the operand is valid */
		if(curr_word_ptr != NULL && *curr_word_ptr != NULL) /* in such case the user does want to update the code image */
		{
			switch(opnd.adss_method) /* so we split the task according to the addressing method of the operand */
			{
				case imediate_a: /* in case of imediate addressing */
					*curr_word_ptr = (*curr_word_ptr)->next; /* the current word was set to represent it in the first scan - so we skip it */
					(*IC_ptr)++; /* and increment the instruction counter */
					break;
				case direct_reg_a: /* in case of direct register addressing */
					break; /* there is nothing to do - it was set in the first scan and does not take an additional memory word */
				case relative_a: /* in case of relative addressing */
					(*curr_word_ptr)->data = opnd.info.mem_word; /* the memory word of the current node in the code image is set to be the operand's memory word */
					*curr_word_ptr = (*curr_word_ptr)->next; /* after setting the current memory word we continue to the next one */
					(*IC_ptr)++; /* and increment the instruction counter */
					break;
				default: /* in this case it has to be addressed using direct addressing method */
					if(ext_list_ptr != NULL && (opnd.info.symb_ptr)->is_extern) /* in case 'ext_list_ptr' is not NULL - the user wishes to update the external symbol list, so if it is an appearance of an external symbol we should add it to the list */
					{
						/* we add it to the external symbol list and check for error in memory allocation */
						if(ext_list_add((opnd.info.symb_ptr)->name, *IC_ptr, ext_list_ptr) == ALLOC_ERR)
							shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* we shut down the program and print an error message */	
					}
					/* now we add it to the current position in the code image: */
					(*curr_word_ptr)->data = (opnd.info.symb_ptr)->rep_word; /* the memory word of the current node in the code image is set to be the symbol's replacement word */
					*curr_word_ptr = (*curr_word_ptr)->next; /* after setting the current memory word we continue to the next one */
					(*IC_ptr)++; /* and increment the instruction counter */
					break;
			}
		}
	}
	return status;
}



/* proc_inst_2 (static function) - this function processes an instrucion statement as part of the first scan of an assembly source code.
'arg_line' is assumed to be the rest of the line that follows an instruction name (the "argument line"); 'curr_word_ptr' is a pointer to a pointer to the current node in the code image (or NULL); 'IC_ptr' points to an Address variable that holds the current address in the code image (the instruction counter); 'ext_list_ptr' is a pointer to the external symbol list of the program (or NULL); and 'symb_table' is the symbol table of the program. 
This function processes the argument line, and updates the code image (only if 'curr_word_ptr' and '*curr_word_ptr' are not NULL). In the end, in any case that the instruction required additional memory words (and only if 'curr_word_ptr' and '*curr_word_ptr' are not NULL, of course), '*curr_word_ptr' will point the next unprocessed memory word in the code image (or NULL in the end of it) and '*IC_ptr' will be updated accordingly.
In case an external symbol appeared in the code image, '*ext_list_ptr' is updated accordingly - if it not NULL.

In case no errors occured, ALL_GOOD is returned. Otherwise INP_ERROR is returned and a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
In case of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err').
Assumes every pointer (except for 'curr_word_ptr' or '*curr_word_ptr') isn't NULL, and that 'arg_line; is no more than MAX_LINE_LEN characters long. */

/*** Algorithm explanation: At the beginning we store the current value of the instruction counter - it is the address of the beginning of the current instruction in the code image. Then we can skip the current memory word in the code image - because it is the first memory word of the instruction and it was set in the first scan. 
We rely on the fact that the first scan had to make sure that the statement and its operands are valid. Therefore we then just keep reading the argument line as long as there are operands in it. We use a "while" loop to do it, each time making sure that a SINGLE comma separates the operands (if not, there was an error which was alerted in the first scan - so we just skip the line). Each time we process the operand using 'proc_opnd_2' defined above. */
static short int proc_inst_2(char *arg_line, struct word_node **curr_word_ptr, Address *IC_ptr, ExtList *ext_list_ptr, SymbolTable symb_table, unsigned long int line_num)
{
	char token[MAX_LINE_LEN + 1]; /* to hold each token from the argument line */
	Address curr_inst_adss = *IC_ptr; /* we store the address of the beginning of the current instruction - which is the current value of the instruction counter */
	short int status = UNFINISHED; /* to hold the status of the process - currently unfinished */
	short int is_1st_opnd = TRUE; /* a flag - TRUE if the next operand that is going to be read is the first operand in the line, or FALSE otherwise */
	
	if(curr_word_ptr != NULL && *curr_word_ptr != NULL) /* in this case we should update the code image */
	{
		*curr_word_ptr = (*curr_word_ptr)->next; /* we skip the current memry word in the code image (because it is the first memory word of the instrucion and it was set in the first scan) */
		(*IC_ptr)++; /* and increment the instruction counter */
	}
			
	while(status == UNFINISHED) /* as long as the process is unfinished and there are more tokens in the argument line, we keep scanning for operands */
	{
		if(get_token(&arg_line, token) == 0) /* we read the next token, and make sure that THERE IS another one */
		{
			/* in this case the line ended here. If no tokens were read (we expected the first token) - it is okay (an empty argument line is fine). Otherwise, a comma was read previously and we expect another operand - and it is an error (that was alerted in the first scan) */
			status = is_1st_opnd? ALL_GOOD : INP_ERROR;
		}
		else /* otherwise - we should make sure that it is a valid operand, and process it */
		{
			if(strcmp(token, ",") == 0) /* in case there is an unexpected comma (before the expected operand) */
			{
				status = INP_ERROR; /* we found an error (that was alerted in the first scan) - we can skip this line */
			}
			else if(proc_opnd_2(token, curr_word_ptr, IC_ptr, curr_inst_adss, ext_list_ptr, symb_table, line_num) == INP_ERROR) /* otherwise, this is an operand - we need to process it as part of the second scan of the assembly source code */
			{
				status = INP_ERROR; /* in case an error occured (it was alerted in the 'proc_opnd_2' function) */
			}
			else if(get_token(&arg_line, token) == 0) /* in this case the operand was valid - now, if there is another token, there is another operand, and there has to be separating comma, so we read the next token and check it */
			{
				status = ALL_GOOD; /* in case the line ended here, no errors occured and the process is done */
			}
			else if(strcmp(token, ",") != 0) /* otherwise, there has to be another operand and the current token has to be a separating comma */
			{
				status = INP_ERROR; /* in case the next (non-empty) token is NOT a separating comma - it is a synax error (that was alerted in the first scan) */
			}
			else /* otherwise the operand and a separating comma were read successfuly */
				is_1st_opnd = FALSE; /* so we EXPECT another operand after the comma - the next one is not the first one */
		}
	}
	return status;
}



/* proc_entry_guide (static function) - processes a ".entry" guidance statement.
'line' is assumed to be the rest of the line after a ".entry" statement name. This function processes it, set the sepcified symbol as "entry" in the symbol table 'symb_table', and returns ALL_GOOD in case no errors occured or INP_ERROR otherwise.
In case no errors occured, ALL_GOOD is returned. Otherwise INP_ERROR is returned and a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
In case of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err').
Assumes every pointer is not NULL, that '*line' is no more than MAX_LINE_LEN characters long. */

/*** ALgorithm explanation: we basically just read the next token from the argument line, make sure that there is another token, look for it in the symbol table, make sure that it was found, and set the 'is_entry' field of it to "TRUE". Then, we just need to make sure that the line ends here. */
static short int proc_entry_guide(char *line, SymbolTable symb_table, unsigned long int line_num) 
{
	char token[MAX_LINE_LEN + 1]; /* to hold the name of the entry symbol */
	Symbol *symb_ptr; /* to point the specified symbol in the symbol table */
	short int status; /* to hold the status of the process */
	
	if(get_token(&line, token) == 0) /* in case the rest of the line is empty */
	{
		printerr(NULL, line_num, FALSE, "A symbol name was expected in a \".entry\" statement.");
		status = INP_ERROR;
	}
	else /* otherwise there is an argument */
	{
		if((symb_ptr = symb_lookup(symb_table, token)) == NULL) /* we look for the symbol in the table */
		{
			/* in this case the symbol was not found in the table */
			printerr(NULL, line_num, FALSE, "Unknown symbol \"%s\" was found in a \".entry\" statement.", token);
			status = INP_ERROR;
		}
		else if(symb_ptr->is_extern) /* in case the symbol is external */
		{
			printerr(NULL, line_num, FALSE, "\".entry\" statement expects an internal symbol. The symbol \"%s\" is external.", token);
			status = INP_ERROR;
		}
		else /* otherwise the symbol was found and it is interanl */
		{
			symb_ptr->is_entry = TRUE; /* we set its entry field to TRUE */
			status = line_end_check(&line, "\".enrty\" statement expects a single parameter - extraneous text in the end of it was encountered", line_num); /* now we just verify that the line actually ends after the first argument */
		}
	}
	return status;
}



/* proc_line_2 (static function) - This function processes a statement line in an assembly source code, as part of the second scan of it.
'line' represents the 'line_num' line of an assembly source code - that the name of the file holding it is specified in the 'printerr' function. 'curr_word_ptr' is a pointer to a pointer to the current node in the code image (or NULL); 'IC_ptr' is a pointer to the current address in the code image (the instruction counter); 'ext_list_ptr' is a pointer to the external symbol list of the program (or NULL); and 'symb_table' is the symbol table of the program. 
This function processes the line - completes the code image of it (in case it is an instruction statement); updates '*curr_word_ptr' (to point the next unprocessed node in the code image), '*IC_ptr', '*ext_list_ptr'; and sets the corresponding "entry" symbol in the symbol table (in case it is a ".entry" statement).
* Note that if 'curr_word_ptr' or '*curr_word_ptr' are NULL - the code image will not be updated, and if 'ext_list_ptr' is NULL - it will not be updated.

In case of an assembly error was encountered, a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned. In case of an error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, in case the process went successfuly - ALL_GOOD is returned.
Assumes 'line' is no more than MAX_LINE_LEN characters long (that the length of the line was examined - and it is not too long), and that 'line' and 'IC_ptr' are not NULL. */
static short int proc_line_2(char *line, struct word_node **curr_word_ptr, Address *IC_ptr, ExtList *ext_list_ptr, SymbolTable symb_table, unsigned long int line_num)
{
	char token[MAX_LINE_LEN + 1]; /* to hold each token from the line */
	unsigned int token_len; /* to hold the length of the tokens */
	short int is_symb_dec; /* a flag: TRUE if the line holds a symbol declaration, and FALSE otherwise */
	short int status = UNFINISHED; /* to hold the status of the process (currently unfinished) */
	
	if(*line == ';' || (token_len = get_token(&line, token)) == 0) /* in case the line starts with ';' (it's a comment) or in case there are no non-white tokens (it's empty). note that we read the first token here as well. */
		status = ALL_GOOD; /* we are not supposed to process the line and no issues can occur */
	else /* otherwise - we need to process the line */
	{
		if(token[token_len-1] == ':') /* in case the first token end with ':' - it is a symbol declaration */
		{
			is_symb_dec = TRUE; /* we set 'is_symb_dec' */
			if(get_token(&line, token) == 0) /* and we read the next token - which should be the key word of the line */
				status = ALL_GOOD; /* the rest of the line is empty - we finish here (the fact that a symbol declaraction was encountered in an empty line was alerted in the first scan) */ 
		}
		else /* in case the first token is not a symbol declaration - it is the key word of the line */
			is_symb_dec = FALSE; /* we set '*is_symb_dec' */
	}
	
	/* at this point, if 'status' is still UNFINISHED - 'token' is the key word of the line and 'line' is the rest of it */
	if(status == UNFINISHED) /* so in this case we should process the line */
	{
		if(token[0] == '.') /* if the key word starts with '.' - it is a guidance statement */
		{
			if(strcmp(token+1, "entry") == 0) /* in case the guidance statement is an entry statement */
			{
				if(is_symb_dec) /* in case there is a symbol declaration - we print a warning message and ignore it */
					printerr(NULL, line_num, TRUE, "A symbol declaration was encountered in a \".entry\" statement - it is ignored.");
				status = proc_entry_guide(line, symb_table, line_num); /* and we process the statement */
			}
			else /* otherwise - this guidance statement was checked and processed in the first scan, so we skip it */
				status = ALL_GOOD;
		}
		else /* otherwise it has to be an instruction statement - we process it */
		{
			if(find_ass_inst(token) != NULL) /* we make sure that the instruction name is valid */
				status = proc_inst_2(line, curr_word_ptr, IC_ptr, ext_list_ptr, symb_table, line_num); /* and process it */
			else /* otherwise, it is an error that was alerted in the first scan */
				status = INP_ERROR;
		}
	}
	return status;
}



/* run_2nd_scan - runs the second scan of the assembly source code found in 'source_file' (which is assumed to be opened for reading).
The rudimentary version of the code image that was created in the first scan is specified by '*code_img_ptr';
The symbol table of the assembly source code that was created in the first scan is specified by'symb_table';
And the current status of the assembler process (that was collected from the first scan) is stored in '*status_ptr', and in case of an error it will be updated to INP_ERROR.
The external-symbol-appearances list of the machine code will be stored in '*ext_list_ptr' (which is assumed to be initialized to an empty list).
Returns '*status_ptr'. In case an error/warning in the code was found, an error/warning message is printed using 'printerr'. In case of error in memory allocation the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, ALL_GOOD is returned.
Assumes every pointer is not NULL. */
short int run_2nd_scan(FILE *source_file, WordList *code_img_ptr, SymbolTable symb_table, short int *status_ptr, ExtList *ext_list_ptr)
{
	char line[MAX_LINE_LEN + 1]; /* to hold each line of the source code in turn */
	unsigned long int line_num = 0; /* to hold the number of the current line */
	short int keep_reading = TRUE; /* a flag: TRUE if the end of the file was not encountered (and we should keep reading lines) or FALSE otherwise */
	struct word_node *curr_word = code_img_ptr->head_ptr; /* a pointer to the current node in the code image - initialized to point the first node */
	Address IC = INITIAL_LOAD_ADSS; /* the current address in the code image (the instruction counter) - initialized to the initial loading address */
	
	rewind(source_file); /* we start reading from the start of the source file */
	while(keep_reading) /* we keep reading until the end of the input (when 'keep_reading' is FALSE) */
	{
		line_num++; /* before reading the next line, we update the number of it */
		switch(get_line(source_file, line, NULL)) /* we read the next line onto 'line' and check the process */
		{
			case EOF_ENC: /* in case we reached the end of the source file */
				keep_reading = FALSE;
				break;
			case STR_TOO_LONG: /* in case the line is too long */
				break; /* we simply just skip it - the first scan handled it completely */
				
			default: /* in case the line is valid - we process it (note that the case in which 'get_line' returns NULL_ENC is not checked, because 'line' is not NULL) */
				if(*status_ptr != INP_ERROR) /* in case no input error occured yet (including the first scan) */
				{
					if(proc_line_2(line, &curr_word, &IC, ext_list_ptr, symb_table, line_num) == INP_ERROR) /* we procoess the line using 'proc_line_2' and check if an error occured */
						*status_ptr = INP_ERROR; /* if so, we update the status */
				}
				else /* in case an error occured previously - we call proc_line_1 with NULL and NULL instead of 'curr_word_ptr' and 'ext_list_ptr' because we do not want to spend time on updating the code image and the external symbol list - an error already occured and we have no reason to keep building them. */
					proc_line_2(line, NULL, &IC, NULL, symb_table, line_num);
				break;
		}
	}
	
	return *status_ptr;
}


