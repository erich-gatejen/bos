/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED

	Version  : 1.0

	File     : BOSMSG.C
	Purpose  : Program code for the Battle Operating System console messages

*/

#include<stdlib.h>
#include<stdio.h>
#include<conio.h>
#include<time.h>
#include<string.h>
#include"bos.h"
#include"bosmsg.h"

/* --- External vars.  Defined in BOS.C ----------------------------------*/
extern char		  MMICmnd[MAXCMDSIZE]; /* User input command	    */
extern unsigned char  MMICmndPtr;		 /* Pointer into ^		    */
extern unsigned char  MMICmndArgs;		 /* Number of arguments to a line */
extern long		  TotalClocks;	      /* Number of clocks so far executed */
extern int            MsgMode;		 /* Message format type to use    */
extern unsigned int	  CoreSize;		 /* Core size in instructions     */


/* -------- MMI Routines -------------------------------------------------*/

/* --- TimeStamp -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Put a time stamp on screen */
void  TimeStamp() {

   time_t   Time;
   char	  *TimeString, *FindEnd;

   Time        = time(NULL); 		/* Get time 		 */
   TimeString  = ctime( &Time );   /* Convert to string */

   /* Chop the string */
   TimeString = TimeString + 11;   /* Move start to start of time */
   FindEnd    = TimeString + 8;
   *FindEnd   = 0;			     /* NULL terminate before year

   /* Put the timestamp */
   cputs( TimeString );

}; /* end TimeStamp */

/* --- BOSMsg -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Send a message from the BOS */
void  BOSMsg ( char  *Message ) {

   cputs("BOS>");

   /* What to put next? */
   switch( MsgMode ) {

	 case TIMEM   : /* Put the timestamp */
				 TimeStamp();
				 cputs("<|");
				 break;

	 case SYSCNTM : /* Put the total system clocks */
				 printf("%ld", TotalClocks );
				 cputs("<|");
				 break;

	 default      : /* No info */
				 putch('|');

   }; /* end case */

   cputs( Message );

}; /* end BOSMsg */


/* --- BOSMsgI-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Send a message from the BOS, interrupt the command line */
void  BOSMsgI ( char  *Message ) {

   cputs("\rBOS>");

   /* What to put next? */
   switch( MsgMode ) {

	 case TIMEM   : /* Put the timestamp */
				 TimeStamp();
				 cputs("<|");
				 break;

	 case SYSCNTM : /* Put the total system clocks */
				 printf("%ld", TotalClocks );
				 cputs("<|");
				 break;

	 default      : /* No info */
				 putch('|');

   }; /* end case */

   cputs( Message );

}; /* end BOSMsgI */


/* --- BOSMsgIDone-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Restore the MMI command line after a BOS message interrupt			*/
void  BOSMsgIDone () {

   /* Put prompt and command line back */
   cputs("\n\rBOS>>");
   MMICmnd[MMICmndPtr] = NULL;
   cputs( MMICmnd );

}; /* BOSMsgIDone */


/* --- BOSMsgTaunt   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Send a taunt from the BOS, interrupt and restore the command line */
void  BOSMsgTaunt ( char  *ProgName, char  *Message ) {

   /* carrage return */
   putch('\r');

   /* Put the message */
   cputs(ProgName);
   putch('>');
 
   /* What to put next? */
   switch( MsgMode ) {

	 case TIMEM   : /* Put the timestamp */
				 TimeStamp();
				 cputs("<|");
				 break;

	 case SYSCNTM : /* Put the total system clocks */
				 printf("%d", TotalClocks );
				 cputs("<|");
				 break;

	 default      : /* No info */
				 putch('|');

   }; /* end case */   

   cputs( Message );
   cputs("\n\r");    /* must have return */

   /* Put prompt and command line back */
   cputs("BOS>>");
   MMICmnd[MMICmndPtr] = NULL;
   cputs( MMICmnd );

}; /* end BOSMsgTaunt */


/* --- BOSMsgU -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Continue a BOS Message with an unsigned integer */
void  BOSMsgU ( char  *String, unsigned int  Number ) {

   cprintf( String, Number );

}; /* end BOSMsgU */


/* --- BOSMsgS -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Continue a BOS Message with a signed integer */
void  BOSMsgS ( char  *String, int  Number ) {

   cprintf( String, Number );

}; /* end BOSMsgS */


