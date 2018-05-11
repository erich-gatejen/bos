/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : LINE.CPP
	Purpose  : Program code for the BattleOS Assembler

	!! Program code for the line processing class members 	    !!
	!! Must be linked to the BOA				    !!

*/


// --------------------------------------------------------------------------
// ---- INCLUDE files -------------------------------------------------------
#include<stdlib.h>
#include<alloc.h>
#include<ctype.h>
#include<string.h>
#include "defines.h"
#include "line.h"


// --------------------------------------------------------------------------
// ---- Global Variables ----------------------------------------------------
// NONE


// --------------------------------------------------------------------------
// |||| Member function code ||||||||||||||||||||||||||||||||||||||||||||||||
// --------------------------------------------------------------------------

// <<-- CLASS  : Element -------------------------------------------------->>

// ---- MEMBER : constructor ----
   Element::Element( char* String, int Size ) {

   // Point to nothing!
   Next = NULL;

   // Get the memory block
   Text = new char[Size + 1];

   // Stuff the data
   strcpy( Text, String ); 

   // Determine the type
   switch( *String ) {

     case '/' : Type = COMMENT;
		break;

     case '!' : Type = PROGNAME;
		break;

     case '=' : Type = ASSIGN;
		break;


     case '[' : Type = REFERENCE;
		break;

     case '$' : Type = IMMEDIATEV;
		break;

     case '-' : Type = ADDRESSM;
		break;

     case '+' : Type = ADDRESSP;
		break;

     case '@' : Type = LABEL;
		break;

     case '"' : Type = TAUNTDEFINE;
		break;

     case '<' : Type = TAUNTIMBED;
		break;

     default  : Type = TEXT;

   }; // end swtich

}; // end constructor


// <<-- CLASS  : Line ----------------------------------------------------->>

// ---- MEMBER : constructor ----
   Line::Line( char*  Input ) {

   // Local data
   char	    Buffer[80];
   int	    Step;		// Iterator
   Element* Node;

   // Clear list pointers
   Head = NULL;
   Tail = NULL;

   // Find first none white-space in line
   while( isspace( *Input ) ) {
      ++Input;
   }; // end while

   // Load element list
   while ( *Input ) {

      // Peal into buffer until white-space
      Step = 0;
      while( !isspace(*Input) ) {

	 Buffer[Step] = *Input;
	 ++Step;
	 ++Input;

      }; // end while . Step retains the length of the buffer.
      Buffer[Step] = NULL;

     // -- Log as an element
      // Create the element
      Node = new Element( Buffer, Step );

      // Is it the first label?
      if ( Head == NULL ) {

	 // Yes!  Add it to the list as the first node
	 Tail = Node;
	 Head = Node;

      } else {

         // -- No!
	 Tail->Next = Node; // Link last to new
	 Tail	    = Node; // Point tail to new

      }; // end if 

      // Find next none white-space in line
      while( isspace( *Input ) ) {
         ++Input;
      }; // end while

   }; // end while

}; // end constructor


// ---- MEMBER : destructor ----
   Line::~Line( void ) {

   // Release all the nodes back to the heap
   while ( Head != NULL ) {

	Tail = Head; 		// Save this node to delete
	Head = Head->Next;  // Point to the next node

	delete Tail;		// Delete the node

   }; // end while

}; // end destructor


// ---- MEMBER : HasElement ----
inline  BOOLEAN Line::HasElement( void ) {

   if ( Head == NULL ) return FALSE;
   else return TRUE;

}; // end HasElement


// ---- MEMBER : PopNext ----
Element*  Line::PopNext( void ) {

  Element*   Node;

  // Get and delink node
  Node  = Head;
  Head  = Node->Next;

  return Node;

}; // end PopNext




