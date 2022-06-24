/* as_1st_scan.c - 'The Assembler's First Scan' - functions' implementation
@ written by Nadav Kahlon, June 2020.

This file contains the definitions of the main functions that help us run the first scan of the assembly source code, as well as the final 'run_1st_scan' function declared in the adjacent header file "as_1st_scan.h".
These functions are mainly meant to help you run the first scan of the assembler program - more info can be found in as_1st_scan.h.

* Note that we use the 'printerr' function declared in ../assembler_io/assembler_io.h for printing errors/warnings in almost every function. We also use 'get_token' and 'get_line' for input reading (these functions are declared in ../assembler_io/assembler_io.h as well). So if you see them, note that we refer to the functions declared in that file.
* Most of the functions here return indicator constants - which are defined in ../indicator/indicators.h.


The functions defined here are:

** get_imed_opnd (static) - to get an imediate addressed operand.
** new_symb_add_1 (static) - to verify a symbol declaration and add it to a symbol table.
** get_opnd_1 (static) - to get an operand, as part of the first scan of the source code.
** inst_opnd_match (static) - to verify that a source/destination operand matches the corresponding instruction.
** get_inst_opnds_1 (static) - to get an instruction's operands, as part of the first scan of the source code.
** creat_inst_word (static) - creates the first memory word in the machine code image of an instruction statement.
** opnd_word_add_1 (static) - to add a memory word to the code image of the program based on a given operand (as part of the first scan of the source code).
** update_code_img_1 (static) - to update the code image of the program according to an instruction's operands as part of the first scan of an assembly source code.
** proc_inst_1 (static) - to processe a whole instrucion statement as part of the first scan of an assembly source code.
** proc_data_guide (static) - to process the arguments of a ".data" guidance statement.
** proc_string_guide (static) - to process the argument of a ".string" guidance statmenet.
** proc_extern_guide_1 (static) - to process the arguments of a ".extern" guidance statement.
** proc_guidance_1 (static) - to process a guidance statement as part of the first scan of an assembly source code.
** proc_line_1 (static) - to process a whole line in the assembly source code, as part of the first scan of it.
- run_1st_scan - to run the first scan of the assembly source code on an assembly source file.

*/


#include <stdio.h>
#include <string.h>
#include "../indicators/indicators.h"
#include "../as_core_design/as_core_design.h"
#include "../as_mem_words/as_mem_words.h"
#include "../as_symb_table/as_symb_table.h"
#include "../assembler_io/assembler_io.h"



/*  get_imed_opnd (static function) - checks if 'num_str' represents a valid number in decimal form - which is assumed to follow a '#' character in the input (= imediate addressed operand): if so - '*opnd' is set to represent it as an imediate addressed operand and the function returns ALL_GOOD, otherwise - a proper error is printed using 'printerr' (with the current file name specified there, and 'line_num' as the current line index) and INP_ERROR is returned.
Assumes every pointer is not NULL. In case of an error in numeric translation '*opnd' is not changed. */

/*** Algorithm explanation: we use the 'str_to_long' functcion (declared in ../as_mem_words/as_mem_words.h) to translate the character string into numeric form. We use a switch-case statement to track the status of the process and act accordingly. If no errors occured, we set the operand (the memroy word is set using the 'set_word_field' and the 'long_to_s21b' macros defined in ../as_mem_words_as_mem_words.h). */
static short int get_imed_opnd(const char *num_str, Operand *opnd, unsigned long int line_num)
{
	long int res_val; /* to hold the numeric value of 'num_str' */
	switch(str_to_long(num_str, &res_val)) /* we check if 'num_str' represents a number, and store the result in 'res_val' */ 
	{
		case STR_EMPTY: /* in case 'num_str' is empty */
			printerr(NULL, line_num, FALSE, "A decimal integer is missing after a '#' character (using imediate operand addressing method).");
			return INP_ERROR;
		case INT_EXP: /* in case the operand is not a valid integer */
			printerr(NULL, line_num, FALSE, "A decimal integer was expected after a '#' character (using imediate operand addressing method); '%s' is not a decimal integer.", num_str);
			return INP_ERROR;
			
		case ALL_GOOD: /* in case the number is valid - we set '*opnd' to represent it */
			opnd->adss_method = imediate_a; /* we set the addressing method of the operand */
			set_word_field(NON_ARE, (opnd->info).mem_word, long_to_s21b(res_val), 3); /* we set the non-ARE field of the operand's memory word (starting at the third bit) to be the number that we read from 'num_str' (converted into signed 21 bit long integer) */
			set_word_field(ARE, (opnd->info).mem_word, ARE_A_set ,0); /* and set the ARE field to "Absolute" (A) */
			break;
	}
	return ALL_GOOD;
}



/* new_symb_add_1 (statci function) - checks a symbol declaration and adds the new symbol to the symbol table 'table', as part of the first scan of the assembly source code.
The name of the symbol is 'name'; the address that it represents (which will be set to the non-ARE field of the replacement word) is 'rep_adss'; 'is_extern' should be TRUE in case the symbol is external or FALSE otherwise; 'is_data' should be TRUE if the symbol is part of the data image, and FALSE otherwise.
In case that an assembly error was encountered, a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned. 'dec_place' might be added to the error message to represent the place in which the symbol was declared. In case it couldn't add a symbol to the table because of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, in case the process went successfuly - ALL_GOOD is returned.
Assumes every pointer is not NULL. */

