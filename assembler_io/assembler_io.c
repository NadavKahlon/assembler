/* assembler_io.h - 'Assembler Input / Output' - functions' implementation
@ written by Nadav Kahlon, May 2020.

This file contains the definitions of the function declared in assembler_io.h - functions that regard most of the parts of the input and the output of an assembler program.

The functions defined here are:
- get_line - to read the next line from a file.
- get_token - to get the next token of a line.
- printerr - for errors/warnings printing.
- shut_down_err - to print technical error messages and shut down the program accordingly.
- symb_check - to check the validity of a symbol name and print a proper error message in error cases.
- comma_check - to make sure that the next token in a line is a comma.
- line_end_check - to make sure that a line was ended.
- get_data_word - to get a parameter of a ".data" statement.
- get_char_string - to get the character string parameter of a ".string" statement. 
- create_ob_file - to creat a ".ob" file including the final machine code of an assembly program.

*/



#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "../indicators/indicators.h"
#include "../as_core_design/as_core_design.h"
#include "../as_mem_words/as_mem_words.h"
#include "../as_symb_table/as_symb_table.h"


/* get_line - reads the next line from 'stream' (which is assumed to be opened for reading) and stores the result (excluding a new-line character) in '*line' (which is assumed to have enough allocated space for it). The line should be no longer than MAX_LINE_LEN characters long (this constant is defined in ../as_core_design/as_core_design.h). The length of the line is stored in '*len_ptr', or discarded if 'len_ptr' is NULL.
Returns NULL_ENC if 'line' is NULL, EOF_ENC if no more characters can be read, STR_TOO_LONG if the line is more than MAX_LINE_LEN characters long (in such case the rest of the line is discarded), or ALL_GOOD otherwise (these constants are defined in ../indicators/indicators.h). */
short int get_line(FILE *stream, char *line, unsigned int *len_ptr)
{
	int ch, i = 0; /* ch - the current character read. i - index used to indicate the current position in *line */
	int limit = MAX_LINE_LEN; /* the amount of characters that we can currently read */
	
	if(line == NULL)
		return NULL_ENC;
	if((ch = fgetc(stream)) == EOF) /*  in case that no more characters can be read from from 'stream' */
		return EOF_ENC;
	else if(ch == '\n') /* in case that the next line is an empty line */
	{
		line[0] = '\0';
	}
	else 
	{
		do {  /* gets the input onto 'line' */
			line[i++] = ch;
		} while(--limit > 0 && (ch = fgetc(stream)) != EOF && ch != '\n');
		line[i] = '\0'; /* to finish up the character string */
	}
	if(len_ptr != NULL)
		*len_ptr = i; /* and we (optionally) store the length of the line. */
	
	if(limit <= 0 && (ch = fgetc(stream)) != EOF && ch != '\n') /* in such case the line is too long - there are stil more characters in the line. Note that the character that is read is NOT be part of the next line - in the worst case it will just be the finishing new-line character. */
	{
		while((ch = fgetc(stream)) != EOF && ch != '\n')
			; /* we keep scanning the line until its end - the rest of the line is discarded */
		return STR_TOO_LONG;
	}
	else
		return ALL_GOOD;
}


/*  get_token - gets the next input token from the character string pointed by 'input' onto the character string 'word', while discarding each character read by incrementing '*input'. The maximum token size is specified by the maximum line length (MAX_LINE_LEN). Returnes the length of the token (in characters), and 0 when reaching the end of the character string. White characters separate each token. 
'word' should be long enough to hold the the input token. Assumes that every pointer is not NULL.
Notice that a comma (',') represents a 1 character long separated token (","). */
unsigned int get_token(char **input, char *word)
{
	unsigned int totch = 0; /* a variable to hold the number of characters read */
	int limit = MAX_LINE_LEN; /* a variable to hold the possible amount of characters left to read */
	
	while(**input != '\0' && isspace(**input))
		(*input)++; /* skips white characters */
		
	if(**input == '\0') /* we reached the end of the input line - an empty token is set */
	{
		word[0] = '\0';
		return 0;
	}
	else if(**input == ',') /* a comma (',') represents a seperate new token with only 1 character (",") */
	{
		(*input)++; /* to indicate that the comma was read */
		word[0] = ',';
		word[1] = '\0';
		return 1;
	}
	
	/* gets the next token */
	while(limit-- > 0 && **input != ',' && !isspace(**input) && **input != '\0')
		word[totch++] = *((*input)++);
	word[totch] = '\0'; /* to finish up the string */ 
	
	return totch;
}


