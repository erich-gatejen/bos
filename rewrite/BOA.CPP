/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : BOA.CPP
	Purpose  : Program code for the BattleOS Assembler

*/


/* ---- INCLUDE files ------------------------------------------------*/

#include<stdio.h>
#include<conio.h>
#include<alloc.h>
#include "binst.h"
#include "label.h"
#include "line.h"
#include "instarry.h"

/* ---- Global Variables --------------------------------------------*/

   FILE	 		*InFile;
   FILE        	*OutFile;		/* File for input and output */
   FILE			*ListF;		/* For a list file		    */

   Taunt       	Taunts[TAUNTSNUMBER];    /* Taunt arrays    */
   char			ProgName[PROGNAMESIZE];  /* Program name    */

   unsigned int     LineCount  = 1; /* Line being processed	     */
   unsigned int     InstrCount = 0; /* Instruction counter       */

   unsigned int	ErrorCount = 0; /* Number of errors encountered */

   /* Boolean variables */
   char			HaveProgName = FALSE;  /* Does a program name exist? */

   /* Pointer to label chain */
   struct Label	*FirstLabel  = NULL;   /* Insure no labels in list   */

   /* Output file name */
   char	  FileOut[14];

   /* List file name */
   char	  ListOut[14];

   /* Flag the use of a list file */
   char	  ListFile = FALSE;	

   /* Flag PassTwo is underway */
   char	  PassTwoF = FALSE;

   /* Put these instr generation vars here to uncomplicate things */
   int	  Operand1;
   int	  Operand2;
   unsigned char    OperandType1;
   unsigned char	OperandType2;

   /* Taunt usage list */
   char	  TauntUsage[8]  = { FALSE, FALSE, FALSE, FALSE, FALSE, 
						 FALSE, FALSE, FALSE			 }; 

   /* Instruction count for list file */
   unsigned int	  InstrPut;

   /* Instruction binary, for list file */
   char	  InstrBinary[15];


/* --- Messages to the user ----------------------------------------*/

void Do_Opening() {

	cputs("\n\r");
	cputs(" !! Battle Operating System !!  (C) 1993 Erich P Gatejen\n\r");
	cputs(" Battle Operating system Assembler ( BOA ) v1.0 -\n\r\n\r");

}; /* end Do_Opening */


void Message_No_File() {

	cputs(" -ERROR:  No filename specified for processing.\n\r\n\r");

}; /* end Message_No_File */


void Message_Bye() {

	cputs(" --- BASM completed  \n\r\n\r");

};

void Message_Cant_Open( char   *Name ) {

	printf(" -ERROR:  Cannot open file '%s'.\n\r\n\r", Name );

};

void Message_Bad_File_Name() {

	cputs("  -ERROR:  Cannot use filename.\n\r\n\r");

};

void Message_ASM_Abort() {

	printf("\n\r - %d errors encountered.\n\r", ErrorCount );
	cputs( " !ABORTING assembly!\n\r\n\r");

}; /* end PassOne */

void Error_Message( char *String ) {

	printf(" --- Error - line %d :", LineCount );
	printf(" %s\n\r", String );
	ErrorCount++;

	/* Put in list file if enabled */
	if ( ListFile == TRUE ) {

	/* if PassTwo error, then write a CR/LF */
	if ( PassTwoF == TRUE ) fputs("\n\r", ListF );

	fprintf( ListF, "  --- Error - line %d :", LineCount );
	fprintf( ListF, "  %s\n\r", String );
        
	}; /* end if */

}; /* end Error_Message */


/* --- Routines ----------------------------------------------------*/

/* -------- OpenFiles routine. ---------- */
int OpenFiles( char	  *FileName ) {

	int    FindExt;

	/* Open file, if good open output, else message error  */
	if ((InFile = fopen( FileName, "rt" )) == NULL) {
	   Message_Cant_Open( FileName );
	   return( 1 );			/* Return nonzero */
	};

 /* Parse file name, change extention, and open it for output */
	strcpy( FileOut, FileName );

	/* Find the extension */
	FindExt = 0;
	while ((FileOut[FindExt] != '.') && (FileOut[FindExt] != NULL)) {
	   FindExt++;
	};

	/* See if this file name is haywire */
	if (FindExt > 8) {
		Message_Bad_File_Name();
		return( 1 );
	};


	if (FileOut[FindExt] == NULL) FileOut[FindExt] = '.';

	/* Add the extension */
	FindExt++;

	FileOut[FindExt] 	= 'B';
	FileOut[FindExt+1]  = 'O';
	FileOut[FindExt+2]  = 'S';
	FileOut[FindExt+3]  = NULL;

	/* Open output file if we can, else error */
	if ((OutFile = fopen(FileOut, "wb")) == NULL) {
	   Message_Cant_Open( FileOut );
	   return( 1 );			/* Return nonzero */
	};

	return( NULL );  /* Return SUCCESS! */

}; /* end OpenFiles */


