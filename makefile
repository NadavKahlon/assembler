# This is the makefile of the assembly program.
# @ written by Nadav Kahlon, August 2020.

assembler: assembler/assembler.o as_1st_scan/as_1st_scan.o as_2nd_scan/as_2nd_scan.o as_core_design/as_core_design.o as_mem_words/as_mem_words.o assembler_io/assembler_io.o as_symb_table/as_symb_table.o
	gcc -g -Wall -ansi -pedantic assembler/assembler.o as_1st_scan/as_1st_scan.o as_2nd_scan/as_2nd_scan.o as_core_design/as_core_design.o as_mem_words/as_mem_words.o assembler_io/assembler_io.o as_symb_table/as_symb_table.o -o assembler/assembler

assembler.o: assembler/assembler.c indicators/indicators.h as_core_design/as_core_design.h as_mem_words/as_mem_words.h as_symb_table/as_symb_table.h assembler_io/assembler_io.h as_1st_scan/as_1st_scan.h as_2nd_scan/as_2nd_scan.h
	gcc -c -Wall -ansi -pedantic assembler/assembler.c -o assembler/assembler.o

as_1st_scan.o: as_1st_scan/as_1st_scan.c indicators/indicators.h as_core_design/as_core_design.h as_mem_words/as_mem_words.h as_symb_table/as_symb_table.h assembler_io/assembler_io.h
	gcc -c -Wall -ansi -pedantic as_1st_scan/as_1st_scan.c -o as_1st_scan/as_1st_scan.o
	
as_2nd_scan.o: as_2nd_scan/as_2nd_scan.c indicators/indicators.h as_core_design/as_core_design.h as_mem_words/as_mem_words.h as_symb_table/as_symb_table.h assembler_io/assembler_io.h
	gcc -c -Wall -ansi -pedantic as_2nd_scan/as_2nd_scan.c -o as_2nd_scan/as_2nd_scan.o

as_core_design.o: as_core_design/as_core_design.c as_core_design/as_core_design.h indicators/indicators.h
	gcc -c -Wall -ansi -pedantic as_core_design/as_core_design.c -o as_core_design/as_core_design.o
	
as_mem_words.o: as_mem_words/as_mem_words.c indicators/indicators.h as_core_design/as_core_design.h as_mem_words/as_mem_words.h assembler_io/assembler_io.h
	gcc -c -Wall -ansi -pedantic as_mem_words/as_mem_words.c -o as_mem_words/as_mem_words.o
	
assembler_io.o: assembler_io/assembler_io.c indicators/indicators.h as_core_design/as_core_design.h as_mem_words/as_mem_words.h as_symb_table/as_symb_table.h
	gcc -c -Wall -ansi -pedantic assembler_io/assembler_io.c -o assembler_io/assembler_io.o

as_symb_table.o: as_symb_table/as_symb_table.c indicators/indicators.h as_core_design/as_core_design.h as_mem_words/as_mem_words.h as_symb_table/as_symb_table.h assembler_io/assembler_io.h
	gcc -c -Wall -ansi -pedantic as_symb_table/as_symb_table.c -o as_symb_table/as_symb_table.o


clean:
	rm -f *.o *~


# GL
