/* as_mem_words.c - 'Assembler Memory Words: Assistance Routines' - functions' implementation
@ written by Nadav Kahlon, May 2020.

This file includes the definitions of the various functions declared in as_mem_words.h - functions regarding the manipulation of memory words of a machine language (represented by the Word data type defined in ./as_core_design/as_core_design.h) in an assembler program, and the management of the different list data types defined in as_mem_words.h.

The functions implemented here are:
- word_to_str - to translate a memory word into a character string hexadecimal form.
- adss_to_str - to trnslate a memory word's address into a character string in decimal form.
- str_to_long - to translate a character string that represents a long integer into numeric form.
- word_list_add - to add a memory word to a word list. 
- clear_word_list - to clear a list of memory words.
- pnt_word_list - to print a list of memory words to a file.
- ext_list_add - to add a an external symbol's memory word address to a list.
- clear_ext_list -  to clear a list of external symbols' addresses
- pnt_ext_list - to print a list of external symbol's memory words to a file.
- form_ext_file - to creat a ".ext" file including information about every external symbol address in the code image of an assembly program.

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "../indicators/indicators.h"
#include "../as_core_design/as_core_design.h"
#include "as_mem_words.h"
#include "../assembler_io/assembler_io.h"


/* word_to_str - translates the memory word represented by the Word variable 'mem_word' into a WORD_BIT_LEN/4 digits long hexadecimal form, and stores the result (including a finishing '\0' character) as a character string in 'str' (which should have enough allocated space to hold the result - at least WORD_BIT_LEN/4 + 1 charcters long). 
Algorithm explanation: we check each 4-bit-field of the word separately using a 4-bit mask, and store the matched hexadecimal digit chracter in 'str' (one at a time). */
void word_to_str(Word mem_word, char *str)
{
	/* hex_digits - a constant static array of every hexadecimal digit character. The index of each character is the value that the character represents (as an hexadecimal digit). */
	static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	static const unsigned long int _4bit_mask = 0xful; /* this value represents a 4 bit mask of a memory word */
	
	short int curr_digit; /* a variable to hold the value of the current hexadecimal digit. */
	short int shift_val; /* this value represents the index of the starting bit of the current 4 bit mask we put on the memory word to check its 4 bit fields one at a time (index 0 represents the least significant bit). In the following loop it is initialised to WORD_BIT_LEN-4 - because we strat in checking the 4 most significant bit of the word. It is decreased by 4 each time to check the next less significant 4 bits - until reaching the least significant 4 bits. */
	for(shift_val = WORD_BIT_LEN-4; shift_val >= 0; shift_val -= 4)
	{
		curr_digit = ((unsigned long)(mem_word & (_4bit_mask << shift_val))) >> shift_val;
		/* explanation: we shift the 4 bit mask to its current position, then we mask the memory word's current 4 bits, and shift them back to the least significant 4 bits position. All shifting is done on unsigned values, so only 0 bits are added. */
		*(str++) = hex_digits[curr_digit]; /* we set the character form of the current digit into str, and increasee it to point the next free character position. */
	}
	*str = '\0'; /* set the finishing character */
}


/* adss_to_str - translate the memory address represent by the Address parameter 'adss' into a ADSS_DEC_LEN digits long decimal form (this constant is defined in ../as_core_design/as_core_design.h), and stores the result (including a finishing '\0' character) as a character string in 'str' (which should have enough allocated space to hold the result - at least ADSS_DEC_LEN + 1 characters long). Any extra digits are discarded.
Algorithm explanation: we check the current least significant character using %10, and add it to its position in 'str'. Then, we discard it using /10. We do this process ADSS_DEC_LEN times. */
void adss_to_str(Address adss, char *str)
{
	int i;
	for(i = ADSS_DEC_LEN-1; i >= 0; i--, adss /= 10) /* scans the number */
		str[i] = '0' + adss % 10ul;  /* and stores each digit in its proper position (adss % 10 is the digit in numeric form, we add it to '0' to get it in character form */
	str[ADSS_DEC_LEN] = '\0'; /* sets a finishing '\0' character */
}


