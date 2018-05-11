/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : BOS.h
	Purpose  : Header file for the BattleOS Assembler

*/

/* Defines */
#define   FALSE	0
#define   TRUE      1


/* Define machine instruction tokens */
#define   DAT       0    /* Data						 */
#define	MOV		1	/* Move                             */
#define   CMP		2    /* Compare                          */
#define   JE		3    /* Jump on =					 */
#define   JB        4    /* Jump below					 */
#define   JA        5    /* Jump above					 */
#define   JMP       6    /* Jump						 */
#define   INC		7    /* Increment				      */
#define   DEC       8    /* Decrement                        */
#define   ADD       9    /* Add						 */
#define   SUB      10    /* Subtract					 */
#define   LOP      11    /* Loop of register C			 */

/* Define BattleOS instruction tokens */
#define   RND      12    /* Random number				 */
#define   WT       13    /* Put wait					 */
#define   TMR      14    /* Time event                       */
#define   HLT      15    /* Halt program till timer or wait  */

#define   Numtokens 	16 /* Number of defined tokens	 */

#define   TInstr_RegionM	   224	/* For masking	non-token	 */

#define   TauntPresent	   128  /* Set taunt present bit    */
#define   Max_Taunt           7  /* Maximum taunt number ( start at 0 ) */

/* Define Operand Tokens */
#define   NOTOKEN           0  /* No token for this operand  */
#define	REG			   1  /* Direct register			 */
#define   PTR			   2  /* Pointer				 */
#define   MEM               4  /* Direct memory	 	      */
#define   IMMEDIATE		   8  /* Immediate value			 */

/* Register definitions	*/
#define   REGPTRBIT           128			/* Register is a ptr bit */
#define   AREGISTER			'A'
#define   BREGISTER			'B'
#define   CREGISTER			'C'
#define   AREGPTR 			(AREGISTER + REGPTRBIT)
#define   BREGPTR			(BREGISTER + REGPTRBIT)
#define   CREGPTR 			(CREGISTER + REGPTRBIT)


/* General defines */
#define   PROGNAMESIZE       32  /* Size of program name     */


/* Type definitions */

/* Define pointer types */


/* Define structures */

/* Define the 'instruction' structure.  Actually each core location. */
typedef struct Instr{
					unsigned char  Token;	 /* Token of instruction  */
					unsigned char  OpToken;   /* Token of operand type */
					int            Operand1;  /* Operand 1			 */
					int			Operand2;  /* Operand 2             */
					unsigned char	OwnerHandl;/* USED by BOS.  Handle  */
										 /* to owner of core loc  */
};


/* Define taunt */
#define   TAUNTSIZE  	   40
#define 	TAUNTSNUMBER       8   /* Number of taunts         */
typedef  char		Taunt[TAUNTSIZE];

/* Define Program states	*/
enum  PCB_State  {
				READY_RUN,	/* Execute instruction on cycle */
				HELD,		/* Spawned but held by BOS	  */
				WAIT,          /* Wait event				  */
				NEW,			/* Loaded but not spawned	  */
				KILLED,        /* Program was killed		  */
				UNUSED		/* PCB not used			  */
};

/* Maximum number of programs (PCBs) and other constants */
#define  MAXPCB	20
#define  BASEPCB     0
#define  NOHANDLE   MAXPCB	/* A programs handle is its PCBs position
						   in the PCBList.  An out-of -range value
						   indicates 'no handle '.			   */
#define  WAITFLAG   128       /* Flags that memory location has a wait
						   placed on it					   */


/* Program Control Block */
#define  PNameText	 10
#define  PNameHandle 2		/* Allows for up to 99 unique program names */

typedef  struct PCB {
				  char			ProgramName[PNameText+PNameHandle+1];
				  unsigned int      InstrPointer;    /* Core IP	   */
				  unsigned int      ProgSize;        /* Load size    */
				  unsigned int    	State;           /* PCB_State    */
				  unsigned int		WaitPointer;     /* Core addr	   */
				  unsigned int      WaitSite;        /* Core addr    */
				  unsigned int      TimerPointer;    /* Core addr    */
				  unsigned int      TickCount;       /* Ticks to go  */
                      long			TotalClocks;     /* Total run    */

				  unsigned int		CoreHeld;        /* Amount owned */
				  unsigned int      WhosCode;		  /* Handle of code */

				  struct Instr 	A, B, C;         /* Registers      */
				  int			CMPResult;       /* Compare result */

                      Taunt			TauntList[TAUNTSNUMBER];

};

/* VERSION DISCRIPTOR... !!!!DO NOT CHANGE!!!!! 		*/
#define  VERSIONDISC    100	   /* indicates v1.00    */

/* COPYRIGHT NOTICE..... !!!!DO NOT CHANGE!!!!!        */
#define  COPYRIGHTNOTICE "BOS1.00/Copyright Erich P Gatejen 1993/All Rights Reserved"
#define  NOTICESIZE		58

/* ---- System defines	------ */
#define	MAXCORESPEC	    9000	/* Max size of core */
#define	MINCORESPEC		256  /* Min size of core */

#define   NOERROR			  0  /* No critical program error         */
#define   ERROR			  1  /* A critical program error exists   */

#define   MINPCB			  0  /* Minimum PCB handle number         */

/* ---- MMI defines  ---------- */
#define   MAXCMDSIZE		 60  /* Max inputed command size */
#define   MAXARGSIZE		 15  /* Max arg text size */
#define   MAXARGS			  8  /* Maximum arguments per MMI command */


/* -------- TYPEDEFS -----------------------------------------------------*/
   typedef  char	ArgText[MAXARGSIZE+1];  /* Text for parsed MMI arg    */


   enum     MsgModes  { BAREM, TIMEM, SYSCNTM }; /* Which Msg format to use*/
