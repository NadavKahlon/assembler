/* as_core_design.h - 'Assembly Core Design' - header file
@ written by Nadav Kahlon, May 2020.

This file contains different definition and constants related to the fundamental design of an assembly language and the machine's binary language, as well as type definitions and different routines to help you manage the fields of the assembly language in an assembler program: check their validity, and connect them to the machine language.

This file is about creating the fundamental pieces of the languages' design - to gather them together into a "core" file. Other files use this file to create an actual usgae of each of these components separately.

* Note: it is assumed that the computer running the assembler is using the 2's compiment method to represent integers in binary base.
* Note: we might mention the 1-bit integers TRUE and FALSE in this file, note that we refer to the constants TRUE and FALSE, defined in ../indicators/indicators.h.

*/


/* AS_CORE_DESIGN_IN - an arbitrary constant that was meant to prevent multiple inclusions of "as_core_design.h". If defined already - it indicates that "as_core_design.h" is already included in a file. */
#ifndef AS_CORE_DESIGN_IN
#define AS_CORE_DESIGN_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "as_core_design.h" for the first time. */



/* ~~~ INPUT AND OUTPUT DESING ~~~ */

#define MAX_LINE_LEN 80 /* The maximum length (in characters) of an input line in the assembly language (excluding a '\n' character and a '\0' character). */

#define INITIAL_LOAD_ADSS 100 /* Initial Loading Address - the address in which the final machine code starts in (where it is assumed to being loaded at) */


/* ~~~ ADDRESSES ~~~ */

/* Address - represents an address in the machine's memory image */
typedef unsigned long int Address;

#define ADSS_DEC_LEN 7 /* The amount of decimal digits of the address of a memory word that should be printed alongside each memory word in the assembler's output. */


/* ~~~ REGISTERS ~~~ */

/* A register is represented by an 'r' character, followed by another single character between the range of MIN_REG_CHR and MAX_REG_CHR. The number that represents the register's index is the distance between the additional character and MIN_REG_CHAR. */
#define MIN_REG_CHR '0' /* The minimum character value that can follow the 'r' character in a register's name */
#define MAX_REG_CHR '7' /* The maximum character value that can follow the 'r' character in a register's name */
#define REG_BIN_LEN 3 /* The amount of bits that are needed to represent a register's index */

/* reg_check (macro) - checks if a given character string (represented by the argument 'str') is a valid register name in the assembly language: if so, it returns the register's index (a REG_BIN_LEN bits long integer), otherwise - it returns -1. */
#define reg_check(str) ( ((str)[0] == 'r' && (str)[2] == '\0' && (str)[1] >= MIN_REG_CHR && (str)[1] <= MAX_REG_CHR)? ((str)[1] - MIN_REG_CHR) : -1 )
/* (We check if the first character is 'r', which is followed only by 1 character - between MIN_REG_CHR and MAX_REG_CHAR) */


/* ~~~ MEMORY WORD DESIGN ~~~ */

#define WORD_BIT_LEN 24 /* The length of a memory word in the final machine code in binary digits (has to be a positive multiplication of 8, no bigger than 32) */

/* Word - a numeric data type which represents a memory word in the final machine code. (uses the fact that a "long int" is at least 4 bytes long). Only the WORD_BIT_LEN least sagnificant bits of the long integer are used to represent the memory word.  */ 
typedef signed long int Word;

#define WORD_MASK 0xffffffl /* The '1' bits of this value represent the bits that are used in a memory word. The 0's represent the part that is not used. */

#define NON_ARE 0xfffff8 /* The '1' bits of this value represent the bits that are used in the memory word, but not part of the ARE field (in the following set of constants, the ARE field mask is defined. Note that this is not just '~ARE', becasue we also make sure we do not include the bits that are not part of the memory word. */


/* ~~~ SYMBOL DESIGN ~~~ */

#define MAX_SYMB_LEN 31 /* The maximum length (in characters) of a symbol's name in the assembly language (excluding a finishing '\0' charater). */

