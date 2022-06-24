/* assembler_io.h - 'Assembler Input / Output' - header file
@ written by Nadav Kahlon, May 2020.

This file contains declarations to a variety of function to help you manipulate the input and the output of an assembler program, such as: functions to read input lines / input token from a file / line, functions to print to a file a list of memory words based on a starting address, etc.
All the functions declared here are defined in the adjacent file assembler_io.c.

More about the different routines can be found below.

*/


/* ASSEMBLER_IO_IN - an arbitrary constant that was meant to prevent multiple inclusions of "assembler_io.h". If defined already - it indicates that "assembler_io.h" is already included in a file. */
#ifndef ASSEMBLER_IO_IN
#define ASSEMBLER_IO_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "assembler_io.h" for the first time. */


#include <stdio.h>
#include "../as_core_design/as_core_design.h"
#include "../as_mem_words/as_mem_words.h"



/* ~~~ INPUT READING ~~~ */


/* get_line - reads the next line from 'stream' (which is assumed to be opened for reading) and stores the result (excluding a new-line character) in '*line' (which is assumed to have enough allocated space for it). The line should be no longer than MAX_LINE_LEN characters long (this constant is defined in ../as_core_design/as_core_design.h). The length of the line is stored in '*len_ptr', or discarded if 'len_ptr' is NULL.
Returns NULL_ENC if 'line' is NULL, EOF_ENC if no more characters can be read, STR_TOO_LONG if the line is more than MAX_LINE_LEN characters long (in such case the rest of the line is discarded), or ALL_GOOD otherwise (these constants are defined in ../indicators/indicators.h). */
short int get_line(FILE *stream, char *line, unsigned int *len_ptr);


/*  get_token - gets the next input token from the character string pointed by 'input' onto the character string 'word', while discarding each character read by incrementing '*input'. Returnes the length of the token (in characters), and 0 when reaching the end of the character string. White characters separate each token.
'word' should have enough allocated space to hold the input token. Assumes that every pointer is not NULL.
Notice that a comma (',') represents a 1 character long separated token (","). */
unsigned int get_token(char **input, char *word);



/* ~~~ ERROR/WARNING PRINTING ~~~*/


/* printerr - prints an error/warning (based on the value 'is_warning' - TRUE for a warning / FALSE for an error) message to the standard error file about an error/warning in an assembly source code. The message is entered the way formatted output is entered in the standard library formatted output functions - 'err_msg_fmt' is the format and additional arguments can be added to it.
The name of the file in which the error/warning was encountered is entered as 'new_file_name'. If 'new_file_name' is NULL, the most recently entered file name is used. If no file name was entered at all, the default name is "unknown-file". 'line_num' is the number of the line in which the error/warning was encountered in this file.
If err_msg_fmt is NULL, no message is printed. Such a case can be used to set the file name without printing anything. (any other argument is ignored in this case).
* Note: the 'new_file_name' pointer should point to a character string in an allocated memory space; this function use and store this pointer AS IT IS, it DOES NOT copy it to a seperate character string, in order to be more efficient. */ 
void printerr(const char *new_file_name, unsigned long int line_num, short int is_warning, const char *err_msg_fmt, ...);


/* set_err_fname (macro) - sets the character string pointer that is used by 'printerr' to represent the file name in which errors and warning are found (if so) to be the argument 'new_fname'. If 'new_fname' is NULL, the current name specified there will be cleared, and the name will be set back to its default form: "unknown-file". */
#define set_err_fname(new_fname) ( printerr((new_fname) != NULL ? (new_fname) : "unknown-file", 0, 0, NULL) )
/* The way it is done: we use the printerr function.
The 'err_msg_fmt' parameter is set to NULL - so no message is printed.
The 'new_file_name' parameter of the function is set to be the 'new_fname' argument if it is not NULL, or "unknown-file" otherwise - so if it is not NULL 'printerr' will set the current file name to 'new_fname', otherwise it will be set defaultly to "unknown-file". Every other parameter will be ignored (because the 'err_msg_fmt' parameter is NULL), so an arbitrary 0 value is entered in these places. */


/* shut_down_err - prints a technical error message to the standard error file (in the name of the "assembler" program) and shuts down the assembler program, while returning the value 'err_indicator' (using: exit(indicator)).
The message is entered the way formatted output is entered in the standard library formatted output functions - 'err_detail_fmt' is the format and additional arguments can be added to it.
In addition, after the formatted error message is printed, another error message is printed using the standard library function 'perror' (for more detail).
Assumes 'err_detail_fmt' is not NULL. */
void shut_down_err(int err_indicator, const char *err_detail, ...);