/* str_to_long - tranlsates the number represented in decimal form in the character string 'str' into a long integer, and stores the result in '*res_ptr'. In case the number is too big to represent as a "long int", its modulo is taken. In case of an error *res_ptr is not changed. No extra white characters are allowed in 'str'.
Returns a value that indicates how the process went - NULL_ENC if 'str' or 'res_ptr' is NULL, STR_EMPTY if 'str' represents an empty string, INT_EXP if the charcter string does not represent a valid integer in decimal form, or ALL_GOOD if the process went with no errors (this constants are declared in ../indicators/indicators.h).

Algorithm explanation: we first check for a sign character and store it. Then we scan the rest of the chracter string checking for the absolute value - we check if each charcater is a decimal digit, and increase the result we got so far accordingly. In the end we just apply the sign and we should have the result. */
short int str_to_long(const char *str, long int *res_ptr)
{
	short int sign; /* to store the sign of the result */
	long int result; /* to store the absolute value of the result, before setting it to res_ptr */
	
	if(str == NULL || res_ptr == NULL)
		return NULL_ENC;
	
	if(*str == '\0') /* in case there are no characters at all in 'str' */
		return STR_EMPTY;
		
	if(*str == '-') /* in case there is a negative sign */
	{
		sign = -1; /* we set the sign */
		if((*(++str)) == '\0') /* we increase 'str' to point the rest of the string, and check if it does not end here */
			return INT_EXP; /* in such case, 'str' does not represent a valid integer */
	}
	else /* in that case the sign should be positive 1 */
	{
		sign = 1; /* we set the sign */
		if(*str == '+' && (*(++str)) == '\0')  /* if there is an additional '+' character, we make 'str' point at the rest of the string and check if it does not end here */
		{
			return INT_EXP; /* in such case, 'str' does not represent a valid integer */
		}
	}
	
	for(result = 0; *str != '\0'; str++)
	{
		if(!isdigit(*str)) /* in case we encountered a non-decimal-digit character - 'str' is not an integer */
			return INT_EXP;
		else /* otherwise, we include the next digit into the result */
			result = result * 10 + *str - '0';
	}
	
	*res_ptr = ((long int) sign) * result; /* we finally set the result in '*res_ptr' */
	return ALL_GOOD;
}


/* word_list_add - adds the memory word represented by the 'mem_word' parameter into the end of the list pointed by 'list_ptr'. New memory space is allocated for the new node using the standard library function "malloc".
Returns a vaule that indicates the status of the process: ALLOC_ERR for an error in memory allocation, NULL_ENC if 'list_ptr' is NULL, or ALL_GOOD otherwise. (these constants are defined in indicators.h).
In case of an error, nothing happens to the list. */
short int word_list_add(Word mem_word, WordList *list_ptr)
{
	struct word_node *new_node; /* a variable to hold the address of the new node that we add to the list */
	
	if(list_ptr == NULL)
		return NULL_ENC; /* list_ptr should not be NULL, in such a case it is an error */
	if((new_node = malloc(sizeof(struct word_node))) == NULL) /* allocates space for the new node, and checks for the status of the allocation process */
	{
		return ALLOC_ERR; /* in case "malloc" couldn't allocate new space properly */
	}
	new_node->data = mem_word; /* sets the data field of the new node in the list to be the specified memory word. */
	new_node->next = NULL; /* the new node is going to be the last node in the list, so it should point NULL */
	
	if(list_ptr->size == 0) /* in case the list is empty */
	{
		/* in such case the first and the last node is the new node - the only node in the list */
		list_ptr->head_ptr = new_node;
		list_ptr->tail_ptr = new_node;
	}
	else
	{
		/* otherwise, we let the current tail point our new node, and set the new node to be the last node (the new tail) */
		(list_ptr->tail_ptr)->next = new_node;
		list_ptr->tail_ptr =  new_node;
	}
	
	list_ptr->size ++; /* we managed to add a new node into the list - we increase the size of the list */
	return ALL_GOOD;
}


