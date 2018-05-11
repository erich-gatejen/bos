/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : ALLOPS.H
	Purpose  : Program code for the BattleOS Assembler

	!! Program code for the allowable operations table	    !!

*/


// --------------------------------------------------------------------------
// ---- INCLUDE files -------------------------------------------------------

// --------------------------------------------------------------------------
// ---- Local Classes -------------------------------------------------------


// --------------------------------------------------------------------------
// ---- Global Variables ----------------------------------------------------

// - Allowable operands table
BOOLEAN	AllowableMoves[4][4] = {

        // LEFT OPERAND
	// REGISTER - REFERENCE - MEMORY - IMMEDIATE
							 // RIGHT OPERAND
	{  TRUE,      TRUE, 	  FALSE,   FALSE     },  // REGISTER
	{  TRUE,      FALSE, 	  FALSE,   FALSE     },  // REFERENCE
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // MEMORY
	{  TRUE,      TRUE, 	  FALSE,   FALSE     },  // IMMEDIATE

};


// - Allowable operands table
BOOLEAN	AllowRightOps[20][4] = {

	// REGISTER - REFERENCE - MEMORY - IMMEDIATE

	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // DAT
	{  TRUE,      TRUE,  	  TRUE,    TRUE      },  // MOV
	{  TRUE,      FALSE,	  TRUE,    TRUE      },  // CMP
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // JE
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // JB
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // JA
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // JMP
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // INC
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // DEC
	{  TRUE,      FALSE,	  TRUE,    TRUE      },  // ADD
	{  TRUE,      FALSE,	  TRUE,    TRUE      },  // SUB
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // CALL
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // RET
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // PUSH
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // POP
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // RND
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // WT
	{  TRUE,      FALSE,	  FALSE,   TRUE      },  // TMR
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // HLT
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // INT - Not used

};

// - Allowable operands table
BOOLEAN	AllowLeftOps[20][4] = {

	// REGISTER - REFERENCE - MEMORY - IMMEDIATE

	{  FALSE,     FALSE,	  FALSE,   TRUE      },  // DAT
	{  TRUE,      TRUE,  	  FALSE,   FALSE     },  // MOV
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // CMP
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // JE
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // JB
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // JA
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // JMP
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // INC
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // DEC
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // ADD
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // SUB
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // CALL
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // RET
	{  TRUE,      FALSE,	  FALSE,   TRUE,     },  // PUSH
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // POP
	{  TRUE,      FALSE,	  FALSE,   FALSE     },  // RND
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // WT
	{  TRUE,      FALSE,	  TRUE,    FALSE     },  // TMR
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // HLT
	{  FALSE,     FALSE,	  FALSE,   FALSE     },  // INT - Not used

};

// - Has operand left
BOOLEAN HasOperandLeft[] = {

	TRUE,	// DAT
	TRUE,   // MOV
	TRUE,   // CMP
	TRUE,   // JE
	TRUE,   // JB
	TRUE,   // JA
	TRUE,   // JMP
	TRUE,   // INC
	TRUE,   // DEC
	TRUE,   // ADD
	TRUE,   // SUB
	TRUE,   // CALL
	FALSE,  // RET
	TRUE,   // PUSH
	TRUE,   // POP
	TRUE,   // RND
	TRUE,   // WT
	TRUE,   // TMR
	FALSE,  // HLT
	FALSE   // INT

};

// - Has operand right
BOOLEAN HasOperandRight[] = {

	FALSE,	// DAT
	TRUE,   // MOV
	TRUE,   // CMP
	FALSE,  // JE
	FALSE,  // JB
	FALSE,  // JA
	FALSE,  // JMP
	FALSE,  // INC
	FALSE,  // DEC
	TRUE,   // ADD
	TRUE,   // SUB
	FALSE,  // CALL
	FALSE,  // RET
	FALSE,  // PUSH
	FALSE,  // POP
	FALSE,  // RND
	TRUE,   // WT
	TRUE,   // TMR
	FALSE,  // HLT
	FALSE   // INT

};