/* ~~~ ASSEMBLER INPUT CHECK FUNCTIONS ~~~ */


/* symb_check - checks if 'symb_name' is a valid symbol name. If so - it returns ALL_GOOD, otherwise - a proper error is printed using 'printerr' (with the current file name specified there, and 'line_num' as the current line index) and INP_ERROR is returned. The 'err_place' character string may be added to the error messages as the place where the valid symbol name was expected to be.
Assumes every pointer is not NULL. */
short int symb_check(const char *symb_name, const char *err_place, unsigned long line_num);


/* comma_check - gets the next token from '*line_ptr' and checks if it is a comma. If so, ALL_GOOD is returned. Otherwise, INP_ERROR is returned. If the end '*line_ptr' is encountered (no tokens left), END_OF_LINE is returned (these values are defined in ../indicators/indicators.h).
If the token is not a comma and 'non_comma_msg' is not NULL - an error message including 'non_comma_msg' is printed. If we reached the end of '*line_ptr' and 'line_end_msg' is not NULL - an error message incluing 'line_end_msg' is printed. Error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes '*line_ptr' is no more than MAX_LINE_LEN characters long, and that 'line_ptr' and '*line_ptr' are not NULL. */
short int comma_check(char **line_ptr, const char *non_comma_msg, const char *line_end_msg, unsigned long int line_num);

/* line_end_check - checks that '*line_ptr' is empty by checking the next token in this line. If so, ALL_GOOD is returned. Otherwise, INP_ERROR is returned and an error message including 'err_msg' is printed. Error messages are printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line).
Assumes '*line_ptr' is no more than MAX_LINE_LEN characters long, and every pointer is not NULL. */
short int line_end_check(char **line_ptr, const char *err_msg, unsigned long int line_num);



/* ~~~ GUIDANCE STATEMENTS' PARAMETERS READING FUNCTIONS ~~~ */


/* get_data_word - reads the next token from '*line_ptr' (which is assumed to be part of the ".data" guidance statement) and makes sure that it is a numeric parameter. If so, 'res_ptr' is set to represent its memory word in the data image and ALL_GOOD is returned, otherwise INP_ERROR is returned.
If the end of the line is encountered, an error message including 'line_end_msg' is printed. If the next token is a comma, an error message including 'at_comma_msg' is printed. And if the next token is simply not a numeric parameter, a proper error message is printed (expecting a numeric parameter in a ".data" statement).
Error messages are printed using 'printerr' (with the current file name specified there, and 'line_num' as the current line index). In case of an error 'res_ptr' is not changed.
Assumes that that all pointers are not NULL, and that '*line_ptr' is no more than MAX_LINE_LEN characters long. */
short int get_data_word(char **line_ptr, Word *res_ptr, const char *at_comma_msg, const char *line_end_msg, unsigned long int line_num);


/* get_char_string - reads the character string 'line' - which is assumed to follow a ".string" guidance statement name, and makes sure that it is a valid assembly character string (white spaces before or after the last '"' characters are ignored). If so, 'res_ptr' is set to represent it (excluding the intial and final '"' characters) and ALL_GOOD is returned. Otherwise - a proper error message is printed using 'printerr' (and the current file name specified there, when 'line_num' is the index of the current line) and INP_ERROR is returned.
Assumes that 'res_ptr' is big enough to hold the WHOLE LINE represented by 'line', and that every pointer is not NULL.
Note that in error cases, 'res_ptr' may already has been changed. */
short int get_char_string(const char *line, char *res_ptr, unsigned long int line_num);



/* ~~~ OBJECT FILE OUTPUT ~~~ */

/* create_ob_file - this function creates a ".ob" file which will hold the required information about the final machine code of an assembly program. The file name (including a ".ob" suffix) is specified by 'fname'. Assumes 'fname' is not NULL.
The code image of the machine code is represented by the memory word list 'code_img', and the data image of the code is represented by the memory word list 'data_img'.
In case an error occured while trying to open / creat / close the file, the program shuts down - returning FILE_OERATION_ERR, after printing a proper error message (using 'shut_down_err').
In case an error occured while writing to the file the program shuts down - returning PNT_ERR, after printing a proper error message (using 'shut_down_err').
(* These 2 constants are defined in ../indicators/indicators.h.  * 'shut_doen_err' is declared in ../assembler_io/assembler_io.h). */
void create_ob_file(char *fname, WordList code_img, WordList data_img);


#endif /* end of the "#ifndef ASSEMBLER_IO_IN" conditional inclusion statement */