/*** Algorithm explanation: we check that the symbol name is valid using 'symb_check' (declared in ../as_core_design/as_core_design.h). Then, we create the replacement memory word of the symbol - we store it in a local Word variable. We set its ARE field according to 'is_extern' (R for FALSE and E for TRUE) and set the rest of the memory word to be the address. We set the different fields of it using the 'set_word_field' macro (defined in ../as_mem_words/as_mem_words). Finally, we add the symbol with all of his properties using 'symb_inst' (declared in ../as_symb_table/as_symb_table.h) to the table, and check the status of the process with a switch statement. */
static short int new_symb_add_1(SymbolTable table, const char *name, Address rep_adss, short int is_extern, short int is_data, char *dec_place, unsigned long int line_num)
{
	short int status; /* to hold the status of the process */	
	Word rep_word = 0; /* to hold the replacement memory word of the symbol */
	
	if(symb_check(name, dec_place, line_num) == INP_ERROR) /* in case the name is invalid */
		status = INP_ERROR;
	else
	{
		set_word_field(ARE, rep_word, is_extern? ARE_E_set : ARE_R_set, 0); /* we set the ARE field of the word according to 'is_extern' */
		set_word_field(NON_ARE, rep_word, rep_adss, 3); /* we set the address that the symbol represents */
		switch(status = symb_inst(table, name, rep_word, is_extern, 0, is_data)) /* we add the symbol to the table and check the process. Note that the 'is_entry' field is set to 0, because in the first scan we cannot know it yet */
		{
			case ALLOC_ERR: /* in case we had an error in memory allocation */
				shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* we shut down the program and print an error message */
				break;
			case DUP_ERR: /* in case the symbol already exists in the symbol table */
				printerr(NULL, line_num, FALSE, "A symbol named \"%s\" already exists.", name); /* we print an error message */
				break;
			default: /* in case no errors occured */
				break;
		}
	}
	return status == ALL_GOOD? ALL_GOOD : INP_ERROR; /* we return a value according to the status of the process */
}



/* get_opnd_1 (static function) - the 1st scan's single-operand-reading function: reads the next token from '*line_ptr' and checks if it is a proper assembly operand, and stores information about it in '*opnd'. If it is addressed using relative or direct addressing method, the 'adss_method' field of '*opnd' is set, but it does nothing to the 'info' field. The 'info' field is set at imeditae / direct register addressed operand. Assumes that every pointer is not NULL, and that '*line_ptr' is no more than MAX_LINE_LEN characters long.
In case a comma was encountered first, the 'at_comma' message is printed. If the operand is not valid or the end of the line was encountered a proper error message is printed (using 'printerr', and the current file name specified there; the line's index is 'line_num'). In such errors INP_ERROR is returned (note that '*opnd' may already has been changed). Otherwise, ALL_GOOD is returned (these constants are defined in ../indicators/indicators.h). */

/*** Algorithm explanation: we get the next token from the line using the 'get_token' function declared in ../assembler_io/assembler_io.h and check its characteristics to determine its its validity as an operand. We use various functions from this file to check operands in different addressing methods */
static short int get_opnd_1(char **line_ptr, Operand *opnd, const char* at_comma,  unsigned long int line_num)
{
	char token[MAX_LINE_LEN + 1]; /* to hold the next token from '*line_ptr' */
	short int reg_i; /* an variable that will hold the register index in case the operand is a registr */
	
	if(get_token(line_ptr, token) == 0) /* gets the next token from '*line_ptr' onto 'token' and checks if the end of the line was ecountered */
	{
		printerr(NULL, line_num, FALSE, "Too few operands were found in a code line.");
		return INP_ERROR;
	}
	
	if(strcmp(token, ",") == 0) /* in case a comma was encountered */
	{
		printerr(NULL, line_num, FALSE, "%s", at_comma); /* prints a proper error message */
		return INP_ERROR;
	}
	else if(token[0] == '#') /* in case the operand is addressed using the imediate method - we get the operand onto */
		return get_imed_opnd(token+1, opnd, line_num); /* 'opnd' using get_imed_opnd (which takes care of errors). */
	else if((reg_i = reg_check(token)) != (-1)) /* in case the token is a register name - we assign its index to 'reg_i' */
	{
		opnd->adss_method = direct_reg_a; /* we set the addressing method of the operand to 'direct registr addressing' */
		(opnd->info).reg_index = reg_i; /* and set the register index based on the return value of the reg_check macro */
		return ALL_GOOD;
	}
	else if(token[0] == '&') /* in case the operand is addressed using the relative method - we check the symbol name */
	{
		opnd->adss_method = relative_a; /* we set the addressing method */
		return symb_check(token+1, "after a '&' character (using relative operand addressing method)" , line_num); /* and check the symbol name that is in the rest of the string (after the '&'character) */
	}
	else /* in this case the operand has to be symbol name - addressed in the direct addressing method */
	{
		opnd->adss_method = direct_a; /* we set the addressing method */
		return symb_check(token, "while using direct operand addressing method", line_num); /* and check the symbol's name */
	}
}


/* inst_opnd_match (static fucntion) - checks if the operand pointed by 'opnd' can be the source/destination operand (based on the value of 'is_source' - TRUE for source / FALSE for destination) of the instruction pointed by 'inst'. If so, ALL_GOOD is returned. Otherwise, a proper error message is returned. Error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes that every pointer is not NULL, and that the instruction SHOULD get a source/destination operand. */

