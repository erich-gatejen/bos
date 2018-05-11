/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : BOA.H
	Purpose  : Header file for the BattleOS Assembler

*/

/* ---- Define classes local to BOA ---- */
/* Define the label class */
class Label {
			char   		Text[32];  /* Text of the Label   		   */
			int    		Offset;    /* Offset or value of the Label */
			struct Label  *Next;      /* * to next label     		   */

};


/* Define the 'instruction' structure */
typedef struct Instr{
					unsigned char  Token;	 /* Token of instruction  */
					unsigned char  OpToken;   /* Token of operand type */
					int            Operand1;  /* Operand 1			 */
					int			Operand2;  /* Operand 2             */
					unsigned char  BOS;		 /* Used by BOS		 */
};


/* Define taunt */
#define  TAUNTSIZE  40
typedef  char		Taunt[TAUNTSIZE];

/* VERSION DISCRIPTOR... !!!!DO NOT CHANGE!!!!! 		*/
#define  VERSIONDISC    100	   /* indicates v1.00    */

/* COPYRIGHT NOTICE..... !!!!DO NOT CHANGE!!!!!        */
#define  COPYRIGHTNOTICE "BOS1.00/Copyright Erich P Gatejen 1992/All Rights Reserved"

#define  NOTICESIZE		58