/* printerr - prints an error/warning (based on the value 'is_warning' - TRUE for a warning / FALSE for an error) message to the standard error file about an error/warning in an assembly source code. The message is entered the way formatted output is entered in the standard library formatted output functions - 'err_msg_fmt' is the format and additional arguments can be added to it.
The name of the file in which the error/warning was encountered is entered as 'new_file_name'. If 'new_file_name' is NULL, the most recently entered file name is used. If no file name was entered at all, the default name is "unknown-file". 'line_num' is the number of the line in which the error/warning was encountered in this file.
If err_msg_fmt is NULL, no message is printed. Such a case can be used to set the file name without printing anything. (any other argument is ignored in this case).
* Note: the 'new_file_name' pointer should point to a character string in an allocated memory space; this function use and store this pointer AS IT IS, it DOES NOT copy it to a seperate character string, in order to be more efficient. */ 

/* Algorithm explanation: we store the file name in the static variable 'curr_file_name' - initialized to "unknown-file". If 'new_file_name' is not NULL, we update 'curr_file_name' to represent it. If 'err_msg_fmt' is not NULL, we print an error message using the format and the rest of the arguments (using the standard library function 'vfprintf' - after initializing a variable to point the first unnamed argument). 'is_warning' is checked to match a proper error/warning message. */
void printerr(const char *new_file_name, unsigned long int line_num, short int is_warning, const char *err_msg_fmt, ...)
{
	static const char *curr_file_name = "unknown-file"; /* to hold the name of the current file that the errors were found at (if the name was not set, the default name is "unknown-file") */
	va_list args; /* to point to each unnamed argument in turn */
	
	if(new_file_name != NULL) /* in case the user is willing to change the file name */
		curr_file_name = new_file_name;
	if(err_msg_fmt != NULL) /* in case the user is willing to print an error message */
	{	
		fprintf(stderr, "\t%s - in assembly source file '%s', at line %lu:\n", (is_warning? "WARNING" : "ERROR"), curr_file_name, line_num); /* prints the start of the error/warning message - the file and the number of the line  in which the error was found */
		
		va_start(args, err_msg_fmt); /* we make 'args' point the first unnamed argument */
		vfprintf(stderr, err_msg_fmt, args); /* and print the error message that was entered using vfprintf */
		va_end(args);
		fprintf(stderr, "\n\n"); /* we seperate error messages */
	}
}


/* shut_down_err - prints a technical error message to the standard error file (in the name of the "assembler" program named) and shuts down the assembler program, while returning the value 'err_indicator' (using: exit(indicator)).
The message is entered the way formatted output is entered in the standard library formatted output functions - 'err_detail_fmt' is the format and additional arguments can be added to it.
In addition, after the formatted error message is printed, another error message is printed using the standard library function 'perror' (for more detail).
Assumes 'err_detail_fmt' is not NULL. */
void shut_down_err(int err_indicator, const char *err_detail_fmt, ...)
{
	va_list args; /* to point to each unnamed argument in turn */
	
	/* we print the beginning of the error message: */
	fprintf(stderr, "A technical error occured in your computer while running the assembler program:\n");
	
	va_start(args, err_detail_fmt); /* we set 'args' to point the first unnamed argument */
	vfprintf(stderr, err_detail_fmt, args); /* and print the error message that was entered using vfprintf */
	va_end(args);
	fprintf(stderr, "\n"); /* and go to a new line */
	
	perror("assembler"); /* finally, we print another error message (with more detail) using 'perror' */
	exit(err_indicator); /* and shut down the program returning 'err_indicator' */
}



/* symb_check - checks if 'symb_name' is a valid symbol name. If so - it returns ALL_GOOD, otherwise - a proper error is printed using 'printerr' (with the current file name specified there, and 'line_num' as the current line index) and INP_ERROR is returned. The 'err_place' character string may be added to the error messages as the place where the valid symbol name was expected to be.
Assumes every pointer is not NULL. */

