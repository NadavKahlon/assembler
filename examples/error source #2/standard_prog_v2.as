; input.as

.entry LIST
.extern W
MAIN:	add r3, LIST
LOOP:	prn #4.8
		lea W,r9
		inc r6
		mov r3,K
		sub r1,r4
		bne END
		cmp K,#-6
		bne &END
		dec W
.entry MAIN
		run MAIN
		jmp r4
		add L3, L3
END:	stop

STR: 	.string "abcd'
LIST: 	.data 6,-9
		.data -100
K: 		.data 31
.extern L3
