/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : BINST.H
	Purpose  : Header file for the BattleOS Instruction class and
			 supporting stuff

*/

#ifndef __BINST_H
#define __BINST_H

/* Include standard definitions */
#include "defines.h"

/* Define the 'instruction' class.  */
class Instr {

   unsigned  Token     : 5;    // Token of instruction
   unsigned  OpTokenL  : 3;    // Token of left operand type
   unsigned  OpTokenR  : 3;    // Token of left operand type
   unsigned  	       : 1;    // Filler

   unsigned  TauntFlag : 1;    // Taunt flag
   unsigned  Taunt     : 3;    // Taunt embed field

   signed    OperandL  : 16;   // Left operand.  Full word
   signed    OperandR  : 16;   // Left operand.  Full word

   /* Used by BOS */
   unsigned  Owner     : 8;    // Owner handle
   unsigned  	       : 7;    // Filler
   unsigned  TrapSet   : 1;    // Wait flag

 public:

   /* BOOLEAN value access 			  */
   BOOLEAN   HasTaunt( void ) { return TauntFlag; };
   BOOLEAN   HasWait(  void ) { return TrapSet;   };

   /* Value access routines.  All inline  */
   unsigned  TokenIs(  void )       { return Token;	 };
   unsigned  LeftOpTokenIs(  void ) { return OpTokenL;  };
   unsigned  RightOpTokenIs( void ) { return OpTokenR;  };
   unsigned  TauntIs(  void )       { return Taunt;	 };
   signed int  LeftOperandIs(  void ) { return OperandL;  };
   signed int  RightOperandIs( void ) { return OperandR;  };
   unsigned  OwnerIs(  void )       { return Owner;	 };

   /* BOOLEAN set access				  */
   void  DoesHaveTaunt( void )     { TauntFlag = TRUE;  };
   void  DoesHaveWait( void )      { TrapSet   = TRUE;  };
   void  DoesNotHaveTaunt( void )  { TauntFlag = FALSE; };
   void  DoesNotHaveWait(  void )  { TrapSet   = FALSE; };

   /* Value set routines.  All inline and overloading the value access  */
   void  TokenIs( unsigned  InToken  ) { Token    = InToken;  };
   void  LeftOpTokenIs(  unsigned Op ) { OpTokenL = Op;       };
   void  RightOpTokenIs( unsigned Op ) { OpTokenR = Op;       };
   void  TauntIs( unsigned  InTaunt  ) { Taunt    = InTaunt;  };
   void  LeftOperandIs(  signed   Op ) { OperandL = Op;       };
   void  RightOperandIs( signed   Op ) { OperandR = Op;       };
   void  OwnerIs( unsigned  InOwner  ) { Owner    = InOwner;  };

   // Constructor
   Instr( void ) {  TokenIs( 0 );
		    LeftOpTokenIs(  0 );
		    RightOpTokenIs( 0 );
		    TauntIs( 0 );
		    LeftOperandIs(  0 );
		    RightOperandIs( 0 );
		    OwnerIs( 0 );

   };

   unsigned int FirstWord( void ) {

      unsigned int	Word;

      Word = Token << 11;
      Word = Word | ( OpTokenL  << 8 );
      Word = Word | ( OpTokenR  << 5 );
      Word = Word | ( TauntFlag << 3 );
      Word = Word | Taunt;

      return Word;

   };

}; // end Instr

#endif