/* -------- OpenList routine. ---------- */
int OpenList( char	  *FileName ) {

	int    FindExt;

	/* Open file, if good open output, else message error  */
	if ((ListF = fopen( FileName, "wb" )) == NULL) {
	   Message_Cant_Open( FileName );
	   return( FALSE );			/* Return nonzero */
	};

	return( TRUE );  /* Return SUCCESS! */

}; /* end OpenList */

/* -------- ListFileTail --------------- */
void  ListFileTail () {

   /* Jump down a few lines */
   fputs("\n\r\n\r", ListF );

   /* Put general info */
   fputs("-- End assembly.\n\r", ListF );

   /* List number of errors found */
   fprintf( ListF, " - Number of errors : %d\n\r\n\r", ErrorCount );

   /* If it made it to pass two, then do label list */
   if  ( PassTwoF == TRUE ) {

	 /* Label list header */
	 fputs("--- Label List ---------------\n\r", ListF );

      /* use FirstLabel pointer to traverse.  ( it's not needed anymore ) */
	 while ( FirstLabel != NULL ) {

	    fprintf( ListF,            "%-10s = %04d \n\r",
			   FirstLabel->Text, FirstLabel->Offset );

	    /* Move to next label */
         FirstLabel = FirstLabel->Next;

      }; /* end while */

   }; /* end if */

}; /* end ListFileTail */


/* -------- CloseFiles routine --------- */
void  CloseFiles () {

	fclose( InFile  );
	fclose( OutFile );

	/* If there was a list file, then close it */
	if ( ListFile == TRUE ) {

	   ListFileTail();
	   fclose( ListF );

     }; /* end if */

}; /* end CloseFiles */


/* -------- CloseFilesK routine --------- */
void  CloseFilesK () {

	fclose( InFile  );
	fclose( OutFile );

	/* Kill the output file */
	unlink( FileOut );

	/* If there was a list file, then close it */
	if ( ListFile == TRUE ) {

        ListFileTail();
	   fclose( ListF );

     }; /* end if */

}; /* end CloseFiles */


/* --------- Get the program name --------- */
void  Get_Prog_Name ( char  *String ) {

	int  Step = 0;


	/* Check if a program name has already been declared 		*/
	if ( HaveProgName == TRUE ) {
	   /* Message error and return 						*/
	   Error_Message("Multipul program names" );
	   return;
	};

	String++;   /* Move to first char				    */

	/* Move program name and set HaveProgName flag	    */
	while ( (*String != '\n')&&( Step != PROGNAMESIZE ) ) {
	   ProgName[Step] = *String;
	   Step++;
	   String++;
	};

     HaveProgName = TRUE;   /* We now have a program name! */

}; /* end Get_Prog_Name */


/* ----------- CheckMnemonic --------------*/
int  CheckMnemonic ( char  *String ) {

   int	Step;
   char   Mnemonic[4];


   for ( Step=0; Step<3; Step++) {

	 Mnemonic[Step] = *String;
	 String++;

   }; /* end for */
   Mnemonic[3] = NULL;

   /* Adjust TAB or CR to space */
   if ((Mnemonic[2] == 9)||(Mnemonic[2] == '\n')) Mnemonic[2] = ' ';

   /* Insure lower case	 */
   strlwr( Mnemonic );

   /* Pad space, if neccessary, for 2 letter mnemonics */
   if (Mnemonic[2] == '\n') Mnemonic[2] = ' ';

   /* Find Token */
   Step = 0;
   while( strcmp( TokenTable[Step].Text, Mnemonic) != 0 ) {

	 Step++;

	 /* If step == number of tokens then the mnemonic is not recognized */
	 if ( Step == Numtokens ) {

     	Error_Message( "Unknown mnemonic!" );
		return( BADMNEMONIC );

	 }; /* end if */

   }; /* end while */

   /* return position in token table */
   return( Step );


}; /* end CheckMnemonic */


/* ----------- Set_Taunt ----------------*/
void  Set_Taunt( char  *String ) {

	int		TauntNum;
	char		NumChar;
	int		Step		= 0;

  /* Get taunt number */
	String++;				/* Point to number */
	NumChar  = *String;
     String++;

	/* Check number validity */
	if ( (NumChar < '0')||(NumChar > '7')||(*String != '"') ) {
		/* Invalid, message and return */
		Error_Message( "Invalid taunt format.");
		return;
		};

	/* Set TauntNum */
	TauntNum = NumChar - '0';

	/* See if the taunt is in use, if so ERROR */
	if ( TauntUsage[TauntNum] == TRUE ) {

	   Error_Message("Taunt number already used.");
	   return;

	}; /* end if */

	/* Set the taunt number as in use */
	TauntUsage[TauntNum] = TRUE;

  /* Move taunt text */
	String++;				/* Point to beginning of text */
	while( (*String != '\n')&&(Step < TAUNTSIZE) ) {
		Taunts[TauntNum][Step] = *String;
		String++;
          Step++;
	};

	/* Done */

}; /* end Set_Taunt */


