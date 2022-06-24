/* as_core_design.c - 'Assembly Core Design' - functions' implementation
@ written by Nadav Kahlon, May 2020.

This file contains the definitions of the functions declared in as_core_design.h - functions which are meant to help you check the validity of a field in an assembly language.

The functions defined here are:
- find_ass_inst - looks for an assembly instruction based on its name, and gets information about it.
- guide_check - to check if a character string is a guidance statement name (and gets a value that indicates which
	guidance atatement it is).
- is_legal_symb - checks if a character string can represent a valid symbol.
- str_check - to check if a charcater string is a legal in an assembly source code.

*/

#include <string.h>
#include <ctype.h>
#include "as_core_design.h"
#include "../indicators/indicators.h"
                                    
/* find_ass_inst - 'find assembly instruction': checks if the name represented by 'str' is a valid operation name in the asembly language, and returns a pointer to the mathced Instruction variable in the instruction table, or NULL if it was not found.
We scan the table (defined inside the function's definition) and look for a matching name. */
Instruction *find_ass_inst(const char *str)
{
	/* inst_table - the instruction table: a constant data structue which holds every possible instruction in the assembly 		language, and information about it. (implemented as an array) */
	static Instruction inst_table[] = {{"mov", 0, 0, 2, TRUE,TRUE,FALSE,TRUE, FALSE,TRUE,FALSE,TRUE},
	                                    {"cmp", 1, 0, 2, TRUE,TRUE,FALSE,TRUE, TRUE,TRUE,FALSE,TRUE},
	                                    {"add", 2, 1, 2, TRUE,TRUE,FALSE,TRUE, FALSE,TRUE,FALSE,TRUE},
	                                    {"sub", 2, 2, 2, TRUE,TRUE,FALSE,TRUE, FALSE,TRUE,FALSE,TRUE},
	                                    {"lea", 4, 0, 2, FALSE,TRUE,FALSE,FALSE, FALSE,TRUE,FALSE,TRUE},
	                                    {"clr", 5, 1, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,FALSE,TRUE},
	                                    {"not", 5, 2, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,FALSE,TRUE},
	                                    {"inc", 5, 3, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,FALSE,TRUE},
	                                    {"dec", 5, 4, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,FALSE,TRUE},
	                                    {"jmp", 9, 1, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,TRUE,FALSE},
	                                    {"bne", 9, 2, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,TRUE,FALSE},
	                                    {"jsr", 9, 3, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,TRUE,FALSE},
	                                    {"red", 12, 0, 1, FALSE,FALSE,FALSE,FALSE, FALSE,TRUE,FALSE,TRUE},
	                                    {"prn", 13, 0, 1, FALSE,FALSE,FALSE,FALSE, TRUE,TRUE,FALSE,TRUE},
	                                    {"rts", 14, 0, 0, FALSE,FALSE,FALSE,FALSE, FALSE,FALSE,FALSE,FALSE},
	                                    {"stop", 15, 0, 0, FALSE,FALSE,FALSE,FALSE, FALSE,FALSE,FALSE,FALSE}};
	static unsigned const short int INST_TABLE_SIZE = 16; /* The amount of instructions in the instruction table array. */
	
	int i;
	for(i = 0; i < INST_TABLE_SIZE; i++) /* scans the instruction table array */
	{
		if(strcmp(str, (inst_table[i]).op_name) == 0) /* in case the name was found in the table */
			return inst_table + i; /* returns a pointer to the Instruction variable in the table */
	}
	return NULL; /* in case we couldn't natch str to any name in the table - it was not found */
}

/* guide_check - checks if 'str' is a guidance statement name - "data"/"string"/"entry"/"extern" (excluding the '.' character): if so, it returns its enum guide_num value, otherwise - it returns -1. */ 
short int guide_check(const char *str)
{
	/* we check if str matches 1 of the 4 guidance statements name: */
	if(strcmp("data", str) == 0)
		return g_data;
	else if(strcmp("string", str) == 0)
		return g_string;
	else if(strcmp("entry", str) == 0)
		return g_entry;
	else if(strcmp("extern", str) == 0)
		return g_extern;
	
	/* in case no name was matched */
	return -1;
}