/*** Algorithm explanation: we just use the different fields of the Instruction variable '*inst' to determine if the addressing method of the operand is matching the capabilities of the instruction. We check separately the case of source operand and the case of destination operand. */ 
static short int inst_opnd_match(Instruction *inst, const Operand *opnd, short int is_source, unsigned long int line_num)
{
	if(is_source) /* in case the operand represents the source operan d of this instruction */
	{
		if((opnd->adss_method == imediate_a && inst->src_imed == FALSE) || /* we check any case in which the actual */
		   (opnd->adss_method == direct_a && inst->src_drct == FALSE) || /* addressing method of the operand is not */
		   (opnd->adss_method == relative_a && inst->src_rltv == FALSE) || /* a possible addressing method of the */
		   (opnd->adss_method == direct_reg_a && inst->src_reg == FALSE)) /* source operand of this instruction. */
		{
			printerr(NULL, line_num, FALSE, "The source operand of the '%s' instruction is addressed using an inappropriate addressing method.", inst->op_name); /* and print a proper error message in this case */
			return INP_ERROR;
		}
		else
			return ALL_GOOD;
	}	
	else /* in case the operand represents the source operan d of this instruction */
	{
		if((opnd->adss_method == imediate_a && inst->des_imed == FALSE) || /* we check any case in which the actual */
		   (opnd->adss_method == direct_a && inst->des_drct == FALSE) || /* addressing method of the operand is not */
		   (opnd->adss_method == relative_a && inst->des_rltv == FALSE) || /* a possible addressing method of the */
		   (opnd->adss_method == direct_reg_a && inst->des_reg == FALSE)) /* destination operand of this instruction. */
		{
			printerr(NULL, line_num, FALSE, "The destination operand of the '%s' instruction is addressed using an inappropriate addressing method.", inst->op_name); /* and print a proper error message in this case */
			return INP_ERROR;
		}
		else
			return ALL_GOOD;
	}
}



/* get_inst_opnds_1 (static function) - the 1st scan's intruction's-operands-reading function: reads the whole line pointed by 'line_ptr' (that is assumed to follow an instruction name in the actual input) and reads the operands that are in it. Information about them (according to the 'get_opnd_1' function) is stored in '*src_opnd_ptr' (the source operands) and '*des_opnd_ptr' (the destination operand). The operands are read according to the number of opereands that the instruction gets and other specifications about it defined in '*inst' - which represents the instruction of the current line.
In case of any assembly syntax error or mismatch between the instruction expectations and the actual operand line, INP_ERROR is returned and a proper error message is printed. Otherwise - ALL_GOOD is returned.
Error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line). Note that in error cases, '*src_opnd_ptr' and '*des_opnd_ptr' may already have been changed.
Note that if the instruction expects 1 operand, only '*des_opnd_ptr' is set, and if it expects no operands at all, non of the Operand pointers are set.
Assumes that the instruction expects no more than 2 operands, that all pointers are not NULL, and that '*line_ptr' is no more than MAX_LINE_LEN characters long. */

/*** Algorithm explanation: we split the task into 3 possible options, according to the number that the instruction expects - 0, 1, or 2 operands. In each case we determine the syntax and read the operands using functions: 'get_opnd_1', 'inst_opnd_match', 'comme_check', and 'line_end_check', that are found defined above. In each case the error messages are set according to the context. We check that every function we call returned ALL_GOOD - to make sure that no errors have occcured in the process. In that case we return ALL_GOOD in the end, or INP_ERROR otherwise. */
static short int get_inst_opnds_1(char **line_ptr, Instruction *inst, Operand *src_opnd_ptr, Operand *des_opnd_ptr, unsigned long int line_num)
{
	switch(inst->opnd_num) /* we split the task according to the number of operands that the instruction expects */
	{
		case 0: /* in case no operands are expected */
			return line_end_check(line_ptr, "The instruction expects no operands - extraneous text after end of the instruction statement was encountered", line_num); /* we just check that the operand line is empty */
		
		case 1: /* in case a single destination operand is expected */
			return ( get_opnd_1(line_ptr, des_opnd_ptr, "Unexpected comma after the instruction name was encountered.", line_num) == ALL_GOOD ) && /* we read it */
			       ( inst_opnd_match(inst, des_opnd_ptr, FALSE, line_num) == ALL_GOOD ) && /* check its validity */
				   ( line_end_check(line_ptr, "The instruction expects a single operand - extraneous text after the end of the instruction statement was encountered", line_num) == ALL_GOOD )? /* and make sure that there are no more tokens */
					ALL_GOOD : INP_ERROR;
		
		case 2: /* in case 2 operands are expected - a source operand and a destination operand */
			return ( get_opnd_1(line_ptr, src_opnd_ptr, "Unexpected comma after the instruction name was encountered.", line_num) == ALL_GOOD ) && /* we read the source operand */
			       ( inst_opnd_match(inst, src_opnd_ptr, TRUE, line_num) == ALL_GOOD ) && /* check its validity */
			       ( comma_check(line_ptr, "A comma is missing between operands", "The instruction expects 2 operands. The second operand is missing.", line_num) == ALL_GOOD ) && /* makes sure that a comma separates the operands */
			       ( get_opnd_1(line_ptr, des_opnd_ptr, "Multiple consecutive commas - expected a single comma between operands.", line_num) == ALL_GOOD ) && /* read the destination operand */
			       ( inst_opnd_match(inst, des_opnd_ptr, FALSE, line_num) == ALL_GOOD ) && /* check its validity */
			       ( line_end_check(line_ptr, "The instruction expects exactly 2 operands - extraneous text after the end of the instruction statement was encountered", line_num) == ALL_GOOD )? /* and make sure that there are no more tokens */
					ALL_GOOD : INP_ERROR;
	}
	return INP_ERROR; /* we should never reach this point (because the instruction has to expect no more than 2 operands), so we return an arbitrary INP_ERROR value in this impossible case */
}