/* Symbol - represents a symbol in an assembly language. */
typedef struct {
	char *name; /* the symbol's name represented as a character string - the text to be replaced */
	Word rep_word; /* the memory word that should replace the symbol's name */
	unsigned int is_extern : 1; /* a flag bit - TRUE if the symbol is external, and FALSE otherwise */ 
	unsigned int is_entry : 1; /* a flag bit - TRUE if the symbol is set as an entry symbol, and FALSE otherwise */
	unsigned int is_data : 1; /* a flag bit - TRUE if the symbol represent an address in the data image, and FALSE if the symbol represent an adress in the code image */
} Symbol;

/* is_legal_symb - checks if 'str' (which is assumed not to be NULL) is a legal symbol name, and returns a value that indicates the result:
VALID if it is legal, STR_EMPTY if 'str' does not include any characters at all, ALPHA_EXP if the first character is not an alphacetic character, ALNUM_EXP if the first character is not followed only by alphabetic characters or decimal digits, STR_TOO_LONG if 'str' is more than MAX_SYMB_LEN characters long, and DUP_ERR if the 'str' is a reserved word of the assembly language (this constants are defined in ../indicators/indicators.h). */
short int is_legal_symb(const char *str);

/* Note: a valid symbol should start with an alphabetic character, followed by alphabetic charcters or decimal digits. It should be no more than MAX_SYMB_LEN characters long, and cannot be a reserved word of the laguage. */


/* get_symb_adss (macro) - returns the address (of type Address) that the symbol (represented by the Symbol variable 'symb') represents. */
#define get_symb_adss(symbol) ((Address)((((unsigned long int)((symbol).rep_word)) >> 3)))
/* The way it is done: we take the rpelacement word of the symbol and discard its ARE field to get the address that it represents. We do it by converting the replacement word into an unsigned form (to prevent sign extension) and shift it into the right position. Then we convert into Address format and we get the result. */


/* ~~~ INSTRUCTION DESIGN ~~~ */

/* The following definitions are constants regarding the distribution of the different fields in the first memory word of a machine instruction (of type 'long int', or 'Word'). */
/* Each constant represents a field in a binary memory word, using its binary digits: the 1's represent the bits that are part of this field, and the 0's represent the rest of the word. */

#define ARE 0x000007l /* The ARE field - bits 0 through 3. (this is actually a field regarding every other machine instruction word as well) */
#define FUNCT 0x0000f8l /* The function field - bits 3 through 7. */
#define DEST_REG 0x000700l /* The destination register's index - bits 8 through 10. */
#define DEST_ADSS 0x001800l /* The addressing method of the destination operand - bits 11 and 12. */
#define SRC_REG 0x00e000l /* The source register's index - bits 13 through 15. */
#define SRC_ADSS 0x030000l /* The addressing method of the source operand - bits 16 and 17. */
#define OPCODE 0xfc0000l /* The operation code - bits 18 through 23. */

/* ARE_set - three 3-bit-unsigned-integers that indicate how the ARE field of a word should look like (in binary) when A is set to 1, R is set to 1, or E is set to 1 (assuming A is the most sagnificant bit of the field, and E is the least sagnificant bit of the field - and the field is 3 bits long). */
enum ARE_set {ARE_A_set = 4, ARE_R_set = 2, ARE_E_set = 1};

/* adss_method - represents an addressing method of an operand in a machine instruction. The value of each addressing method is the actual 2 bit long numeric value that should appear in the first memory word of a machine instruction to indicate the addressing methd of the source operand and the destination operand. */
enum adss_method {imediate_a, direct_a, relative_a, direct_reg_a};
/* imediate_a - imediate addressing: the operand is a '#' character followed by a number. */
/* direct_a - direct addressing: the operand is a symbol's name. */
/* relative_a - relative addressing: the operand is a '&' character followed by a symbol's name. */
/* a_didirect_reg_arect_reg - direct register addressing: the operand is a register name (an 'r' character followed by a valid register additional character - more about it can be found below). */