/* ----------- Get_Value ------------------*/
int  Get_Value ( char *String, int *Value ) {

	char		Buffer[6];
	long 	Number;
	int		Step;
	int		SignValue;


     Step = 0;

	/* Check for data, error if data not flush with indicator (+,#,or $) */
	if ( (*String == ' ')||(*String == '\t')||(*String =='\n')||
	    ( (*String <  '0')&&(*String > '9')&&(*String != '+')&&
		 (*String != '-') )                                    ) {

	   /* Error */
	   Error_Message("Incorrect data format");
        return( FALSE );

	}; /* end if */


	/* Check for a negative sign */
	if ( *String == '-' ) {

	   SignValue = -1;
	   String++;		/* Move beyond sign character */
	   }
	else
	   SignValue = 1;

	/* If there is a '+', suck it up */
	if ( *String == '+' ) ++String;

	/* Stuff the buffer */
	while( (*String >= '0') && (*String <= '9') ) {

	   /* See if number is automatically to large ( in char size ) */
	   if ( Step == 5 ) {

		 /* if so, report the error, and return */
		 Error_Message( "Data item to large." );
		 return( FALSE );

	   };

	   Buffer[Step] = *String;
	   Step++;
	   String++;
	};

	/* Null terminate the string */
	Buffer[Step] = NULL;

     /* Translate to long int     */
	Number = atol( Buffer );

	/* Sign the number		    */
	Number = Number * SignValue;

	/* See if the number is outside a sign int range, if so then error */
	if ( (Number > INTHI ) || (Number < INTLOW) )  return( FALSE );

	/* Stuff into Value and return good */
	*Value = Number;
     return( TRUE );


}; /* end Get_Value */


/* ----------- Find_More ------------------*/
void  Find_More ( char  *String ) {

	int   Step;


	/* Find the next non space character of the line ( or no character ) */
	while ( (*String == 32)||(*String == 9) ) {
	   String++;
	};
                              /*  Tab char ^   */
	/* case through possibilities */
	switch ( *String ) {

	   /* Program name character, not allowed!	*/
	   case '!'		  : 
		 Error_Message( "Duplicate label names" );
		 break;

	   /* Label character, not allowed!		*/
	   case '@'         : 
		 Error_Message( "Multipul labels on a single line." );
		 break;

	   /* Taunt define character, not allowed! */
	   case '"'		: 
		 Error_Message( "Taunt not on separate line." );
		 break;

	   /* Taunt inbed character, not allowed! */
	   case '['		: 
		 Error_Message( "Taunt inbed not after instruction." );
		 break;

	   /* Data character, increase instruction counter    */
	   case '#'         : InstrCount++;
					  break;

	   /* Program comment character, ignore line */
	   case '/'         : break;	

	   /* NULL or newline, do nothing		 */
	   case NULL        : 
	   case '\n'		: break;

	   /* OTHERWISE, Make sure it's text, else error   */
	   default		: if ( CheckMnemonic(String) != BADMNEMONIC )

						/* Assume an instruction */
						InstrCount++;

					  else

						/* Otherwise it is garbage */
						Error_Message("Garbage text found.");

					  /* end if */

	  }; /* end case */


   /* DONE!! */


}; /* end Find_More */