/* creat_inst_word (static function) - creates the first memory word in the code image of an instruction statement based on the instruction represented by '*inst' and the (optional - based on the instruction) destination operand - 'des_opnd', and the (optional - based on the instruction) source operand 'src_opnd', and returns the result. If the instruction does not get a sorce/destination operand, 'src_opnd'/'des_opnd' is not used (respectively).
Assumes that 'inst' is not NULL, and that the operands match the instruction's requirement. */

/*** Algortithm explanation: we check the information we need about the instruction and its operands, and set the different fields of the first memory word of the instruction using the 'set_word_field' macro defined in ../as_mem_words/as_mem_words.h. */
static Word creat_inst_word(Instruction *inst, Operand src_opnd, Operand des_opnd)
{
	Word result = 0; /* to hold the result of the process - initialized to 0 */
	
	set_word_field(ARE, result, ARE_A_set, 0); /* set the ARE field to "absolute" */
	set_word_field(FUNCT, result, inst->funct ,3); /* set the function field to the function index of the instruction */
	set_word_field(OPCODE, result ,inst->op_code, 18); /* set the operation code field to the proper operation code */
	
	if(inst->opnd_num > 0) /* in case there is at least a destination operand - we set its fields accordingly */
	{
		set_word_field(DEST_ADSS, result, des_opnd.adss_method, 11); /* we set its addressing method */
		set_word_field(DEST_REG, result, des_opnd.adss_method == direct_reg_a? des_opnd.info.reg_index : 0, 8); /* and its register index - if the addressing method is "direct regiser" (otherwise, this field is set to 0) */
		
		if(inst->opnd_num > 1) /* in case there is also a source operand - we set its fields accordingly */
		{
			set_word_field(SRC_ADSS, result, src_opnd.adss_method, 16); /* we set its addressing method */
			set_word_field(SRC_REG, result, src_opnd.adss_method == direct_reg_a? src_opnd.info.reg_index : 0, 13); /* and its register index - if the addressing method is "direct regiser" (otherwise, this field is set to 0) */
		}
		else /* in case there is no source operand - every source operand field is set to 0 */
			set_word_field(SRC_REG | SRC_ADSS, result, 0, 13);
	}
	else /* in case there are no operands at all - every operand field is set to 0 */
		set_word_field(DEST_REG | DEST_ADSS | SRC_REG | SRC_ADSS, result, 0, 8);
	
	return result;
}



/* opnd_word_add_1 (static funciton) - adds a memory word corresponding to the instruction operand 'opnd' to the end of the word list '*code_img_ptr' - which represents the code image of the assembler program. The memory word that is added is based only on information that is accessable in the first scan: in imediate addressing method the memory word of the operand is added as it is, but in direct or relative addressing method a memory word with an arbitrary value of 0 is added (because the actual symbol's address is unknown yet), and in direct register addressing no memory word is added at all (because the register is set in the first word of the instruction).
In case it couldn't add memory words to the list because of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err').
Assumes that 'code_img_ptr' is not NULL. */

/*** Algorithm explanation: we use a "switch-case" statement to split the task into each possible addressing method of the operand, and add the corresponding memory word to the list using 'word_list_add' (declared in ../as_mem_words/as_mem_words.h). We keep track of the status of the adding process using the 'add_status' local variable, and check it in the end to make sure no errors occured while allocating new memory space. */
static void opnd_word_add_1(Operand opnd, WordList *code_img_ptr)
{
	short int add_status; /* to hold the status of the adding process */
	switch(opnd.adss_method) /* we split the task into every possible addressing method of the operand */
	{
		case imediate_a: /* in case of imediate addressing method - we imediately add its memory word to the list */
			add_status = word_list_add(opnd.info.mem_word, code_img_ptr);
			break;
		case direct_a: /* in case of direct addressing method - in the first scan we cannot know the address of the symbol, */
			add_status = word_list_add(0, code_img_ptr); /* so we add an arbitrary memory word with a value of 0 */
			break;
		case relative_a: /* in case of relative addressing method - in the first scan we cannot knoe the distance to the */
			add_status = word_list_add(0, code_img_ptr); /* symbol, so we add an arbitrary memory word with a value of 0 */
			break;
		default: /* otherwise - in case of direct register addressing method - we do not add a memory word at all */
			add_status = ALL_GOOD; /* so we just set add_status to ALL_GOOD to indicate no errors */
			break;
	}
	
	if(add_status == ALLOC_ERR) /* we check the status of the adding process - in case of error in memory allocation: */
		shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* in such a case, we shut down the program */
	/* Note that the 'NULL_ENC' case of 'word_list_add' is not possible, because 'code_img_ptr' is assumed not to be NULL. */
}



/* update_code_img_1 (static function) - 'line' is assumed to be a line that holds the operands which follow an instruction name that is represented by 'inst'. The functions reads it and adds the correspunding memory words of the instruction to the word list pointed by 'code_img_ptr' - only in case 'code_img_ptr' is not NULL, as part of the first scan of the assembly source code. That word list represents the code image of the assembly code.
In case of any assembly syntax error or mismatch between the instruction expectations and the actual operand line, INP_ERROR is returned and a proper error message is printed. In case it couldn't add memory words to the list because of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise - ALL_GOOD is returned. In error cases '*code_img_ptr' may already has been changed.
Logical error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes that the instruction expects no more than 2 operands, that all pointers are not NULL (except fot 'code_img_ptr'), and that 'line' is no more than MAX_LINE_LEN characters long. The operands' memory words are added using 'opnd_word_add1' (so it matches the first scan). */

