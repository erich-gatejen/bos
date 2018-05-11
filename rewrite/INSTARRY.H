/*
	BattleOS : Engage battling programs.

	Copyright (C) 1992 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : INSTARRY.h
	Purpose  : Header file for the BattleOS Assembler

*/


/* This array is for translation of text instruction to instruction token */
struct  InstItem {
				char	       Text[4];
				unsigned char  Token;

} InstrTable[] = {
				"dat", DAT,
				"mov", MOV,
				"cmp", CMP,
				"je ", JE,
				"jb ", JB,
				"ja ", JA,
				"jmp", JMP,
				"inc", INC,
				"dec", DEC,
				"add", ADD,
				"sub", SUB,
				"cal", CALL,
				"ret", RET,
				"psh", PUSH,
				"pop", POP,
                                "rnd", RND,
				"wt ", WT,
				"tmr", TMR,
				"hlt", HLT,
				"int", INT

};