/* --------------- SetLabel ---------------*/
void  Set_Label( char  *String ) {

	int            Step  = 0;    /* Looping var 				           */
	static int	Value;        /* Hold value for number. Force to DGROUP */
	struct Label   *Node,
				*Last;       /* Pointer into label chain           */
	char			Text[10],    /* Temp hold for label text	      */
				*FindEq;     /* Walk the String in search of and = */


  /* If no label has yet been defined, define this one to start chain */
  if (FirstLabel == NULL) {

	   FirstLabel = (struct Label *)malloc(sizeof(struct Label));/* Make node */
	   FirstLabel->Next   = NULL;
	   FirstLabel->Offset = InstrCount; /* Set this offset for the label*/

	   /* Stuff label text */
	   while ( (*String != '\n')&&(Step < 10)&&(*String != ' ')&&
			 (*String != '	' ) ) {
                /* Tab char  ^ */
		 FirstLabel->Text[Step] = *String;
		 Step++;
		 String++;
	   };

	   /* Pad text with a NULL */
	   FirstLabel->Text[Step] = NULL;
	   Node = FirstLabel;			/* In case we need it later */

	   /* Done with first label */

  } else {

	/* Not the first label */
     /* Now, move label text into local and traverse label chain to
	   check for duplicate labels								*/

	/* Stuff label text into temp local var */
     Step = 0;
	while ( (*String != '\n')&&(Step < 10)&&(*String != ' ')&&
	        (*String != '	' ) ) {
                /* Tab char  ^ */
	   Text[Step] = *String;
	   Step++;
	   String++;
	};
	Text[Step] = NULL;  /* Null terminate the temp string */

	/* Now start to traverse */
	Node = FirstLabel;
	while ( Node != NULL ) {
	   /* check for duplication */
	   if ( strcmp(Text, Node->Text) == 0 ) {
	      Error_Message( "Duplicate label names." );
		 return;		/* Dup found!, return */	
	   }; /* end if */

	   /* move to next node */
	   Last = Node;		/* Remember last node */
	   Node = Node->Next;    /* Point to next	  */

	}; /* end while */

	/* We have found the last node and there is no duplication, SO add label */
	Node = (struct Label *)malloc(sizeof(struct Label));  /* Make node */
	Node->Next   = NULL;
	Node->Offset = InstrCount; /* Set this offset for the label*/
     
	/* Link to last in chain */
	Last->Next   = Node;

	/* Stuff label text */
	Step = 0;
	while ( (Text[Step] != NULL)&&(Step < 10)) {
	   Node->Text[Step] = Text[Step];
	   Step++;
	}; /* end while */


	/* Pad text with a NULL */
	Node->Text[Step] = NULL;

  }; /* end if */

  /* --  See if this is a label set line */

	/* Find WS */
	FindEq = String;
	while ( (*FindEq != 32)&&(*FindEq != 9)&&(*FindEq != '\n')&&
	        (*FindEq != '\n') ) {
	  ++FindEq;
	};
	/* Jump WS */
	while ( (*FindEq == 32)||(*FindEq == 9) ) {
	  ++FindEq;
	};

	/* Is there a '=' sign ? */
	if ( *FindEq == '=' ) {

	   /* YES!!, it is a label set line! */
	   /* Find next non WS */
	   ++FindEq;
	   while ( (*FindEq == 32)||(*FindEq == 9) ) {
		++FindEq;
	   };

	   /* Find the value, and place it */
	   Get_Value( FindEq, &Value );
	   Node->Offset = Value;

	} else

        /* NO!! Then continue normal line processing.   		  */
	   /* Call Find_More to see if there is anything else on line */
	   /* String must point to char following label */
	   Find_More ( String );

	/* end if */

	/* DONE! */

}; /* end Set_Label */


/* ----------- X_Label ---------------------*/
int	X_Label( char  *String, int	*Op ) {

	struct Label	*Node;
	char	  		Text[10];
	int	  		Found, Step;

     Found = FALSE;

	/* Stuff label text into temp local var */
     Step = 0;
	while ( (*String != '\n')&&(Step < 10)&&(*String != ' ')&&
	        (*String != '	' ) ) {
                /* Tab char  ^ */
	   Text[Step] = *String;
	   Step++;
	   String++;
	};
	Text[Step] = NULL;  /* Null terminate the temp string */

	/* Traverse until found or not found */
	Node = FirstLabel;
	while ( (Node != NULL)&&(Found == FALSE) ) {

	   /* check for duplication */
	   if ( strcmp(Text, Node->Text) == 0 ) {
		 /* We found it */
		 Found = TRUE;

		 /* Get the data, data is an offset from current position */
		 *Op = Node->Offset - InstrCount;

	   }; /* end if */

	   /* move to next node */
	   Node = Node->Next;    /* Point to next	  */

	}; /* end while */

	/* if not found then error */
	if (Found == FALSE) {
	   Error_Message("Unknown label.");
	   return (FALSE);
	};

	/* Otherwise, return good */
	return( TRUE );

}; /* end X_Label */