/*** Algorithm explanation - we use 'get_inst_opnd_1' to decipher the operands and make sure that no errors occured in this instruction line. Then, only if 'code_img_ptr' is not NULL, we add the corresponding memory words to the code image. The first word is formed using 'create_inst_word' and the rest of the memory words are added using 'opnd_word_add_1' - in the right order, according to the number of operands. */
static short int update_code_img_1(char *line, Instruction *inst, WordList *code_img_ptr, unsigned long int line_num)
{
	Operand src_opnd, des_opnd; /* variables to hold the (optional) source and destination operands of the instruction */
	
	if(get_inst_opnds_1(&line, inst, &src_opnd, &des_opnd, line_num) == INP_ERROR) /* gets the operands */
		return INP_ERROR; /* we also check the process - in this case there is an error in the line */
	
	if(code_img_ptr != NULL) /* we add the memory words only if 'code_img_ptr' is not NULL */
	{
		if(word_list_add(creat_inst_word(inst, src_opnd, des_opnd), code_img_ptr) == ALLOC_ERR) /* we add the first memory word of the instruction to the code image. the memory word is created using 'creat_inst_word'. */
		{
			shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* in this case we had an error in memroy allocation - we shut down the program */
		}
	
		switch(inst->opnd_num) /* we add the operands memory words based on the number of operands */
		{
			case 0: /* in case there are no operands */
				break; /* so there are memory words to add */
			case 1: /* in case there's a single operand - the destination */
				opnd_word_add_1(des_opnd, code_img_ptr); /* we add it */
				break;
			case 2: /* in case there are 2 operands - a source operand and a destination operand */
				opnd_word_add_1(src_opnd, code_img_ptr); /* we add the source operand first */
				opnd_word_add_1(des_opnd, code_img_ptr); /* and then we add the destination operand */
				break;
		}
	}
	return ALL_GOOD; /* if we reached here, no errors occured - so we can return ALL_GOOD */
}



/* proc_inst_1 (static function) - this function processes an instrucion statement as part of the first scan of an assembly source code.
'arg_line' is assumed to be the rest of the line that follows an instruction named 'inst_name' (which is assumed not to be empty). 'is_symb_dec' should be TRUE if there is a symbol declaration, and FALSE otherwise. In such case the symbol name is specified by 'symb_name'. The code image of the assembly code is represented by the word list '*code_img_ptr' - and it will be updated only if it is not NULL. The symbol table of the code is 'symb_table'.
Note that if 'code_img_ptr' is NULL, new symbols of the data image will be added with an arbitrary address of 0.
In case that an assembly error was encountered, a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned. In case of an error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, in case the process went successfuly - ALL_GOOD is returned.
Assumes 'arg_line' is no more than MAX_LINE_LEN characters long, and that every pointer (except for 'code_img_ptr') is not NULL. */
static short int proc_inst_1(char *arg_line, const char *inst_name, const char *symb_name, short int is_symb_dec, WordList *code_img_ptr, SymbolTable symb_table, unsigned long int line_num)
{
	Instruction *inst_ptr; /* a pointer to the instruction in the instruction table */
	short int status; /* to hold the status of the process */
	
	if(is_symb_dec && new_symb_add_1(symb_table, symb_name, code_img_ptr != NULL? code_img_ptr->size + INITIAL_LOAD_ADSS : 0, FALSE, FALSE, "while declaring a new symbol", line_num) == INP_ERROR) /* we add the symbol declared (in case there is a symbol declaration) as an internal symbol in the code image, and check the process. Note that the address of it is the current size of the code image (= the current address in it) + the initial loading address; or an arbitrary 0 if 'code_img_ptr' is NULL */
	{
		status = INP_ERROR; /* in such case an error occured */
	}
	else /* otherwise, if a symbol had to be declared - it was declared succesfuly. Now we had to process the line */
	{
		if((inst_ptr = find_ass_inst(inst_name)) == NULL) /* we look for the corresponding instruction in the instruction table */
		{ /* in this case the instruction was not found */
			printerr(NULL, line_num, FALSE, "Unknown instruction name \"%s\".", inst_name);
			status = INP_ERROR;
		}
		else
			status = update_code_img_1(arg_line, inst_ptr, code_img_ptr, line_num); /* we process it using 'update_code_img_1' and store the status of the process */
	}
	
	return status;
}



/* proc_data_guide (static function) - 'line' represent the parameter line that follows a ".data" guidance statement. this function processes it, and adds the corresponding memory words to the data image of the assembly code, represented by the word list '*data_img_ptr'. In any case of errors, a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line). and INP_ERROR is returned, otherwise ALL_GOOD is returned.
In case we coudln't add the memory words because of erors in memory allocation, a proper technical error message is printed using'shut_down_err' and the program shuts down.
In case 'data_img_ptr' is NULL, the new data memory words are discarded.
Assumes that line is not NULL, and that 'line' is no more than MAX_LINE_LEN characters long. */

