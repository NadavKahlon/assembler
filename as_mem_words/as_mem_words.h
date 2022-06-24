/* as_mem_words.h - 'Assembler Memory Words: Assistance Routines' - header file
@ written by Nadav Kahlon, May 2020.

This interface includes declarations to different routines (functions and macros) to help you manipulate memory words and their addresses in an assembler program - such as different routines to set the different fields in an insturcion memory word, set a numeric value into a memory word (using the 2's compliment method - the method used by the machine language), convert a memory word into a character string in hexadecimal form or a memory address into a character string in decimal form, etc.

You can also find here the "WordList" data type which represents a dynamic linear data structure which holds memory words and the "ExtList" data type which represents a dynamic linear data structure which holds addresses of external symbols in the code image of the assembler (both implemented as linked lists - in which the "word_node" structure defined here represents a node in a WordList list, and the "ext_node" structure defined here represents a node in the list of external symbols' memory addresses).


More about the different routines can be found below.

* Note: it is assumed that the computer running the assembler is using the 2's compiment method to represent integers in binary base.

*/


/* AS_MEM_WORDS_IN - an arbitrary constant that was meant to prevent multiple inclusions of "as_mem_words.h". If defined already - it indicates that "as_mem_words.h" is already included in a file. */
#ifndef AS_MEM_WORDS_IN
#define AS_MEM_WORDS_IN 1 /* if it is not defined yet, we define it (with an arbitrary value of 1) and include the contents of "as_mem_words.h" for the first time. */


#include <stdio.h>
#include "../as_core_design/as_core_design.h"



/* ~~~ CHARACTER STRING TRANSLATION FUNCTIONS ~~~ */

/* word_to_str - translates the memory word represented by the Word variable 'mem_word' into a WORD_BIT_LEN/4 digits long hexadecimal form, and stores the result (including a finishing '\0' character) as a character string in 'str' (which is assumed to point to enough allocated space to hold the result - at least WORD_BIT_LEN/4 + 1 characters long). */
void word_to_str(Word mem_word, char *str);

/* adss_to_str - translate the memory address represent by the Address parameter 'adss' into a ADSS_DEC_LEN digits long decimal form (this constant is defined in ../as_core_design/as_core_design.h), and stores the result (including a finishing '\0' character) as a character string in 'str' (which is assumed to point to enough allocated space to hold the result - at least ADSS_DEC_LEN + 1 characters long). Any extra digits are discarded. */
void adss_to_str(Address adss, char *str);

/* str_to_long - tranlsates the number represented in decimal form in the character string 'str' into a long integer, and stores the result in '*res_ptr'. In case the number is too big to represent as a "long int", its modulo is taken. In case of an error *res_ptr is not changed. No extra white characters are allowed in 'str'.
Returns a value that indicates how the process went - NULL_ENC if 'str' or 'res_ptr' is NULL, STR_EMPTY if 'str' represents an empty string, INT_EXP if the charcter string does not represent a valid integer in decimal form, or ALL_GOOD if the process went with no errors (this constants are declared in ../indicators/indicators.h). */
short int str_to_long(const char *str, long int *res_ptr);



/* ~~~ WORD FIELDS AND NUMERIC CONVERSIONS ~~~ */

/* set_word_field (macro) - sets a binary field of the memory word represented by the Word parameter 'mem_word', to be the value entered as the 'set_val' argument, and returns the new value of the memory word.
The starting bit of the field is represented by the unsigned integer argument 'st_bit' (where 0 represents the least significant bit, 1 represents the second least significant bit, and so on).
The first argument 'field_mask' is a long integer value that represents the bits which are included in this field (its 1 bits are part of the field, and the 0's are not). Note that we filter 'set_val' using 'field_mask' (after shifting it to the proper position in the word) in order to take only the bits that we need. */
#define set_word_field(field_mask, mem_word, set_val, st_bit) ( (mem_word) = ((mem_word) & (~(field_mask))) | ((((Word)(set_val)) << (st_bit)) & (field_mask)) )
/* The way it is done: we first set the field's bits in the memory word to 0 (by the (mem_word) & (~(field_mask)) statement). Then we shift the new value to the field's position (using the 'st_bit') and mask it to make sure we do not take any other bits from it. And finally, we set the new value into the memory word (using the | operator). */


/* long_to_s21b (macro) - translate the first argument ('num') into a signed 21 bit integer, and returns the result. In case the binary representation of the first argument is too long we take the modulo 2^20 of it (not 2^21, becasue we keep the sign bit) */
#define long_to_s21b(num) ((num) % 0x100000l) /* We take only the 21 bits of the value by using modulo 2^20, the last bit will be kept as the sign bit by the modulo operator. */


/* long_to_s24b (macro) - translate the first argument ('num') into a signed 24 bit integer, and returns the result. In case the binary representation of argument is too long we take the modulo 2^23 of it (not 2^24, becasue we keep the sign bit) */
#define long_to_s24b(num) ((num) % 0x800000l) /* We take only the 24 bits of the value by using modulo 2^23, the last bit will be kept as the sign bit by the modulo operator


char_to_word (macro) - converts the charcater entered as the argument 'ch' into the memory word value (of type Word) that represents the ASCII value of it, and returns the result. */
#define char_to_word(ch) ((Word)((unsigned char)(ch)))
/* The way it is done: we basically just convert the numeric value of 'ch' to the 'Word' numeric type using a cast. We make sure to cast 'ch' to an unsigned form to prevent sign extension when casting it again into Word (which is, in the current case, the 'signed long int' data type - as defined in ../as_core_design/as_core_design.h). */



/* ~~~ MEMORY WORD LISTS ~~~ */