/* --- BOSMsgL -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Continue a BOS Message with a long integer */
void  BOSMsgL ( char  *String, long  Number ) {

   cprintf( String, Number );

}; /* end BOSMsgL */


/* --- BOSMsgST -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Continue a BOS Message with a string */
void  BOSMsgST ( char  *String, char  *String2 ) {

   cprintf( String, String2 );

}; /* end BOSMsgST */


/* --- BOSMsgC -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Continue a BOS Message */
void  BOSMsgC ( char  *String ) {

   cputs( String );

}; /* end BOSMsgC */


/* --- BOSPrompt -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Put the BOS prompt on the screen */
void  BOSPrompt () {

   cputs("BOS>>");

}; /* end BOSPrompt */


/* --- MsgInitBOS   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Tell user BOS is ready */
void  MsgInitBOS() {

   clrscr();
   cputs("[[[         BATTLE OPERATING SYSTEM  v1.0           ]]]\n\r");
   cputs("[ Copyright Erich P Gatejen 1993  All Rights Reserved ]\n\n\r");
   printf("!BOS Initialized!  Core Available : %d\n\n\r", CoreSize);

}; /* end MsgInitBOS */


/* --- MsgNoCoreSpec   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Tell user that they need to specify a core size a the first param of the
   command line												*/
void  MsgNoCoreSpec () {

   cputs("\n\r");
   BOSMsg("Core size not specified on command line!\n\r");
   BOSMsg("BOS ABORTING!\n\n\r");

}; /* end MsgNoCoreSpec */


/* --- MsgBadCoreSpec -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Tell user that they specified a core incorrect size ont the command line */
void  MsgBadCoreSpec () {

   cputs("\n\r");
   BOSMsg("Bad core size specified on command line!\n\r");
   BOSMsg("BOS ABORTING!\n\n\r");

}; /* end MsgNoCoreSpec */


/* --- MsgNoCore -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Tell user that BOS could not obtain memory for a core */
void  MsgNoCore () {

   cputs("\n\r");
   BOSMsg("Cannot allocate core memory.");
   BOSMsg("BOS ABORTING!\n\n\r");

}; /* end MsgNoCoreSpec */


/* --- MsgBadCommand   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Tell user that the command entered is bad ( not recognized ) */
void  MsgBadCommand () {

   int	Step;
   char	Text[80];

   Step = 1;
   /* Find first space after the first character and NULL it */
   while (( Step < MAXCMDSIZE + 1 )&&( MMICmnd[Step] != ' ')) Step++;
   MMICmnd[Step] = NULL;

   /* Leave safety net so MMI command displayed doesn't excede displayable
	 size                                                                */
   MMICmnd[40] = NULL;

   /* Assemble message */
   strcpy( Text, "Command > " );
   strcat( Text, MMICmnd );
   strcat( Text, " < not used!\n\n\r");

   /* Send the message */
   BOSMsg( Text );

};


/* --- DisplayState   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Put the state of the screen.  Always use 8 chars */
void  DisplayState( int  State ) {

   /* Case through states */
   switch( State ) {

	 case READY_RUN  : cputs("RUNNING ");
      		         break;	

	 case HELD       : cputs("HELD    ");
			         break;

	 case WAIT       : cputs("WAITING ");
   				    break;

	 case NEW        : cputs("NEW     ");
				    break;

	 case KILLED     : cputs("KILLED  ");
				    break;

	 case UNUSED     : cputs("UNUSED  ");
			         break;

   }; /* end case */

}; /* end DiplayState */


/* --- DisplayOpToken   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Put the character for the optoken */
void  DisplayOpToken( unsigned char  Byte ) {

   /* -- Put the first token */
   switch( Byte ) {

	    case NOTOKEN : BOSMsgC("X");
				    break;
	    case REG     : BOSMsgC("R");
				    break;
	    case PTR     : BOSMsgC("^");
				    break;
	    case MEM     : BOSMsgC("M");
				    break;
	    case IMMEDIATE : BOSMsgC("I");

    }; /* end case */

}; /* end DisplayOpToken */


/* --- Continue   -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Put message and wait for user to hit a key */
char  Continue() {

   cputs("-- Press [ENTER] to continue -- -- -- -- -- -- \n\r");
   return ( getch() );

}; /* end Continue */


/* --- BOSMsgClean-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Clean the next screen line */
void  BOSMsgClean() {

   cputs("\r");
   clreol();

}; /* end BOSMsgClean */