/*** Algorithm explanation: we use a the 'status' local variable to hold the status of the process in each step. We read each parameter in a loop using the 'get_data_word' function declared in ../assembler_io/assembler_io.h, and add it to the list (only if 'datt_img_ptr' is not NULL) using the 'word_list_add' function declared in ../as_mem_words/as_mem_words.h - while making sure that no errors occured. Before each reading (starting in the second one) we look for a comma between the operands - if no token is found we reached the end properly, otherwise we make sure we reached a comma. In the end, we check the status of the process - making sure that we broke out of the loop because we reached the end of the line, and not because of an error (the check is done according to the 'status' local variable - that we set in almost every part of the process to represent the processe's statuss). */
static short int proc_data_guide(char *line, WordList *data_img_ptr, unsigned long int line_num)
{
	Word param_word; /* to hold the memory word of each parameter in his turn */
	short int status; /* to hold the status of the process in each step of it */
	
	status = get_data_word(&line, &param_word, "Unexpected comma after the \".data\" guidance statement name was encountered.", "Expected at least one parameter in the \".data\" guidance statement.", line_num);
	if(status == ALL_GOOD && data_img_ptr != NULL && word_list_add(param_word, data_img_ptr) == ALLOC_ERR) /* and add it to the data image in case no errors occured. Note that we do it only if 'data_img_ptr' is not NULL */
	{
		shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* in case we had an error in memory allocation - we shut down the program and print an error message */
	}
	
	while(status == ALL_GOOD && (status = comma_check(&line, "Expected a comma between the \".data\" statement's parameters", NULL, line_num)) == ALL_GOOD) /* we keep scanning the list, looking for a comma (between operands), until reaching an error or end of line */
	{
		if((status = get_data_word(&line, &param_word, "Unexpected multiple consecutive commas were encountered after a parameter of the \".data\" statement.", "Unexpected comma in the end of the \".data\" statement was encountered.", line_num)) == ALL_GOOD) /* we continue to read the next parameter (which should exist - because there was a comma */
		{
			if(data_img_ptr != NULL && word_list_add(param_word, data_img_ptr) == ALLOC_ERR) /* and add it to the data image (only if no errors occured previously). Note that we do it only if 'data_img_ptr' is not NULL */
			{
				shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* in case we had an error in memory allocation - we shut down the program and print an error message */
			}
		}
	}
	
	if(status == INP_ERROR) /* in case we broke the loop because of error */
		return INP_ERROR;
	else /* in case we broke the loop because we reached the end of the line properly */
		return ALL_GOOD;
}



/* proc_string_guide (static function) - reads the character string 'line' - which is assumed to follow a ".string" guidance statement name, and makes sure that it is a valid assembly character string (white spaces before or after the last '"' characters are ignored). If so, the ASCII value of each character in it (including a finshing '\0' character) is added to the word list '*data_img_ptr' (only if it is not NULL) and ALL_GOOD is returned. Otherwise - a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned.
In case it couldn't add memory words to the list because of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err').
Assumes that 'line' is no more than MAX_LINE_LEN characters long, and that 'line' is not NULL. */

/*** Algorithm explanation: we use 'get_char_string' to transform 'line' into an assembly character string, and check the status of the process. Then, If it is valid and 'data_img_ptr' is not NULL, we scan through the character string and add each character in it (including the finishing '\0' character) to the word list 'data_img_ptr' (only if it is not NULL) using 'word_list_add' - and make sure that no errors occured in memory allocation. We transform each character into a memory word form using the 'char_to_word' macro defined in ../as_mem_words/as_mem_words.h. */
static short int proc_string_guide(const char *line, WordList *data_img_ptr, unsigned long int line_num)
{
	char char_str[MAX_LINE_LEN + 1]; /* to hold the character string entered itself */
	unsigned int i;
	short int status; /* to hold the status of the process */
	
	if(get_char_string(line, char_str, line_num) == INP_ERROR) /* we decipher it and insert it into 'char_str' */
		status = INP_ERROR; /* in case the charcater string is not valid */
	else /* in case the charcter string is valid */
	{
		if(data_img_ptr != NULL) /* in this case we should add the charcters in it to the data image */
		{
			i = 0;
			do { /* we scan through the character string (including the finishing '\0' character) */
				if(word_list_add(char_to_word(char_str[i]), data_img_ptr) == ALLOC_ERR) /* and add each character in the string to the data image and make sure no errors occured */
				{
					shut_down_err(ALLOC_ERR, "An error occured while trying to allocate new memory space"); /* in case we had an error in memory allocation - we shut down the program and print an error message */
				} /* note that the case in which 'word_list_add' returns NULL_ENC is not possible because 'data_img_ptr' is assumed not to be NULL */
			} while(char_str[i++] != '\0');
		}
		status = ALL_GOOD; /* if we reached here, no errors occured */
	}
	return status;
}



/* proc_extern_guide_1 (static function) - processes a ".extern" guidance statement as part of the first scan of an assembly source code.
'line' is assumed to be the rest of the line after a ".extern" statement name. This function processes it, add the new external symbol to 'symb_table'.
In case no errors occured, ALL_GOOD is returned. Otherwise INP_ERROR is returned and a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
In case of error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err').
Assumes every pointer is not NULL, that '*line' is no more than MAX_LINE_LEN characters long, and that you had a great day. */