/* struct word_node - represents a single node in linked list of memory words. */
struct word_node {
	Word data; /* The value of the memory word in the current node. */
	struct word_node *next; /* A pointer to the next node in the list. */
};


/* WordList - represents a linked list of memory words - values of the Word data type. */
typedef struct {
	struct word_node *head_ptr; /* A pointer to the first node in the list. */
	struct word_node *tail_ptr; /* A pointer to the last node in the list. */
	unsigned long int size; /* A variable to hold the number of elements in the list. */
} WordList;
/* Note: the last node of the list always should point NULL. */


/* NEW_WORD_LIST - a constant that represents a new an empty list. It is used to initialize WordList variables properly. */
#define NEW_WORD_LIST {NULL, NULL, 0} /* (the first and the last nodes does not exist - so the tail and head pointers are set to NULL, and there are 0 elements in the list - so the size is set to 0) */


/* word_list_add - adds the memory word represented by the 'mem_word' parameter into the end of the list pointed by 'list_ptr'. New memory space is allocated for the new node using the standard library function "malloc".
Returns a vaule that indicates the status of the process: ALLOC_ERR for an error in memory allocation, NULL_ENC if 'list_ptr' is NULL, or ALL_GOOD otherwise. (these constants are defined in indicators.h).
In case of an error, nothing happens to the list. */
short int word_list_add(Word mem_word, WordList *list_ptr);


/* clear_word_list - frees any allocated space that the memory word list pointed by 'wlist_ptr' (which is assumed not to be NULL) is holding - "clearing" every memory space that it takes. It also initializes the list back into an empty form - so no illegal pointers will be left.
Assumes that the STRUCTURE of the list was managed ONLY using this interface (it is okay if you chanaged the VALUES of the different nodes in the list (their 'data' field), or used the 'head_ptr' field of the list or the 'next' field of the nodes to get to the different nodes in the list). */
void clear_word_list(WordList *wlist_ptr);


/* pnt_word_list - prints the memory word list 'wlist' to 'stream' (which is assumed to be opened for writing). Each word is printed in ADSS_DEC_LEN/4 digits long hexadecimal form alogside its address - which is printed in ADSS_DEC_LEN digits long decimal form. The address of the first word in the list is 'curr_adss', and every other word in the list is assumed to be 1 address after the previous one.
Returns PNT_ERR if an error occurred while trying to print to 'stream', or ALL_GOOD otherwise (these constants are defined in ../indicators/indicators.h). In case of an error some characters may already have been printed. */
short int pnt_word_list(FILE *stream, WordList wlist, Address curr_adss);



/* ~~~ EXTERNAL SYMBOLS' MEMORY ADDRESSES LISTS ~~~ */


/* struct ext_node - represents a single node in linked list of external symbols' memory addresses. */
struct ext_node {
	char *ext_symb_name; /* The name of the external symbol that is represented by this node */
	Address adss; /* The external symbol's address in the code image (in the assembler's output) */
	struct ext_node *next; /* A pointer to the next node in the list. */
};


/* ExtList - represents a linked list of external symbols' memory addresses */
typedef struct {
	struct ext_node *head_ptr; /* A pointer to the first node in the list. */
} ExtList;
/* Note: the last node of the list always should point NULL. */


/* NEW_WORD_LIST - a constant that represents a new an empty list. It is used to initialize ExtList variables properly. */
#define NEW_EXT_LIST {NULL} /* (the list is empty so the head pointer is set to NULL) */


/* ext_list_add - adds the external symbol's name 'new_name' and the address 'new_adss' into the beginning of the list pointed by 'list_ptr'. New memory space is allocated for the new node and its name using the standard library function "malloc".
Returns a vaule that indicates the status of the process: ALLOC_ERR for an error in memory allocation, NULL_ENC if 'list_ptr' or 'new_name' is NULL, or ALL_GOOD otherwise. (these constants are defined in indicators.h).
In case of an error, nothing happens to the list. */
short int ext_list_add(char *new_name, Address new_adss, ExtList *list_ptr);


/* clear_ext_list - frees any allocated space that the external symbol addresses list pointed by 'elist_ptr' is using - "clearing" every memory space that it takes. It also initializes the list back into an empty form - so no illegal pointers will be left.
Assumes that 'elist_ptr' is not NULL, and that the list was managed ONLY using this interface. */
void clear_ext_list(ExtList *elist_ptr);


/* pnt_ext_list - prints the external-symbols'-memory-addresses-list 'elist' to 'stream' (which is assumed to be opened for writing). Each external name is printed alongside its address - which is printed in ADSS_DEC_LEN decimal digits.
Returns PNT_ERR if an error occurred while trying to print to 'stream', or ALL_GOOD otherwise (these constants are defined in ../indicators/indicators.h). In case of an error some characters may already have been printed. */
short int pnt_ext_list(FILE *stream, ExtList elist);


/* form_ext_file - in case ths external symbol addresses list 'elist' is not empty, this function will creat a ".ext" file which will hold the required information about each external symbol address in the list. The file name (including a ".ext" suffix) is specified by 'fname'. Assumes 'fname' is not NULL
In case an error occured while trying to open / creat / close the file, the program shuts down - returning FILE_OERATION_ERR, after printing a proper error message (using 'shut_down_err').
In case an error occured while writing to the file the program shuts down - returning PNT_ERR, after printing a proper error message (using 'shut_down_err').
(* These 2 constants are defined in ../indicators/indicators.h.  * 'shut_doen_err' is declared in ../assembler_io/assembler_io.h). */
void form_ext_file(char *fname, ExtList elist);


#endif /* end of the "#ifndef AS_MEM_WORDS_IN" conditional inclusion statement */