/* Instruction - represents a single instruction in an assembly language, and more information about it.  */
/* Note: it is declared as constant because the information about an instruction should not be changed - it should be consistent everywhere, so instruction variables are read-only variables - You cannot change them! */
typedef const struct {
	const char *op_name; /* The name of the operation (constant - so you cannot change the character string itself) */
	unsigned int op_code : 6; /* The operation numeric code (6 bits long). */
	unsigned int funct : 5; /* The function index of the operation (5 bits long). */
	unsigned int opnd_num : 2; /* The number of operands that the instruction should have (max - 2). */
	
	/* Possible addressing methods for the source and destination operands - TRUE/FALSE: */
	
	unsigned int src_imed : 1; /* Can the source operand be addressed using imediate addressing method? */
	unsigned int src_drct : 1; /* Can the source operand be addressed using direct addressing method? */
	unsigned int src_rltv : 1; /* Can the source operand be addressed using relative addressing method? */
	unsigned int src_reg : 1; /* Can the source operand be addressed using direct register addressing method? */
	
	unsigned int des_imed : 1; /* Can the destination operand be addressed using imediate addressing method? */
	unsigned int des_drct : 1; /* Can the destination operand be addressed using direct addressing method? */
	unsigned int des_rltv : 1; /* Can the destination operand be addressed using relative addressing method? */
	unsigned int des_reg : 1; /* Can the destination operand be addressed using direct register addressing method? */
} Instruction;

/* find_ass_inst - 'find assembly instruction': checks if the name represented by 'str' is a valid operation name in the asembly language, and returns a pointer to the mathced Instruction variable in the instruction table (a constant data structure, implemented as an array, which holds information about every instruction in the assembly language), or NULL if it was not found. */
Instruction *find_ass_inst(const char *str);
/* Note: the instruction table and its components are defined privately in the adjacent definition file as_core_design.c. */

/* Operand - represents an operand of an instruction statement. This data type can hold information about operand in each one of the addressing methods - to access information based on the addressing method you should access the right field of the 'info' union. */
typedef struct {
	unsigned int adss_method : 2; /* holds an 'enum adss_method' value that represents the addressing method of this operand. */
	union {
		Word mem_word; /* in case of imediate / relative addressing: the memory word that represents the operand */
		Symbol *symb_ptr; /* in case of direct addressing: a pointer to the symbol in the symbol table */
		unsigned int reg_index : REG_BIN_LEN; /* in case of direct register addressing: the register's index */
	} info; /* holds information about the operand based on the addressing method of it */
} Operand;


/* ~~~ GUIDANCE STATEMENT ~~~ */
/* Note: this file does not include a lot of data about guidance statements, mainly because they are basically about guiding the assembler in different ways. There is not much "flat" information about them. The assembler itself is responsible to translate them into actions properly. */

/* guide_num - different numeric values which represent each one of the guidance statements. */
enum guide_num {g_data, g_string, g_entry, g_extern};

/* guide_check - checks if 'str' is a guidance statement name - "data"/"string"/"entry"/"extern" (excluding the '.' character): if so, it returns its 'enum guide_num' value, otherwise - it returns -1. */ 
short int guide_check(const char *str);


/* ~~~~ RESERVED WORDS OF THE ASSEMBLY LANGUAGE ~~~ */
/* is_reserved (macro) - returns a non-zero integer if a given character string (represented by the argument 'str') is a reserved word of the language, or 0 otherwise. */
#define is_reserved(str) (find_ass_inst((str)) != NULL || guide_check((str)) != -1 || reg_check((str)) != -1)
/* (We check if it is the name of an instruction using find_ass_inst, if it is the name of a guide statement using guide_check, or if it is the name of a register using reg_check) */


/* ~~~ CHARACTER STRINGS ~~~ */

/* str_check - checks if 'input' represents a valid input character string, and stores it in 'str_out'. A valid input character string should be a string of printable characters delimited by 2 '"' characters. White spaces before the first '"' or after the last '"' are ignored.
Returns STR_EMPTY if 'input' includes only white characters, PRFX_EXP if 'input' is missing a '"' at the beginning, SFX_EXP if 'input' is missing a '"' at the end, INVALID_CHR if the input string holds a non-printable character, and VALID otherwise (this constants are defined in ../indicators/indicators..h). In case of such an error 'str_out' may already have been changed.
* Notice: it is assumed that 'str_out' has enough allocated space to hold the WHOLE INPUT. */
short int str_check(const char *input, char *str_out);


#endif /* end of the "#ifndef AS_CORE_DESIGN_IN" conditional inclusion statement */