/* ----------- Get_Operand -----------------*/
char *Get_Operand ( char  *String, int  *Operand, int  *OperandT ) {

     /* Temporaries */
	int	Op, OpT;

	/* Now find the next character	*/
	while ( (*String == 32)||(*String == 9) ) {
	  String++;
	};

	/* Case through what we may have found */
	switch ( *String ) {

	   /* Taunt inbed character, no operand here */
	   case '['  :

	   /* End of line, no operand here			*/
	   case '\n' :
	   case NULL :                               

	   /* Comment, no operand here			*/
	   case '/'  : OpT = NOOPERAND;
				break;

	   /* Label, so go find it				*/
	   case '@'  : if ( X_Label( String, &Op ) == FALSE )
					OpT = ERROROPERAND;     /* An error was returned */
				else
					OpT = MEM;


				break;

	   /* Registers */
	   case 'A'  :
	   case 'B'  :
	   case 'C'  : OpT = REG;
				Op  = *String;	/* Make certain they are the same */
				break;

	   /* Pointer */
	   case '^'  : String++;			/* Point to next character 	     */
				OpT  = PTR;		/* Should be pointer type          */
				/* See what type of pointer it is					*/
				switch( *String ) {

				   /* Register pointer	*/
				   case 'A'  :
				   case 'B'  :
				   case 'C'  : OpT = REG;   /* Still a register */
							Op  = (*String + REGPTRBIT);
							break;

	   			   /* Memory offset pointer  */
	   			   case '+'  :
				   case '-'  : if( Get_Value( String, &Op ) == FALSE ) {
					             OpT = ERROROPERAND;
					             Error_Message( " Invalid Offset value.");
				               }; /* end if */
 				               break;

				   /* Immediate pointer */
				   case '$'  : String++;
                                   /* See if Error is returned */
				               if( Get_Value( String, &Op ) == FALSE ) {
					             OpT = ERROROPERAND;
					             Error_Message( " Invalid Immediate value.");
				               }; /* end if */
				               break;

	                 /* Label, so go find it				*/
	                 case '@'  : if ( X_Label( String, &Op ) == FALSE )
					            OpT = ERROROPERAND;     
						    break;



				   /* NOT A VALID POINTER, ERROR */
				   default   : OpT = ERROROPERAND;
							Error_Message(
							"Pointer operand not valid type." );

				}; /* end case */
				break;

	   /* Memory offset */
	   case '+'  :
	   case '-'  :	OpT	= MEM;

				if( Get_Value( String, &Op ) == FALSE ) {
					OpT = ERROROPERAND;
					Error_Message( " Invalid Offset value.");
				}; /* end if */
				break;

	   /* Get immediate */
	   case '$'  : String++;
				OpT = IMMEDIATE;

                    /* See if Error is returned */
				if( Get_Value( String, &Op ) == FALSE ) {
					OpT = ERROROPERAND;
					Error_Message( " Invalid Immediate value.");
				}; /* end if */
				break;

	   /* Anything else is an error */
	   default   : OpT  = ERROROPERAND;
				Error_Message( "Unknown operand type." );

	}; /* end case */

	/* Report the values */
	*Operand  = Op;
     *OperandT = OpT;

	/* Make sure String is returned pointing at whitespace, if no taunt */
	if ( *String != '[' )

	   while ((*String != ' ')&&(*String != 9)&&(*String != NULL)&&
		     (*String != '\n')) {
	      String++;
	   };

	/* end if */

	return( String );

}; /* end Get_Operand */


/* ----------- Data_Instr ------------------*/
void Data_Instr ( char  *String ) {

	char			InstrBBuf[5];  /* Buffer for binary data		 */

	long	int		Number;		/* Number obtained by the translate */

	unsigned int	TempInt = 0;

     struct Instr   Instruction;


	/* Adjust back the instr counter for list file */
	InstrPut++;

	/* Look at next char 	    */
	String++;

	/* Get value and look for error */
	if ( Get_Value( String, &Number ) == FALSE ) {

        /* Error occured */
	   InstrCount++;
        return;

	}; /* end if */

	/* Generate instruction */
	Instruction.Token    = DAT;
	Instruction.OpToken  = 0;
	Instruction.Operand1 = Number;	/* Stuff data into first  operand	*/
	Instruction.Operand2 = 0;
	Instruction.BOS	 = 0;

	/* Put the instruction to the file */
	fwrite( &Instruction, sizeof(struct Instr), 1, OutFile );

     /* if list file is enabled, then put the instruction binary data int
	 the list file buffer									 */
     if ( ListFile == TRUE ) {
	 
	  TempInt = Instruction.Token;
	  TempInt = TempInt << 8;	     		  /* Move to high byte */
	  TempInt = TempInt + Instruction.OpToken; /* Add TokenOp byte  */
	  sprintf( InstrBinary, "%04x", TempInt );

	  strcat( InstrBinary, ":" );

	  TempInt = Instruction.Operand1;
	  sprintf( InstrBBuf, "%04x", TempInt );
	  strcat( InstrBinary, InstrBBuf );

	  strcat( InstrBinary, ":" );

	  TempInt = Instruction.Operand2;
	  sprintf( InstrBBuf, "%04x", TempInt );

       strcat( InstrBinary, InstrBBuf );

     }; /* end if */

	/* Find white space or eoln */
	while ( (*String != ' ')&&(*String != '\t')&&(*String != '\n') ) {
	   String++;
	};

	/* Now insure the line ends or there is a comment	*/
	while ( (*String == ' ')||(*String == '\t') ) {
	  String++;
	};

	/* case through possibilities */
	switch( *String ) {

		/* Comment character, OK!, move on   */
		case  '/'	    : break;

		/* Taunt imbed, not allowed for data */
		case  '['	    : Error_Message("Taunt imbed not allowed on data.");
					 break;

		/* End of line, OK!, move on		  */
		case  NULL    :
		case  '\n'    : break;

		/* Otherwise there is an error	  */
		default	    : Error_Message("Incorrect Syntax.");

	}; /* end case */

	/* Up instruction counter */
	InstrCount++;

	/* DONE! */

}; /* end Data_Instr */