/* is_legal_symb - checks if 'str' (which is assumed not to be NULL) is a legal symbol name, and returns a value that indicates the result:
VALID if it is legal, STR_EMPTY if 'str' does not include any characters at all, ALPHA_EXP if the first character is not an alphacetic character, ALNUM_EXP if the first character is not followed only by alphabetic characters or decimal digits, STR_TOO_LONG if 'str' is more than MAX_SYMB_LEN characters long, and DUP_ERR if the 'str' is a reserved word of the assembly language (this constants are defined in ../indicators/indicators.h). */
short int is_legal_symb(const char *str)
{
	int i;
	
	if(str[0] == '\0') /* if the first character is the finishing '\0' character -> str is empty */
		return STR_EMPTY;
	else if(!isalpha(str[0])) /* if the first character is not an alphabetic letter */
		return ALPHA_EXP;
	
	if(is_reserved(str)) /* in case 'str' is already a reserved word of the language */
		return DUP_ERR;
	
	/* we reach here only if the first character is legal - we keep scanning the rest of 'str' */
	for(i = 1; str[i] != '\0' && i < MAX_SYMB_LEN; i++)
	{
		if(!isalnum(str[i])) /* if we found a non-alphabetic-or-digit character */
			return ALNUM_EXP;
	}
	/* if we reached here it is because 1 of 2 reasons: we reached the end and str is valid - in case that str[i] is '\0', or: we reached the maximum symbol name character length - in case that i >= MAX_SYMB_LEN */
	if(str[i] == '\0')
		return VALID;
	else
		return STR_TOO_LONG;
}

/* str_check - checks if 'input' represents a valid input character string, and stores it in 'str_out'. A valid input character string should be a string of printable characters delimited by 2 '"' characters. White spaces before the first '"' or after the last '"' are ignored.
Returns STR_EMPTY if 'input' includes only white characters, PRFX_EXP if 'input' is missing a '"' at the beginning, SFX_EXP if 'input' is missing a '"' at the end, INVALID_CHR if the input string holds a non-printable character, and VALID otherwise (this constants are defined in ../indicators/indicators..h). In case of such an error 'str_out' may already have been changed.
* Notice: it is assumed that 'str_out' has enough allocated space to hold the WHOLE INPUT.
ALgorithm explanation: we scan 'input'. If the first non-white character is not '"', then we are missing a starting '"' character. Unless, we start scanning the rest of the input and storing EVERYTHING in str_out, and check the printability of each character. When reaching a non-white charcater we store its index. At the end, we check (using the index that we stored) if the last non-white character is '"'. If so, we set it to '\0' to finish up the output. If not, we are missing a finishing '"' character. We also keep track of the postion of the first non-printable charcter, to check in the end if it was inside the character string. */
short int str_check(const char *input, char *str_out)
{
	int i;
	int non_white_i = -1; /* to store the index of the last non-white charcater, initialized to -1 to indicate that we haven't found such a character yet in the inside of the input string */
	int non_pnt_i = -1; /* to store the index of the first non-printable character, initialized to -1 to indicate that we haven't found such a character yet */
	
	while(isspace(*input))
		input++; /* skips white characters */
	
	if(*input == '\0')
		return STR_EMPTY; /* in case there are only white spaces in 'input' */
	else if(*input != '"') /* in a case we are missing a starting '"' character */
		return PRFX_EXP;
	
	input++; /* the start of the input character string is after the first '"' */
	for(i = 0; (str_out[i] = *input) != '\0'; i++, input++) /* scans the rest of the string and stores it in str_out */
	{
		if(!isprint(str_out[i]) && non_pnt_i == -1) 
		{
			non_pnt_i = i; /* in such case this is the first time we encounter a non-printable character - we store its index */
		}
		if(!isspace(str_out[i]))
			non_white_i = i; /* we store the index of the last non-white character */
	}
	
	if(non_white_i == -1 || str_out[non_white_i] != '"') /* in case the last non-white character wasn't '"' */
		return SFX_EXP;
	else /* in case the input is finished with a '"' character - positioned in index 'non_white_i' */
	{
		if(non_pnt_i != -1 && non_pnt_i < non_white_i)
			return INVALID_CHR; /* in such a case we encountered a non printable character INSIDE the input string */
		else
		{
			str_out[non_white_i] = '\0'; /* in such case everything is well - we finish the string in the last '"' */
			return VALID;
		}
	}
}