/*** Algorithm explanation: we basically just use the 'is_legal_symb' function declared in ../as_core_design/as_core_design.h to check the validity of the symbol name and print a proper message based on the status of the process. */
short int symb_check(const char *symb_name, const char *err_place, unsigned long line_num)
{
	switch(is_legal_symb(symb_name)) /* we check the validity of 'str' */
	{
		case STR_EMPTY: /* in case the name entered is empty */
			printerr(NULL, line_num, FALSE, "Expected a symbol name %s, but no characters were found.", err_place);
			return INP_ERROR; 
		case ALPHA_EXP: /* in case the first character of the name is not alphabetic */
			printerr(NULL, line_num, FALSE, "Invalid symbol name '%s' was found %s. A symbol name should start with an alphabetic character.", symb_name, err_place);
			return INP_ERROR;
		case ALNUM_EXP: /* in case the name has a character that is non-alphabetic and not a decimal digit */
			printerr(NULL, line_num, FALSE, "Invalid symbol name '%s' was found %s. A symbol name has to include only alphabetic characters or decimal digits.", symb_name, err_place);
			return INP_ERROR;
		case STR_TOO_LONG: /* in case the name is too long */
			printerr(NULL, line_num, FALSE, "The symbol name '%s' that was found %s is too long.\nA symbol name should be no longer than %d characters long.", symb_name, err_place, MAX_SYMB_LEN);
			return INP_ERROR;
		case DUP_ERR: /* in case the name entered is a reserved word of the language */
			printerr(NULL, line_num, FALSE, "Invalid symbol name '%s' was found %s.\nA symbol name cannot be a reserved word of the assembly language.", symb_name, err_place);
			return INP_ERROR;
	}
	return ALL_GOOD; /* if the status of the validity of the symbol is not one of the values above, it has to be 'VALID' - in such case the symbol name is valid and  we can return ALL_GOOD without printing any error message. */
}



/* comma_check - gets the next token from '*line_ptr' and checks if it is a comma. If so, ALL_GOOD is returned. Otherwise, INP_ERROR is returned. If the end '*line_ptr' is encountered (no tokens left), END_OF_LINE is returned (these values are defined in ../indicators/indicators.h).
If the token is not a comma and 'non_comma_msg' is not NULL - an error message including 'non_comma_msg' is printed. If we reached the end of '*line_ptr' and 'line_end_msg' is not NULL - an error message incluing 'line_end_msg' is printed. Error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes '*line_ptr' is no more than MAX_LINE_LEN characters long, and that 'line_ptr' and '*line_ptr' are not NULL. */

/*** Algorithm explanation: we basically just read the next token from the line using 'get_token' and make sure that it is a comma. If the end of line was encountered or the next token is not a comma, we check the error message that was entered and print an appropriate message accordingly. */
short int comma_check(char **line_ptr, const char *non_comma_msg, const char *line_end_msg, unsigned long int line_num)
{
	char token[MAX_LINE_LEN + 1]; /* to hold the next token from '*line_ptr' */
	
	if(get_token(line_ptr, token) == 0) /* gets the next token from '*line_ptr' onto 'token' and checks if the end of the line was ecountered */
	{
		if(line_end_msg != NULL) /* if the end of the line is encountered and 'line_end_message is not NULL */
			printerr(NULL, line_num, FALSE, "%s", line_end_msg); /* we print an error message */
		return END_OF_LINE; /* and return END_OF_LINE */
	}
	else if (strcmp(token, ",") != 0) /* in case the next token is not a comma */
	{
		if(non_comma_msg != NULL)
			printerr(NULL, line_num, FALSE, "%s (before '%s' token).", non_comma_msg, token); /* we print an error message */
		return INP_ERROR; /* and return INP_ERROR */
	}
	else
		return ALL_GOOD; /* in this case the next token is indeed a comma - the reading went successfully. */
}



/* line_end_check - checks that '*line_ptr' is empty by checking the next token in this line. If so, ALL_GOOD is returned. Otherwise, INP_ERROR is returned and an error message including 'err_msg' is printed. Error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes '*line_ptr' is no more than MAX_LINE_LEN characters long, and every pointer is not NULL. */