/*** ALgorithm explanation: we basically just read the next token from the argument line, make sure that there is another token, and add it to the symbol table as a new symbol (with arbitrary address of 0) using 'new_symb_add_1'. Then, we just need to make sure that the line ends here. */
static short int proc_extern_guide_1(char *line, SymbolTable symb_table, unsigned long int line_num) 
{
	char token[MAX_LINE_LEN + 1]; /* to hold the name of the external symbol */
	
	if(get_token(&line, token) == 0) /* in case the rest of the line is empty */
	{
		printerr(NULL, line_num, FALSE, "An external symbol name was expected in an \".extern\" statement.");
		return INP_ERROR;
	}
	else
	{
		if(new_symb_add_1(symb_table, token, 0, TRUE, FALSE, "in \".extern\" statement", line_num) == INP_ERROR)
			return INP_ERROR; /* we add the new symbol (with address 0, as an exteranal symbol) and check the process - in this case an error occured */
		else
			return line_end_check(&line, "\".extern\" statement expects a single parameter - extraneous text in the end of it was encountered", line_num); /* now we just verify that the line actually ends after the first argument */
	}
}



/* proc_guidance_1 (static function) - this function processes a guidance statement as part of the first scan of an assembly source code.
'arg_line' is assumed to be the rest of the line that follows a guidance statement named 'stmnt_name' (which is assumed to follow a '.' character). 'is_symb_dec' should be TRUE if there is a symbol declaration, and FALSE otherwise. In such case the symbol name is specified by 'symb_name'. The data image of the assembly code is represented by the word list '*data_img_ptr' - and it will be updated only if it is not NULL. The symbol table of the code is 'symb_table'.
Note that if 'data_img_ptr' is NULL, new symbols of the data image will be added with an arbitrary address of 0.
In case of an assembly error was encountered, a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned. In case of an error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, in case the process went successfuly - ALL_GOOD is returned.
Assumes 'arg_line' is no more than MAX_LINE_LEN characters long, and that every pointer (except for 'data_img_ptr') is not NULL. */
static short int proc_guidance_1(char *arg_line, const char *stmnt_name, const char *symb_name, short int is_symb_dec, WordList *data_img_ptr, SymbolTable symb_table, unsigned long int line_num)
{
	short int status; /* to hold the status of the process */
	int guide_num; /* to hold the "enum guide_num' value of the guidance statement */
	
	if(stmnt_name[0] == '\0') /* in case the statement name is empty */
	{
		printerr(NULL, line_num, FALSE, "Expected a guidance statement name after '.' character.");
		status = INP_ERROR;
	}
	else
		switch(guide_num = guide_check(stmnt_name)) /* otherwise, we check its type and act accordingly */
		{
			case g_data: /* in case it is a ".data" statement */
			case g_string: /* OR in case it is a ".string" statement */
				if(is_symb_dec && new_symb_add_1(symb_table, symb_name, data_img_ptr != NULL? data_img_ptr->size : 0, FALSE, TRUE, "while declaring a new symbol", line_num) == INP_ERROR) /* we add the symbol declared (in case there is a symbol declaration) as an internal symbol in the sata image, and check the process. note that the address of it is the current size of the data image - the current address in it; or an arbitrary 0 if 'data_img_ptr' is NULL */
				{
					status = INP_ERROR; /* in such case an error occured */
				}
				else /* otherwise, if a symbol had to be declared - it was declared succesfuly. Now we had to process the line */
				{
					if(guide_num == g_data) /* in case it is a ".data" statement */
						status = proc_data_guide(arg_line, data_img_ptr, line_num);
					else /* otherwise - in case it is a '.string' statement */
						status = proc_string_guide(arg_line, data_img_ptr, line_num);
				}
				break;
			
			case g_extern:
				if(is_symb_dec) /* in case there is a symbol declaration - we print a warning message and ignore it */
					printerr(NULL, line_num, TRUE, "A symbol declaration was encountered in a \".extern\" statement - it is ignored.");
				status = proc_extern_guide_1(arg_line, symb_table, line_num); /* and we process the statement */
				break;
			
			case g_entry: /* in case the statement is ".entry" */
				status = ALL_GOOD; /* we skip it - it will be processed in the second scan */
				break;
			
			default: /* in case the guidance statement's name was not identified */
				printerr(NULL, line_num, FALSE, "Unknown guidance statement \".%s\".", stmnt_name);
				status = INP_ERROR;
				break;
		}
	return status;
}