/* clear_word_list - frees any allocated space that the memory word list pointed by 'wlist_ptr' is holding - "clearing" every memory space that it takes. It also initializes the list back into an empty form - so no illegal pointers will be left.
Assumes that the STRUCTURE of the list was managed ONLY using this interface (it is okay if you chanaged the VALUES of the different nodes in the list (their 'data' field), or used the 'head_ptr' field of the list or the 'next' field of the nodes to get to the different nodes in the list). */
void clear_word_list(WordList *wlist_ptr)
{
	struct word_node *curr, *next; /* 'curr' - a pointer to the current node in the list, 'next' - a pointer to the next node in the list */
	
	curr = wlist_ptr->head_ptr; /* we start by setting the current node to be the first node */
	/* and reinitialize the list to an enpty list: */
	wlist_ptr->head_ptr = NULL;
	wlist_ptr->tail_ptr = NULL;
	wlist_ptr->size = 0; 
	
	while(curr != NULL) /* then we scan the list until its end (when reaching a NULL pointer) - freeing each node in it one at a time */
	{
		next = curr->next; /* before we free the current node, we store the pointer to the next node in 'next' */
		free(curr); /* we free the space we allocated for the current node */
		curr = next; /* and set the current node to be the next one */
	}
}


/* pnt_word_list - prints the memory word list 'wlist' to 'stream' (which is assumed to be opened for writing). Each word is printed in ADSS_DEC_LEN/4 digits long hexadecimal form alogside its address - which is printed in ADSS_DEC_LEN digits decimal form. The address of the first word in the list is 'curr_adss', and every other word in the list is assumed to be 1 address after the previous one.
Returns PNT_ERR if an error occurred while trying to print to 'stream', or ALL_GOOD otherwise (these constants are defined in ../indicators/indicators.h). In case of an error some characters may already have been printed. */
short int pnt_word_list(FILE *stream, WordList wlist, Address curr_adss)
{
	/* we use word_to_str and adss_to_str to translate the addresses and the memory words into character strings (this functions are declared in ../as_mem_words/as_mem_words.h). We need a charcter array to hold the results of these functions, but in order to save space we use the same character array - 'str'. The size of this array is set to be big enough to hold the results of both functions: */
	char str[((ADSS_DEC_LEN > (WORD_BIT_LEN/4))? ADSS_DEC_LEN : (WORD_BIT_LEN/4)) + 1]; /* see explanation above */
	struct word_node *curr_wnode; /* a pointer to the current node in wlist */
	
	for(curr_wnode = wlist.head_ptr; curr_wnode != NULL; curr_wnode = curr_wnode->next, curr_adss++) /* scans 'wlist' */
	{
		adss_to_str(curr_adss, str); /* we translate the current address to the printing format */
		if(fprintf(stream, "%s ", str) < 0) /* we print it and check the result */
			return PNT_ERR; /* in such case an error occured in the process of printing. */
		
		word_to_str(curr_wnode->data, str); /* we translate the current memory word to the printing format */
		if(fprintf(stream, (curr_wnode->next != NULL? "%s\n" : "%s"), str) < 0) /* we print it with a new line character (only if it is not the last word in the list) and check the process. */
		{
			return PNT_ERR; /* in such case an error occured in the process of printing. */
		}
	}
	return ALL_GOOD;
}


/* ext_list_add - adds the external symbol's name 'new_name' and the address 'new_adss' into the beginning of the list pointed by 'list_ptr'. New memory space is allocated for the new node and its name using the standard library function "malloc".
Returns a vaule that indicates the status of the process: ALLOC_ERR for an error in memory allocation, NULL_ENC if 'list_ptr' or 'new_name' is NULL, or ALL_GOOD otherwise. (these constants are defined in indicators.h).
In case of an error, nothing happens to the list. */
short int ext_list_add(char *new_name, Address new_adss, ExtList *list_ptr)
{
	struct ext_node *new_node; /* a pointer to the new node in the list */
	
	if(list_ptr == NULL || new_name == NULL)
		return NULL_ENC; /* list_ptr and ext_symb_name should not be NULL, in such a case it is an error */
	
	/* allocates space for the new node and its name, and checks for the status of the allocation process: */
	if((new_node = malloc(sizeof(struct ext_node))) == NULL || (new_node->ext_symb_name = malloc(strlen(new_name)+1)) == NULL)
		return ALLOC_ERR; /* in case "malloc" couldn't allocate new space properly */
	/* note that the space we allocated for the name is strlen(name) bytes long + space for a a finishing '\0' character - to fit a copy of 'name' */
	
	/* sets the node's fields: */
	strcpy(new_node->ext_symb_name, new_name);
	new_node->adss = new_adss;
	/* and adds it to the list: */
	new_node->next = list_ptr->head_ptr;
	list_ptr->head_ptr = new_node;
	return ALL_GOOD;
}