/* ----------- Imbed_Taunt -----------------*/
int  Imbed_Taunt ( unsigned char  *Token, char   *String ) {

   unsigned char  TauntNum;


   /* First see if is a DAT instruction.  TAUNTS not allowed */
   if ( *Token == DAT ) {
	 Error_Message("Taunt imbed not allowed on data");
	 return(FALSE);
   }; /* end if */

  /* Point to taunt number */
   String++;

   /* See is taunt number is in range */
   if ( ( *String < '0' ) || ( *String > '7' ) ) {

	 /* Taunt number out of range.  !!ERROR!! */
	 Error_Message("Taunt number out of range.");
	 return( FALSE );

   }; /* end if */

   /* Get taunt number */
   TauntNum = *String - '0';

   /* If taunt not defined then ERROR */
   if ( TauntUsage[TauntNum] == FALSE ) {
	 Error_Message("Taunt not defined.");
	 return(FALSE);
   }; /* end if */    

    /* Shift the number into 64/32/16 bits */
   TauntNum = TauntNum << 4;

   /* Flag that a taunt is present */
   TauntNum = TauntNum + TauntPresent;

   /* Combine with token */
   *Token = *Token + TauntNum;

   /* DONE, return good */
   return( TRUE );

}; /* end Imbed_Taunt */


/* ----------- Code_Instr ------------------*/
void Code_Instr ( char  *String ) {

   char	Mnemonic[4],
		InstrBBuf[5];

   int	Step;

   unsigned char	Token,
				OpsAll;
   unsigned char	OperandToken;

   struct Instr     Instruction;

   unsigned int	TempInt;



   /* Adjust back the insrt counter for list file */
   InstrPut++;

   /* Get the mnemonic position in table */
   Step = CheckMnemonic( String );

   /* if Step indicates a bad mnemonic, then return */
   if ( Step == BADMNEMONIC ) return;

   /* Extract the token */
   Token  = TokenTable[Step].Token;
   OpsAll = TokenTable[Step].OpsAllowed;

   /* Now point to whitespace	*/
   while ( (*String != 32)&&(*String != 9)&&(*String != '\n') ) {
	  String++;
   };

   /* -- Pop operands -- */

   Operand2		 = 0;
   OperandToken      = 0;
   OperandType2      = 0;

   /* Get the operands */
   String = Get_Operand( String, &Operand1, &OperandType1);  
   if ( OperandType1 != NOOPERAND )
	   String = Get_Operand( String, &Operand2, &OperandType2 );

   /* If either operands have an error then return */
   if ( (Operand1 == ERROROPERAND) || (Operand2 == ERROROPERAND) ) return;

   /* Built Operand Token */
   OperandType1 	= OperandType1 << 4;	      /* Shift into MSBs     */
   OperandToken     = OperandType1 + OperandType2; /* Mrg into token byte */

   /* Let's see if there is an illegal operand */
   if ( OperandToken != (OperandToken & OpsAll) ) {

	 /* The OperandToken does not fit in the Allowed Operands !!ERROR!! */

	 /* Peel off the LSBs token and see if problem is there */
	 OperandToken = OperandToken & 0x0F;
	 OpsAll	    = OpsAll & 0x0F;		/* Eliminates the MSBs */

	 if ( OperandToken != OperandToken & OpsAll )
		/* Error in the LSBs token */

		/* See if it is because there is no second operand */
		if ( OpsAll == 0 )
			Error_Message("Instruction does not have second operand.");
		else
			Error_Message("Second operand not an allowed type.");

	 else {
		/* Error in MSBs token */
		Error_Message("First operand not an allowed type.");

	     /* Up instruction counter */
		InstrCount++;

		return;

      }; /* end if */

   }; /* end error ops if */


   /* ------ We should have a valid instruction ------ */

   /* -- See if we have a taunt imbed on this line --- */
   /* Now find the next character	*/
   while ( (*String == 32)||(*String == 9) ) {
	 String++;
   };

   /* If it is the imbed char then do the imbed */
   if ( *String == '[' )
	 if ( Imbed_Taunt( &Token, String ) == FALSE ) {
	    /* Error occured in imbed, return */

	    /* Up instruction counter */
	    InstrCount++;

         return;

      }; /* end if */

   /* Build the instruction */
   Instruction.Token 	= Token;
   Instruction.OpToken   = OperandToken;
   Instruction.Operand1  = Operand1;
   Instruction.Operand2  = Operand2;
   Instruction.BOS		= NULL;

   /* Put the instruction to the file */
   fwrite( &Instruction, sizeof(struct Instr), 1, OutFile );

   /* if list file is enabled, then put the instruction binary data int
	 the list file buffer									 */
   if ( ListFile == TRUE ) {
	 
	 TempInt = Instruction.Token;
	 TempInt = TempInt << 8;	     		  /* Move to high byte */
	 TempInt = TempInt + Instruction.OpToken; /* Add TokenOp byte  */
	 sprintf( InstrBinary, "%04x", TempInt );

	 strcat( InstrBinary, ":" );

	 TempInt = Instruction.Operand1;
	 sprintf( InstrBBuf, "%04x", TempInt );
	 strcat( InstrBinary, InstrBBuf );

	 strcat( InstrBinary, ":" );

	 TempInt = Instruction.Operand2;
	 sprintf( InstrBBuf, "%04x", TempInt );

      strcat( InstrBinary, InstrBBuf );

   }; /* end if */

   /* Up instruction counter */
   InstrCount++;

}; /* end Code_Instr */