/* proc_line_1 (static function) - This function processes a statement line in an assembly source code, as part of the first scan of it.
'line' represents the 'line_num' line of an assembly source code - that the name of the file holding it is specified in the 'printerr' function. '*code_img_ptr' represents the code image of the source code; '*data_img_ptr' represents the data img of the source code; and 'symb_table' represents the symbol table of the source code.
This function processes the line and updates the code image, the data image, and the symbol table of it accordingly.
* Note that the memory word of every operand (in case it is an instruction statement) that relies on a symbol is set arbitrarily to 0 - because in the first scan no information about all of the symbols are known yet.
* Note that if 'data_img_ptr' is NULL, it will not be updated, and if 'code_img_ptr' is NULL, it will not be updataed.

In case of an assembly error was encountered, a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned. In case of an error in memory allocation, the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, in case the process went successfuly - ALL_GOOD is returned.
Assumes 'line' is no more than MAX_LINE_LEN characters long (that the length of the line was examined - and it is not too long), and that 'line' is not NULL.  */
static short int proc_line_1(char *line, WordList *code_img_ptr, WordList *data_img_ptr, SymbolTable symb_table, unsigned long int line_num)
{
	char token_1st[MAX_LINE_LEN + 1], token_2nd[MAX_LINE_LEN + 1]; /* to hold the first and the second (optionally) tokens of the line */
	char *key_word_ptr, *symb_name; /* a pointer to the key word of the line (the naim command name - for example 'mov' or '.extern', and a pointer to the name of the symbol declared here */
	short int is_symb_dec; /* a flag: TRUE if the line holds a symbol declaration, and FALSE otherwise */
	unsigned int token_1st_len; /* to hold the length of the first token */
	short int status = UNFINISHED; /* to hold the status of the process */
	
	if(*line == ';' || (token_1st_len = get_token(&line, token_1st)) == 0) /* in case the line starts with ';' (it's a comment) or in case there are no non-white tokens (it's empty). note that we read the first token here as well. */
		status = ALL_GOOD; /* we are not supposed to process the line and no issues can occur */
	else /* in this case it is a command line */
	{
		if(token_1st[token_1st_len-1] == ':') /* in case the first token end with ':' - it is a symbol declaration */
		{
			is_symb_dec = TRUE; /* we set 'is_symb_dec' */
			token_1st[token_1st_len-1] = '\0'; /* we delete the ':' character and are left with the symbol name */
			symb_name = token_1st; /* 'symb_name' will now point the symbol name */
			if(get_token(&line, token_2nd) == 0) /* we read the next token and make sure it is not empty */
			{
				printerr(NULL, line_num, TRUE, "A symbol declaration was encountered in an empty line - it is ignored."); /* we print a warning message */
				status = ALL_GOOD; /* we finish here - the rest of the line is empty */ 
			}
			else
				key_word_ptr = token_2nd; /* in case there is a second token - it should be the key word */
		}
		else /* in case there is not a symbol declaration */
		{
			is_symb_dec = FALSE; /* we set '*is_symb_dec' */
			key_word_ptr = token_1st; /* and the key word is the first token */
		}
	}
	
	if(status == UNFINISHED) /* in case the process is not done yet - which means that the line is a command line that we need to process (the rest of it now is the argument part) */
	{ /* We split the procedure into 3 possible cases according to the key word of the statement */
		if(strcmp(key_word_ptr, ",") == 0) /* in case the key word is a comma: we alert this unusual case separately */
		{
			printerr(NULL, line_num, FALSE, "An unexpected comma was encountered %s.", is_symb_dec? "after a symbol declaration" : "at the beginning of a statement"); /* note that the message is different in case there is a symbol declaration */
			status = INP_ERROR; /* (this case is an error) */
		}
		else if(key_word_ptr[0] == '.') /* in case the key word starts with a '.' character: it is a guidance statement - we process it and store the status */
			status = proc_guidance_1(line, key_word_ptr+1, symb_name, is_symb_dec, data_img_ptr, symb_table, line_num);
		else /* otherwise: it has to be an instruction statement - we process it and store the status */
			status = proc_inst_1(line, key_word_ptr, symb_name, is_symb_dec, code_img_ptr, symb_table, line_num);
	}
	return status;
}

/* run_1st_scan - runs the first scan of the assembly source code found in 'source_file' (which is assumed to be opened for reading).
The code image of the machine code will be stored in '*code_img_ptr' (which is assumed to be initialized to an empty list);
The data image of the machine code will be stored in '*data_img_ptr' (which is assumed to be initialized to an empty list);
The symbol table of the assembly source code will be stored in 'symb_table' (which is assumed to be initialized to an empty symbol table).
In case an error/warning in the code was found, an error/warning message is printed using 'printerr' (and the current file name specified there), and INP_ERROR is returned. In case of error in memory allocation the program shuts down - returning ALLOC_ERR, after printing a proper error message (using 'shut_down_err'). Otherwise, ALL_GOOD is returned.
* Note that the memory word of every operand (in case it is an instruction statement) that relies on a symbol is set arbitrarily to 0 - because in the first scan no information about all of the symbols are known yet.
Assumes every pointer is not NULL. */
short int run_1st_scan(FILE *source_file, WordList *code_img_ptr, WordList *data_img_ptr, SymbolTable symb_table)
{
	char line[MAX_LINE_LEN + 1]; /* to hold each line of the source code in turn */
	unsigned long int line_num = 0; /* to hold the number of the current line */
	short int status = ALL_GOOD; /* to hold the status of the process (currently no errors occured) */
	short int keep_reading = TRUE; /* a flag: TRUE if the end of the file was not encountered (and we should keep reading lines) or FALSE otherwise */
	
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
				printerr(NULL, line_num, FALSE, "Line too long. An assembly line should be no longer than %d characters long.", MAX_LINE_LEN);  /* we print an error message */
				status = INP_ERROR; /* and update the status */
				break;
			
			default: /* in case the line is valid - we process it (note that the case in which 'get_line' returns NULL_ENC is not checked, because 'line' is not NULL) */
				if(status != INP_ERROR) /* in case no input error occured yet */
				{
					if(proc_line_1(line, code_img_ptr, data_img_ptr, symb_table, line_num) == INP_ERROR) /* we procoess the line using 'proc_line_1' and check if an error occured */
						status = INP_ERROR; /* if so, we update the status */
				}
				else /* in case an error occured previously - we call proc_line_1 with NULL and NULL instead of 'code_img_ptr' and 'data_img_ptr' because we do not want to spend time on updating the code and the data image - an error already occured and we have no reason to keep building them. */
					proc_line_1(line, NULL, NULL, symb_table, line_num);
				break;
		}
	}
	
	inc_data(symb_table, code_img_ptr->size + INITIAL_LOAD_ADSS); /* we increase the address that every symbol in the data image represents by the size of the code image (+ the initial loading address) - because the data image comes right after the code image */
	return status;
	
}


