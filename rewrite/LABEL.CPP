/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : LABEL.CPP
	Purpose  : Program code for the BattleOS Assembler

	!! Program code for the label list class members !!
	!! Must be linked to the BOA				    !!

*/


// --------------------------------------------------------------------------
// ---- INCLUDE files -------------------------------------------------------
#include<stdlib.h>
#include<alloc.h>
#include<ctype.h>
#include<string.h>
#include "defines.h"
#include "label.h"


// --------------------------------------------------------------------------
// ---- Global Variables ----------------------------------------------------
// NONE


// --------------------------------------------------------------------------
// |||| Member function code ||||||||||||||||||||||||||||||||||||||||||||||||
// --------------------------------------------------------------------------

// <<-- CLASS  : ListLabel ------------------------------------------------>>

// ---- MEMBER : constructor ----
   LabelList::LabelList( void ) {

   // Point to nothing!
   Head = NULL;
   Tail = NULL;

}; // end constructor


// ---- MEMBER : destructor ----
   LabelList::~LabelList( void ) {

   // Release all the nodes back to the heap
   while ( Head != NULL ) {

	Tail = Head; 		// Save this node to delete
	Head = Head->Next;  // Point to the next node

	delete Tail;		// Delete the node

   }; // end while

}; // end destructor


// ---- MEMBER : AddLabel ----
LabelResult  LabelList::AddLabel ( char**  StringPointer, int  Number ) {

   // Local var
   char*	String   = *StringPointer;	// Our string pointer
   char	Copy[32];					// Our copy of the text
   char	ValueP[7];				// Our copy of the text
   int	Step;					// General iterator
   int	Value;					// Value to place
   int	Sign;					// Sign Value

   Label* Node;					// Working node pointer

   // Load the local copy of the string
   Step = 0;
   while ( !isspace(*String) ) {             // Loop until white space
	 if ( Step < 31 ) Copy[Step] = *String; // Only copy is room left
	 ++String;
	 ++Step;
   }; // end while
   Copy[Step] = NULL;	// Null terminate the string

   // Point String to anything but white space
   while ( isspace(*String) ) { ++String; };

   // Is this a explicit label set ( '=' char )
   if ( *String == '=' ) {

    // --- Yes, then pick the value
	 ++String;
	 // Find sign value or number; if none, then error
	 while( isspace(*String) ) { ++String; };
	 if (*String != '-') {          // minus sign
	    Sign = -1;
	    ++String;
	 } else if (*String != '+') {   // positive sign
	    Sign = 1;
	    ++String;
	 } else if (!isdigit(*String))  // digit, assume positive
	    Sign = 1;
	 else
	    return BADLABELSET;	       // !!Last case!! ERROR

	 // Get the number text
	 Step = 0;
	 while ( !isspace(*String) ) {               // Loop until white space
	    if ( Step == 5 ) return VALUEOUTOFRANGE; // Too large ?
	    ValueP[Step] = *String;
	    ++String;
	    ++Step;
	 }; // end while
	 ValueP[Step] = NULL;	// Null terminate the string

	 Value = atoi( ValueP );
	 Value = Value * Sign;

	 // Make sure the value is in range.  If not, return error
	 if ( (Value < INTLOW) || (Value > INTHI) ) return VALUEOUTOFRANGE;

   } else {

	 // Value is the number passed into the function
	 Value = Number;

   }; // end if

   // Return string pointer
   *StringPointer = String;

   // Is it the first label?
   if ( Head == NULL ) {

    // -- Yes!

	 // Add it to the list as the first node
	 Node = new Label;
	 Tail = Node;
	 Head = Node;

   } else {

    // -- No!

	 // Traverse the list and see if it is a duplicate label
	 Node = Head;      // Point to first node
	 do {
	   // check for duplication
	   if ( strcmp(Copy, Node->Text) == 0 ) {
		 return DUPLABEL;					// Dup found!, return
	   }; // end if

	   // Move to next node
	   Node = Node->Next;

	 } while ( Node != NULL ); // end while whil no more nodes

	 // No duplicate.  So link in this new label
	 Node 	  = new Label;
	 Node->Next = NULL;

	 Tail->Next = Node;	// Link last to new
	 Tail	  = Node; // Point tail to new

   }; // end if first

   // Ok, stuff the data
   Node->Value = Value;
   strcpy(Node->Text, Copy );

   // DONE!! return no error
   return NOERROR;


}; // end AddLabel


// ---- MEMBER : ExtractLabel ----
LabelResult  LabelList::ExtractLabel( char**  StringPointer, int&   Value  ) {

   // Local var
   Label*	Node;
   char		Copy[32];
   char*	String = *StringPointer;
   int		Step;

   // Load the local copy of the string
   Step = 0;
   while ( !isspace(*String) ) {             // Loop until white space
	 if ( Step < 31 ) Copy[Step] = *String; // Only copy is room left
	 ++String;
	 ++Step;
   }; // end while
   Copy[Step] = NULL;	// Null terminate the string

   // Point String to anything but white space
   while ( isspace(*String) ) { ++String; };

   // Traverse the list and try to find the label.
   Node = Head;
   while ( Node ) {

	 // check for duplication
	 if ( strcmp(Copy, Node->Text) == 0 ) {
	    // We found it

	    // Get the data
	    Value = Node->Value;

	    // Return good
	    return NOERROR;

	 }; // end if

	 // move to next node
	 Node = Node->Next;    // Point to next

   }; // end while

   // If we got here, then the label wasn't found.  ERROR
   return LABELNOTFOUND;

}; // end ExtractLabel