/* clear_ext_list - frees any allocated space that the external symbol addresses list pointed by 'elist_ptr' is using - "clearing" every memory space that it takes. It also initializes the list back into an empty form - so no illegal pointers will be left.
Assumes that the list was managed ONLY using this interface. */
void clear_ext_list(ExtList *elist_ptr)
{
	struct ext_node *curr, *next; /* 'curr' - a pointer to the current node in the list, 'next' - a pointer to the next node in the list */
	
	curr = elist_ptr->head_ptr; /* we set the current node to be the first node */
	/* and reinitialize the list to be an empty list: */
	elist_ptr->head_ptr = NULL; 
	
	while(curr != NULL) /* we now keep scanning the list until reaching the end (a NULL pointer) - and free each node in it */
	{
		next = curr->next; /* before clearing the node we store the pointer to the next node in 'next' */
		
		free(curr->ext_symb_name); /* we free the space we allocated for the name of the current node */
		free(curr); /* we free the space we allocated for the current node itself */
		
		curr = next; /* and set the current node to be the next one */
	}

}


/* pnt_ext_list - prints the external-symbols'-memory-addresses-list 'elist' to 'stream' (which is assumed to be opened for writing). Each external name is printed alongside its address - which is printed in ADSS_DEC_LEN decimal digits.
Returns PNT_ERR if an error occurred while trying to print to 'stream', or ALL_GOOD otherwise (these constants are defined in ../indicators/indicators.h). In case of an error some characters may already have been printed. */
short int pnt_ext_list(FILE *stream, ExtList elist)
{
	char adss_str[ADSS_DEC_LEN+1]; /* to hold the character string representation of each address */
	struct ext_node *curr_enode; /* a pointer to the current node in elist */
	
	for(curr_enode = elist.head_ptr; curr_enode != NULL; curr_enode = curr_enode->next) /* scans 'elist' */
	{
		if(fprintf(stream, "%s ", curr_enode->ext_symb_name) < 0) /* we print the current external symbol's name and check the process */
		{
			return PNT_ERR; /* in such case an error occured in the process of printing. */
		}
		
		adss_to_str(curr_enode->adss, adss_str); /* we translate the address of the current external symbol memory word to the printing format */
		if(fprintf(stream, (curr_enode->next != NULL? "%s\n" : "%s"), adss_str) < 0) /* we print it with a new line character (only if it is not the last word in the list) and check the process. */
		{
			return PNT_ERR; /* in such case an error occured in the process of printing. */
		}
	}
	return ALL_GOOD;
}


/* form_ext_file - in case ths external symbol addresses list 'elist' is not empty, this function will creat a ".ext" file which will hold the required information about each external symbol address in the list. The file name (including a ".ext" suffix) is specified by 'fname'. Assumes 'fname' is not NULL
In case an error occured while trying to open / creat / close the file, the program shuts down - returning FILE_OERATION_ERR, after printing a proper error message (using 'shut_down_err').
In case an error occured while writing to the file the program shuts down - returning PNT_ERR, after printing a proper error message (using 'shut_down_err').
(* These 2 constants are defined in ../indicators/indicators.h.  * 'shut_doen_err' is declared in ../assembler_io/assembler_io.h). */
void form_ext_file(char *fname, ExtList elist)
{
	FILE *out_stream; /* a FILE pointer to the output file */	
	
	if(elist.head_ptr != NULL) /* if the head of the list is not NULL - it means that the list is not empty */
	{
		if((out_stream = fopen(fname, "w")) == NULL) /* we create the ".ent" file and check the process */
		{
			shut_down_err(FILE_OERATION_ERR, "An error occured while trying to open/create a file named \"%s\"", fname); /* in case an error occured while trying to open the file - we shut down the program using 'shut_down_err' */
		}
		if(pnt_ext_list(out_stream, elist) == PNT_ERR) /* we print the list using 'pnt_ext_list (defined above) and check the status of the process */
		{
			shut_down_err(PNT_ERR, "An error occured while trying to print to file named \"%s\"", fname); /* in such a case an error occured in the printing process - we shut down the program using 'shut_down_err' */
		}
		if(fclose(out_stream) != 0) /* finally, we close the output file and check the status of the process */
		{
			shut_down_err(FILE_OERATION_ERR, "An error occured while trying to close the file named: \"%s\"", fname); /* in this case an error occured - we shut down the program using 'shut_down_err' */
		}
	}
}
