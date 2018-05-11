/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : LABEL.H
	Purpose  : Header file for the BattleOS Assembler label handler class

	!! Class Line provides all functions !!

*/

/* Enumerate the possible returns from class members */
enum	 LabelResult { NOERROR  = 0, BADLABELSET   = 1, VALUEOUTOFRANGE = 2,
		       DUPLABEL = 3, LABELNOTFOUND = 4 };

/* Declare the Label type */
struct Label {
	char	Text[32];
	int	Value;

	Label*  Next;

};

/* Declare the Line class */
class LabelList {

	// Label List pointers
	Label*	Head;
	Label*	Tail;

  public:

	LabelList(void);	/* Constructor */
	~LabelList();		/* Destructor  */

	LabelResult  AddLabel	( char**	StringPointer, int	  Number );
	LabelResult  ExtractLabel( char**  StringPointer, int&   Value  );

}; // end LabelList