/* ----------- Lable_Line ------------------*/
void Label_Line ( char  *String ) {


	/* Find first white space */
	while ( (*String != 32)&&(*String != 9)&&(*String != NULL)&&
		   (*String != '\n' ) ) {
	   String++;
	};

     /* See is anything else on line		*/
	while ( (*String == 32)||(*String == 9) ) {
		String++;
	};

	/* case through possibilities */
	switch ( *String ) {


	   /* Data character, DATA!, put it into an instruction   */
	   case '#'         : Data_Instr( String );
					  break;

	   /* If it was a label set line, do nothing  		   */
	   case '='		:

	   /* NULL or newline, do nothing, it's an empty line	   */
	   case NULL        :
	   case '\n'		: break;

	   /* OTHERWISE, assume it is an instruction and parse it */
	   default		: Code_Instr( String );

	  }; /* end case */


   /* DONE!! */


}; /* end Label_Line */

/* --------------- ListHeader --------------- */
void  ListHeader() {

   /* Put header lines */
   fputs("\n\r !! Battle Operating System !!  (C) 1993 Erich P Gatejen\n\r",
         ListF );
   fputs(" Battle Operating system Assembler ( BOA ) -\n\r\n\r", ListF );
   fputs("Assembly list for program file :", ListF );
   fputs( FileOut, ListF );
   fputs("\n\r\n\r", ListF );

}; /* end ListHeader */


/* --- Primary routines --------------------------------------------*/

/* -- PassOne  : Do the first pass of the assembly -- */
void	 PassOne () {

	char		LineBuffer[81];
	int 		Step;


   LineBuffer[81] = 0;

   /* Reset the instruction counter */
   InstrCount     = 0;

   /* Process file line at a time until end of file */
	while ( !feof( InFile ) ) {

	   /* Get the line */
	   fgets( LineBuffer, 80, InFile );

	   /* If a list file is enabled, then put it in */
	   if ( ListFile == TRUE ) {

		 fprintf( ListF, "%4u ", LineCount );
           fputs( LineBuffer, ListF );

	   }; /* end if */

	   /* Find the first character of the line ( or no character ) */
	   Step = 0;
	   while ( (LineBuffer[Step] == 32)||(LineBuffer[Step] == 9) ) {
		 Step++;
	   };

	   /* Case through the posible lines */
	   switch( LineBuffer[Step] ) {

		/* Program name character	*/
		case '!'		  : Get_Prog_Name( &LineBuffer[Step]);
					    break;

		/* Label character			*/
		case '@'         : Set_Label( &LineBuffer[Step]);
					    break;

		/* Taunt define character */
		case '"'		  : Set_Taunt( &LineBuffer[Step]);
					    break;

		/* Data character, increase instruction counter    */
		case '#'         : InstrCount++;
					    break;

		/* Program comment character, ignore line */
		case '/'         : break;	

	     /* Taunt inbed character, not allowed! */
	     case '['		  : 
		 Error_Message( "Taunt inbed not after instruction." );
		 break;

		/* NULL or newline, do nothing		 */
		case NULL        : 
		case '\n'        : break;

		/* OTHERWISE, Check Mnemonic, else error   */
		default		  : if ( CheckMnemonic( &(LineBuffer[Step]) ) !=
						    BADMNEMONIC )

						  /* Assume an instruction */
						  InstrCount++;

	  }; /* end case */

	LineCount++;	/* Point to the next line */

	}; /* end while */

}; /* end PassOne */


/* -- DoBOSHeader : Write the BOS executable file header to the output file */
void  DoBOSHeader () {

   /* Write BOS version discriptor and copyright */
   fputc( VERSIONDISC,     OutFile );
   fputs( COPYRIGHTNOTICE, OutFile );

   /* Adjust instruction count for next write */
   InstrCount++;  /* Adjust count.  From 1 instead of 0 */

   /* Write word containing program size */
   fwrite( &InstrCount, sizeof( unsigned int), 1,  OutFile );

   /* Write the program name and taunt list */
   fwrite( ProgName, sizeof( char ), PROGNAMESIZE, OutFile );
   fwrite( Taunts,   TAUNTSIZE,      TAUNTSNUMBER, OutFile );

   
}; /* end DoBOSHeader */


