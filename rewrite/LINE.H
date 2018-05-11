/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : LINE.H
	Purpose  : Header file for the BattleOS Assembler Line processor class

	!! Class Line provides all functions !!

*/

// Enumerate element types
enum	 ElementTypes { COMMENT,            // /
			PROGNAME,           // !
			ASSIGN,             // =
			REFERENCE,          // [
			IMMEDIATEV,         // $
			ADDRESSM,	    // -
			ADDRESSP,	    // +
			LABEL,		    // @
			TEXT,		    // TEXT, no specific id
			TAUNTDEFINE,	    // "
			TAUNTIMBED	    // <

};


// Declare the Element type
struct Element {
	char*		Text;
	ElementTypes	Type;

	Element*  	Next;

  public:

	// Members
	Element( char* String, int Size );
       ~Element( void ) { delete Text; };

	ElementTypes  IsA( void ) { return Type; };

};

// Declare the Line class
class Line {

	char	        Text[80];

	// Element List pointers
	Element*	Head;
	Element*	Tail;

  public:

	 Line( char* Input );		// Constructor, supply the line
	~Line();			// Destructor

	BOOLEAN   HasElement( void );
	Element*  PopNext(    void );
	Element*  ShowNext(   void )  { return Head; };

}; // end Line

