/*
	BattleOS : Engage battling programs.

	Copyright (C) 1992 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : Instname.h
	Purpose  : Header file for the BattleOS containing instruction names

     Dependent upon file BOS.h

*/

char   InstNames[Numtokens][4] = { "DAT",
							"MOV",
							"CMP",
							"JE ",
							"JB ",
							"JA ",
							"JMP",
							"INC",
							"DEC",
							"ADD",
							"SUB",
							"LOP",
							"RND",
							"WT ",
							"TMR",
							"HLT"
		
}; /* end InstNames definition */