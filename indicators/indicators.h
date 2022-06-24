/* indicators.h - 'Indication Constants' - header file
@ written by Nadav Kahlon, May 2020.

This file contains a variety of constant integer values, which are used by different routines to indicate different states, errors, statuses, etc.
To be clear - each indicator in this file has a unique different value, so you can use constants from different sets in the same context.
Also, you might find it useful to know that every indicator here, except for FALSE, is not 0.

*/


/* INDICATORS_IN - an arbitrary constant that was meant to prevent multiple inclusions of "indicators.h". If defined already - it indicates that "indicators.h" is already included in a file. */
#ifndef INDICATORS_IN
#define INDICATORS_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "indicators.h" for the first time. */



/* 1-bit-length boolean values: */

enum boolean {FALSE, TRUE};
/* TRUE indicates that something is true. (it is non-zero - so it represents a "true" condition)
FALSE indicates that something is false. (it is zero - so it represents a "false" condition)
Note: these values (0 / 1) can represent a 1 bit integer (as a 1 bit-field value for instance). */



/* Process status indicators: */

enum proc_stat {ALL_GOOD = 2, END_OF_LINE, UNFINISHED, INP_ERROR, TECHL_ERR, UNKNOWN_SPEC, ALLOC_ERR, DUP_ERR, NULL_ENC, NUM_LEN_ERR, PNT_ERR, FILE_OERATION_ERR};
/* ALL_GOOD: indicates that some process went with no issues. */
/* END_OF_LINE: indiactes the end of a line (or a line process). */
/* UNFINISHED: indicates that a process is not finished yet */
/* INP_ERROR - input error: indicates a mismatch in the expected form of the input of a program. */
/* TECHL_ERR - technical error: indicates an technical error in a program. */
/* UNKNOWN_SPEC - unknown specification: indicates that an unknown structure was encountered */
/* ALLOC_ERR - allocation error: indicates an error in the process of memory allocation. */
/* DUP_ERR - duplication error: indiactes that an object with the same name is already existing. */
/* NULL_ENC - NULL encountered: indicates that NULL was encountered in place where it shouldn't be. */
/* NUM_LEN_ERR - number length error: indicates that the binary representation of a number is to long. */
/* PNT_ERR - printing error: indicates an error in the process of printing. */
/* FILE_OERATION_ERR - file operation error: indicates that an error occured while trying to manipulate a file */


/* A character string check indicators: */

enum str_check {VALID = 14, INVALID_CHR, ALPHA_EXP, ALNUM_EXP, INT_EXP, NON_NUM, STR_TOO_LONG, STR_EMPTY, EOF_ENC, PRFX_EXP, SFX_EXP};
/* VALID: indicates that a character string is valid. */
/* INVALID_CHR: indicates that a character is invalid. */
/* ALPHA_EXP - alphabetic character expected: indicates that an alphabetic character was expected, but another character was encountered. */
/* ALNUM_EXP - alphabetic character/number expected: indicates that an alphabetic charcater or a decimal digit was expected, but another characater was encountered. */
/* INT_EXP - integer expected - indicates that an integer was expected to be found in a character string. */
/* NON_NUM - non-numeric error: indicates that a character string does not represent a valid number. */
/* STR_TOO_LONG - character string too long: indicates that a character string is too long. */
/* STR_EMPTY - empty character string: indicates that a character string that shouldn't be empty, is empty. */
/* EOF_ENC - EOF encountered: indicates the EOF was encountered. */
/* PRFX_EXP - prefix expected - indicates that a character string is missing an expected prefix. */
/* SFX_EXP - suffix expected - indicates that a character string i missing an expected suffix. */


#endif /* end of the "#ifndef INDICATORS_IN" conditional inclusion statement */
 
