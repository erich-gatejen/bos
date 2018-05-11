/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : DEFINES.H
	Purpose  : Standard defines for all BattleOS code
*/

/* Preclude multiple declarations */
#ifndef __DEFINES_H
#define	__DEFINES_H

/* Common constants */
#define   FALSE	0
#define   TRUE      1

#define   INTHI     32000     /* Highest value an integer can hold */
#define   INTLOW   -32000     /* Lowest ( negative )		   */


/* Define machine instruction tokens */
#define   DAT       0    /* Data						 */
#define	  MOV	    1	 /* Move                             */
#define   CMP	    2    /* Compare                          */
#define   JE	    3    /* Jump on =					 */
#define   JB        4    /* Jump below					 */
#define   JA        5    /* Jump above					 */
#define   JMP       6    /* Jump						 */
#define   INC	    7    /* Increment				      */
#define   DEC       8    /* Decrement                        */
#define   ADD       9    /* Add						 */
#define   SUB      10    /* Subtract					 */
#define   CALL	   11	 /* Call subroutine				 */
#define   RET	   12	 /* Return 					 */
#define   PUSH	   13	 /* Push stack					 */
#define	  POP	   14	 /* Pop stack					 */

/* Define BattleOS instruction tokens */
#define   RND      16    /* Random number				 */
#define   WT       17    /* Put trap					 */
#define   TMR      18    /* Set timer 	                     */
#define   HLT      19    /* Halt program till timer or trap  */
#define   INT	   20    /* Interrupt service routine. !!NOT IMPLEMENTED!! */

/* Register definitions	*/
#define   AREGISTER			'A'
#define   BREGISTER			'B'
#define   CREGISTER			'C'
#define   DREGISTER			'D'

#define	  SREGISTER			'S'  // Stack pointer

#define   PREGISTER			'P'  // Future use.  NOT IMPLEMENTED

/* -- Define standard types */

typedef	int	BOOLEAN;

/* Define Operand Token */
enum  OpToken { NOOPERAND = 0, REG = 1, RREFERENCE = 2, MEM = 3,
		IMMEDIATE = 4, ERROROPERAND = 5 		};

#endif