/* -- PassTwo  : Do the second pass of the assembly -- */
void	 PassTwo () {

	char		LineBuffer[81];
     char      ListBuffer[60];
	int 		Step, TabStep, PutStep;
	

	LineBuffer[81] = 0;
	LineCount      = 1;	/* Reset the line counter */

	PassTwoF       = TRUE; /* Flag pass two is underway */

   /* Reset the instruction counter */
   InstrCount     = 0;

   /* Process file line at a time until end of file */
   while ( !feof( InFile ) ) {

	   /* If list file is enabled then take the time to clear the first
		 40 chars of the LineBuffer							*/
        if ( ListFile == TRUE )
		 for ( Step = 0; Step < 55; Step++ ) LineBuffer[Step] = NULL;

	   /* Get the line */
	   fgets( LineBuffer, 80, InFile );


	   /* If a list file is enabled, Put line to file */
	   if ( ListFile == TRUE ) {

           /* Kill the binary data */
           InstrBinary[0] = NULL;

		 PutStep = 0;
		 Step    = 0;

		 while ( PutStep < 55 ) {
		    
		    /* if Null or LF then put a space */
		    if ( (LineBuffer[Step] == '\n')||(LineBuffer[Step] == NULL ) )

			  /* Turn LFs and Nulls into spaces */
			  ListBuffer[PutStep] = ' ';

		    else

			  /* Otherwise, check for tab */
			  if ( LineBuffer[Step] != '\t' )

                    /* Not TAB, straight copy */
				ListBuffer[PutStep] = LineBuffer[Step];

			  else {

                    /* Convert the TAB to five spaces */
				for ( TabStep = 0; TabStep < 5; TabStep++ ) {
				 
				   /* Put space */
			        ListBuffer[PutStep] = ' ';

				   /* Increment the Step counter */
                       PutStep++;

				}; /* end for */ 

				/* Adjust PutStep down */
				PutStep--;

			  }; /* end if */

		    /* end if */

		    Step++;
              PutStep++;

		 }; /* end while */

		 /* Make sure the line is always only 53 chars */
		 ListBuffer[55] = NULL;
    
		 /* Do adjust to instr. count for a list file */
		 InstrPut = InstrCount-1;
					 
        }; /* end if */

	   /* Find the first character of the line ( or no character ) */
	   Step = 0;
	   while ( (LineBuffer[Step] == 32)||(LineBuffer[Step] == 9)) {
		 Step++;
        };

	   /* Case through the posible lines */
	   switch( LineBuffer[Step] ) {

		/* Label character, look for instruction on this line	*/
		case '@'         : Label_Line( &LineBuffer[Step]);
					    break;

		/* Data character, DATA! put it into an instruction	*/
		case '#'         : Data_Instr( &LineBuffer[Step]);
					    break;

		/* Taunt define character, ignore this line  		*/
		case '"'		  : break;

		/* Program name character, ignore this line	*/
		case '!'		  : 	

		/* Program comment character, ignore line */
		case '/'         : 

		/* Taunt inbed character, not allowed!, should have been caught
		   on the first pass									 */


		/* NULL or newline, do nothing		 */
		case NULL        : 
		case '\n'        : break;

		/* OTHERWISE, assume it is an instruction and parse it  */
		default		  : Code_Instr( &LineBuffer[Step] );

	 }; /* end case */

      /* Is there a list file */
	 if ( ListFile == TRUE ) {

	    /* Adjust if went negetive */
	    if ( InstrPut > 65355 ) InstrPut = 0;

	    /* Put standard data */
	    fprintf( ListF, "%04u :", InstrPut );
	    fputs(   ListBuffer, ListF );

	    /* If the binary string has data, then write it too */
	    if ( InstrBinary[0] != NULL ) {

		  fputs( InstrBinary, ListF );

	    }; /* end if */

	    /* Now write the return */
	    fputc( '\n', ListF );

	 }; /* end if */

	 LineCount++;	/* Point to the next line */

   }; /* end while */

}; /* end PassTwo */


/* --- MAIN --------------------------------------------------------*/

void main ( int argc, char *argv[] ) {

	char		*FileName;	/* Pointer to filename from command line */

 /* Message the opening									 */
	Do_Opening();


 /* Get file name from command line and pass it to file open func */
	/* Check to see if there is a file name, if not message and exit */
	if (argc < 2 ) {
		Message_No_File();
		Message_Bye();
		exit(0);
	};

	/* Do open file, if non-zero returned then message user and end */
	if (OpenFiles( argv[1] )) {
		Message_Bye();
		exit(0);
	};

	/* See if the user wants a list file, if so then set it up */
	if (argc > 2) 

		/* If the list file name was valid, then open and flag */
		if (OpenList( argv[2] ) == TRUE) {

             /* Enable list file outputs */
		   ListFile = TRUE;
		   
		   /* Put the header to the file */ 
		   ListHeader();

		}; /* end if */
	

 /* Do pass one, quit if errors found                    	   */
	PassOne();
	if (ErrorCount) {
		CloseFilesK();
		Message_ASM_Abort();
		exit(0);
	};

	/* Survived pass one, if list file is enabled then kill it and
	   start second pass listing							*/
	if ( ListFile == TRUE ) {

	   rewind( ListF );
	   ListHeader();

	};

	/* Reset the input file and write .BOS header	   		*/
	rewind( InFile );
	DoBOSHeader();

	/* Do pass two, quit if errors found                        */
	PassTwo();
	if (ErrorCount) {
		CloseFilesK();
		Message_ASM_Abort();
		exit(0);
	};
		

	/* Done.  Clean up.  Close files. 						   */
	CloseFiles();
	Message_Bye();


}; /* end main */
