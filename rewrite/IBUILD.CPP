/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : IBUILD.CPP
	Purpose  : Program code for the BattleOS Assembler

	!! Program code for the instruction builder class members   !!
	!! Must be linked to the BOA				    !!

*/


// --------------------------------------------------------------------------
// ---- INCLUDE files -------------------------------------------------------
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include "ibuild.h"

// --------------------------------------------------------------------------
// ---- Local Classes -------------------------------------------------------


// --------------------------------------------------------------------------
// ---- Global Variables ----------------------------------------------------

// - Allowable operands table
#include "allops.h"

// - Instruction mnemonic table
#include "instarry.h"

// --------------------------------------------------------------------------
// |||| Member function code ||||||||||||||||||||||||||||||||||||||||||||||||
// --------------------------------------------------------------------------

// <<-- CLASS  : IBuilder ------------------------------------------------->>

// ---- MEMBER : constructor ----
   IBuilder::IBuilder( void ) {

   // Start with a bad instruction
   MValid = FALSE;
   LValid = FALSE;
   RValid = FALSE;

}; // end constructor


// ---- MEMBER : IsValid ----
BOOLEAN IBuilder::IsValid( void ) {

   if ( MValid && LValid && RValid ) return TRUE;
   else return FALSE;

}; // end IsValid


// ---- MEMBER : IsMnemonic ----
BOOLEAN  IBuilder::IsMnemonic( char  *Text ) {

  int	Step;

  // Search the instruction table for the mnemonic
  for ( Step = DAT; Step <= INT; ++Step ) {
     if ( !strcmp( Text, InstrTable[Step].Text ) ) return TRUE;
  };

  // Not found
  return FALSE;

}; // end IsMnemonic


// ---- MEMBER : BinaryIs ----
char*  IBuilder::BinaryIs ( void ) {

   unsigned int	  Word;

   // First word
   Word = Instruction.FirstWord();
   sprintf( Binary, "%04x", Word );
   Binary[4] = ':';

   // Second word
   Word = Instruction.LeftOperandIs();
   sprintf( &Binary[5], "%04x", Word );
   Binary[9] = ':';

   // Third word
   Word = Instruction.RightOperandIs();
   sprintf( &Binary[10], "%04x", Word );

   return( Binary );

}; // end BinaryIs


// ---- MEMBER : LoadMnemonic ----
InstrError  IBuilder::LoadMnemonic( char*  Text ) {

  int	Step;

  // Search the instruction table for the mnemonic
  for ( Step = DAT; Step <= INT; ++Step ) {
     if ( !strcmp( Text, InstrTable[Step].Text ) ) {

	// - Found
	Instruction.TokenIs( InstrTable[Step].Token );
        MValid = TRUE;
	return NOERROR;

     }; // end if

  }; // end for

  // Not found
  return UNKNOWNMNEMONIC;

}; // end LoadMnemonic