/*** Algorithm explanation: we basically just read the next token fron the line using 'get_token' and make sure that it is the end of the line (that the length of the next token is '0'). */
short int line_end_check(char **line_ptr, const char *err_msg, unsigned long int line_num)
{
	char token[MAX_LINE_LEN + 1]; /* to hold the next token from '*line_ptr' */
	
	if(get_token(line_ptr, token) != 0) /* in case the length of the next token is not NULL - there is more tokens to read */
	{
		printerr(NULL, line_num, FALSE, "%s (starting in '%s' token).", err_msg, token);
		return INP_ERROR;
	}
	else
		return ALL_GOOD;
}




/* get_data_word - reads the next token from '*line_ptr' (which is assumed to be part of the ".data" guidance statement) and makes sure that it is a numeric parameter. If so, 'res_ptr' is set to represent its memory word in the data image and ALL_GOOD is returned, otherwise INP_ERROR is returned.
If the end of the line is encountered, an error message including 'line_end_msg' is printed. If the next token is a comma, an error message including 'at_comma_msg' is printed. And if the next token is simply not a numeric parameter, a proper error message is printed (expecting a numeric parameter in a ".data" statement).
Error messages are printed using 'printerr' (with the current file name specified there, and 'line_num' as the current line index). In case of an error 'res_ptr' is not changed.
Assumes that that all pointers are not NULL, and that '*line_ptr' is no more than MAX_LINE_LEN characters long. */

/*** ALgorithm explanation: we read the next token using the 'get_token' function, verify that it is not the end of the line or a comma (in these cases we print the corresponding error message), and convert it into numeric memory word format using the 'str_to_long' function and the 'long_to_s24b' macro that are found in ../as_mem_words/as_mem_words.h. */
short int get_data_word(char **line_ptr, Word *res_ptr, const char *at_comma_msg, const char *line_end_msg, unsigned long int line_num)
{
	char token[MAX_LINE_LEN + 1]; /* to hold the next token from '*line_ptr' */
	long int long_res; /* to hold the numeric value of the parameter */
	short int status; /* to hold the status of the process */

	if(get_token(line_ptr, token) == 0) /* in case the end of the line is encountered (no more tokens are left) */
	{
		printerr(NULL, line_num, FALSE, "%s", line_end_msg); /* we print an error message including 'line_end_msg' */
		status = INP_ERROR;
	}
	else if(strcmp(token, ",") == 0) /* in case the next token is a comma */
	{
		printerr(NULL, line_num, FALSE, "%s", at_comma_msg); /* we print an error message including 'at_comma_msg' */
		status = INP_ERROR; 
	}
	else /* in case the next token is valid token */
	{
		if(str_to_long(token, &long_res) == INT_EXP) /* we convert the token into long integer form and check the process */
		{
			printerr(NULL, line_num, FALSE, "Every operand of the \".data\" guidance statement is expected to be a numeric decimal value, however - '%s' is not a deciaml number.", token); /* in case the token is not numeric - we print an error message */
			status = INP_ERROR;
		}
		else /* in this case the convertion went with no issues (because 'token' and '&long_res' are not NULL for sure, and 'token' does not represent an empty string - we already checked that there are more tokens to read) */
		{
			*res_ptr = long_to_s24b(long_res); /* we set the memory word to represent it (in 24 bit form). */
			status = ALL_GOOD;
		} /* note that the case in which 'str_to_long' returns NULL_ENC is not possible, because 'token' and '&long_res' are not NULLs */
	}
	return status;
}





/* get_char_string - reads the character string 'line' - which is assumed to follow a ".string" guidance statement name, and makes sure that it is a valid assembly character string (white spaces before or after the last '"' characters are ignored). If so, 'res_ptr' is set to represent it (excluding the intial and final '"' characters) and ALL_GOOD is returned. Otherwise - a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned.
Assumes that 'res_ptr' is big enough to hold the WHOLE LINE represented by 'line', and that every pointer is not NULL.
Note that in error cases, 'res_ptr' may already has been changed. */

