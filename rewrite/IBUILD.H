/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : IBUILD.H
	Purpose  : Header file for the BattleOS Assembler Instruction builder
        	   module	

*/

// -- Includes --------------------------------------------------------------
#include "binst.h"

// Enumerate the possible errors from the following class(es)
enum  InstrError { NOERROR            = 0,
		   UNKOWNMNEMONIC     = 6,
		   LEFTOPINVALID      = 7,
		   RIGHTOPINVALID     = 8,
		   LEFTNOTALLOWED     = 9,
		   RIGHTNOTALLOWED    = 10,
		   NOOPERANDS	      = 11,
		   LEFTOPONLY	      = 12

};


// Define instruction builder class
class IBuilder {

   Instr   Instruction;	// Working instruction
   char	   Binary[15];	// Instruction binary image

   BOOLEAN MValid;	// Flags for valid componants
   BOOLEAN LValid;
   BOOLEAN RValid;

 public:

   // Constructor
   IBuilder( void );

   // Information members
   BOOLEAN  IsValid(  void );
   BOOLEAN  IsMnemonic( char* Text );
   char*    BinaryIs( void );

   // Loader members
   InstrError  LoadMnemonic(    char*  Text );
   InstrError  LoadLeftOperand(  signed Value, unsigned  OpToken );
   InstrError  LoadRightOperand( signed Value, unsigned  OpToken );
   InstrError  LoadTaunt( unsigned TauntNumber );

   Instr*  BondInstr( void );

}; // end IBuilder