/*** Algorithm explanataion: we use the 'str_check' function declared in ../as_core_design/as_core_design.h to transform the line into an assembly character string. We check the status of the process - looking for errors. We print an error message, unless the the string is valid. */
short int get_char_string(const char *line, char *res_ptr, unsigned long int line_num)
{
	short int status; /* to hold the status of the process */
	switch(status = str_check(line, res_ptr)) /* we check 'line' using 'str_check' and determine the status of the process */
	{
		case STR_EMPTY: /* in case no tokens were entered at all */
			printerr(NULL, line_num, FALSE, "Expected a character string after the \".string\" guidance statement name.");
			break;
			
		case PRFX_EXP: /* in case the character string is missing an initial '"' character */
			printerr(NULL, line_num, FALSE, "Invalid character string was entered in a \".string\" guidance statement:\nAn initial double quotes character ('\"') is missing.");
			break;
			
		case SFX_EXP: /* in case the character string is missing a finishing '"' character */
			printerr(NULL, line_num, FALSE, "Invalid character string was entered in a \".string\" guidance statement:\nA finishing double quotes character ('\"') is missing.");
			break;
			
		case INVALID_CHR: /* in case the charcater string holds a non-printable character */
			printerr(NULL, line_num, FALSE, "Invalid character string was entered in a \".string\" guidance statement:\nA valid character string should include printable characters only.");
			break;
			
		default: /* in this case no errors occured - VALID was returned from 'str_check' */
			break;
	}
	
	return status == VALID? ALL_GOOD : INP_ERROR; /* and we return a value according to the validity of the character string */
}





/* create_ob_file - this function creates a ".ob" file which will hold the required information about the final machine code of an assembly program. The file name (including a ".ob" suffix) is specified by 'fname'. Assumes 'fname' is not NULL.
The code image of the machine code is represented by the memory word list 'code_img', and the data image of the code is represented by the memory word list 'data_img'.
In case an error occured while trying to open / creat / close the file, the program shuts down - returning FILE_OERATION_ERR, after printing a proper error message (using 'shut_down_err').
In case an error occured while writing to the file the program shuts down - returning PNT_ERR, after printing a proper error message (using 'shut_down_err').
(* These 2 constants are defined in ../indicators/indicators.h.  * 'shut_doen_err' is declared in ../assembler_io/assembler_io.h). */
/*** Algorithm explanation: we first open the file, and print the header line of the file - including the size of the code image and the size of the code image (specified by the 'size' field of them). Then we use 'pnt_word_list' (declared in ../as_mem_words/as_mem_words.h) to print the code and the data image (separated by a new-line character). Finally, we close the file. */
void create_ob_file(char *fname, WordList code_img, WordList data_img)
{
	FILE *out_stream; /* a FILE pointer to the output file */	
	
	if((out_stream = fopen(fname, "w")) == NULL) /* we create the ".ob" file and check the process */
	{
		shut_down_err(FILE_OERATION_ERR, "An error occured while trying to open/create a file named \"%s\"", fname); /* in case an error occured while trying to open the file - we shut down the program using 'shut_down_err' */
	}
	if(fprintf(out_stream, "%lu %lu\n", code_img.size, data_img.size) < 0) /* we print the sizes of the images and check: */
	{
		shut_down_err(PNT_ERR, "An error occured while trying to print to file named \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
	}
	if(pnt_word_list(out_stream, code_img, INITIAL_LOAD_ADSS) == PNT_ERR) /* we print the code image (the initial address is the initial program loading address as defined in ../as_core_design/as_core_design.h) and check the process: */
	{
		shut_down_err(PNT_ERR, "An error occured while trying to print to file named \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
	}
	if(fprintf(out_stream, "\n") < 0) /* we print a new-line character and check the process */
	{
		shut_down_err(PNT_ERR, "An error occured while trying to print to file named \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
	}
	if(pnt_word_list(out_stream, data_img, INITIAL_LOAD_ADSS + code_img.size) == PNT_ERR) /* we print the data image (the initial address is the initial program loading address + the size of the code image - so it follows directly the code image) and check the process */
	{
		shut_down_err(PNT_ERR, "An error occured while trying to print to file named \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
	}
	if(fclose(out_stream) != 0) /* finally, we close the output file and check the status of the process */
	{
		shut_down_err(FILE_OERATION_ERR, "An error occured while trying to close the file named: \"%s\"", fname); /* in this case an error occured - we shut down the program using 'shut_down_err' */
	}
}
