/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED

	Version  : 1.0

	File     : BOS.C
	Purpose  : Program code for the Battle Operating System

*/


/* -------- INCLUDE Files ------------------------------------------------*/
#include<stdlib.h>
#include<stdio.h>
#include<conio.h>
#include<alloc.h>
#include<time.h>
#include<string.h>
#include<bios.h>
#include<dos.h>
#include<dir.h>
#include"bos.h"
#include"bosmsg.h"


/* -------- DEFINES ------------------------------------------------------*/
#define   CTOKENHI			256  /* MMI Token defines 			  */
#define   NOCMDTOKEN		  0


/* -------- COMMAND TOKENS -----------------------------------------------*/
#define   CEXIT		    ( 'e' * CTOKENHI ) + 'x'  /* EXit		    */
#define   CLOAD		    ( 'l' * CTOKENHI ) + 'o'  /* LOad program    */
#define   CLIST		    ( 'l' * CTOKENHI ) + 'i'  /* LIst progs      */
#define   CSLIST             ( 's' * CTOKENHI ) + 'l'  /* System List     */
#define   CSET               ( 's' * CTOKENHI ) + 'e'  /* SEt system pars */
#define   CCORE              ( 'c' * CTOKENHI ) + 'o'  /* COre listing    */
#define   CSPAWN		    ( 's' * CTOKENHI ) + 'p'  /* Spawn a program */
#define   CHOLD              ( 'h' * CTOKENHI ) + 'o'  /* HOld a program  */ 
#define   CRELEASE           ( 'r' * CTOKENHI ) + 'e'  /* RElease program */
#define   CSHOLD             ( 's' * CTOKENHI ) + 'h'  /* SHold program   */
#define   CCHECKP	         ( 'c' * CTOKENHI ) + 'h'  /* CHeck point     */
#define   CCRESTORE          ( 'c' * CTOKENHI ) + 'r'  /* Check pnt restore */
#define   CCLEAR             ( 'c' * CTOKENHI ) + 'l'  /* CLear programs  */
#define   CDISK              ( 'd' * CTOKENHI ) + 'i'  /* DIsk prog list  */

/* -------- GLOBALS ------------------------------------------------------*/
   struct Instr	far *Core;	/* Pointer to main core */

   unsigned int    	CoreSize;		/* Core size in instructions      */

   struct PCB		PCBList[MAXPCB];	 /* Program Control Block list     */
   char			MMICmnd[MAXCMDSIZE]; /* User input command	    */
   unsigned char    MMICmndPtr;		 /* Pointer into ^		    */
   unsigned char    MMICmndArgs;		 /* Number of arguments to a
								    MMI command		    */
   ArgText		MMIList[MAXARGS];    /* Parsed MMI arguments	    */

   unsigned int     MemoryUsed;	/* RAM needed to build core	    */

   int			NumProgScheduled = 0, /* Number of programs scheduled  */
				NumProgHeld      = 0, /* Number of programs held       */
				NumProgWait      = 0, /* Number of programs waiting    */
				TimersActive     = 0; /* Number of timers active       */

   int			WaitPending	  = NOHANDLE,
				WaitPending2     = NOHANDLE; /* Wait trip pending flag */

   /* -- The following are changable system settings			 -- */
    
   int 		ProgDistance   = 100;   /* Minimum distance between progs*/
   int		DistanceTrys   = 5;     /* Max tries to find region	  */
   int         MsgMode        = TIMEM; /* Message format type to use    */

   long		TotalClocks    = 0;	    /* Number of clocks so far executed */

   int		CriticalError  = NOERROR; /* Flag for critical error     */
   int         DisplayTaunts  = TRUE;  /* Flag if to display taunts     */

   #include "instname.h" /* Includes the instruction name table (array) */

   /* -- Log file variables 									  */
   FILE		*LogFile		= NULL;
   int		LogActive		= FALSE;


/* -------- FUNCTION prototypes  -----------------------------------------*/
char	 *FindWS( char  *String );
char  *FindNonWS( char  *String );
unsigned int  CalcAddr( unsigned int  Addr, int  Offset );
struct Instr   far *CoreAddr( unsigned int  Offset );
struct Instr   far *CoreAddrC( unsigned int  Offset );
void   ClearCore( void );
int	  FindProg( char *String );
int    ParseCommand( void );
unsigned char  LoadProgram( unsigned char  *String );
void   SimpleList( void );
void   NormalList( void );
void   SystemList( void );
void   CoreList(  void );
void   SetSystem( void );
void   Spawn(       char  *Name  );
void   HoldProg(    char  *Name  );
void   ReleaseProg( char  *Name  );
struct Instr  GetValue( int  PCB, unsigned char  OpToken, int  Operand );
unsigned int   GetAddr( int  PCB, unsigned char  OpToken, int  Operand,
				    int  IFlag );
void  LoadRegister( int  PCB, struct Instr  Value, int Register );
void  KillProgram( int PCB );
unsigned int  CreateCore ( char *Size );
int	 DoCommand( unsigned int	Token );
int   CritCmndEntry( void );
int   DoKeyPress( void );
void  ExecInstruction ( int	PCB );
void  Scheduler ( void );
void  Master    ( void );
int   CheckPoint( char  *File );
int   RestoreCP(  char  *File );
void  ClearProgram( char*  Name );
void  DiskList ( void );


/* -------- COMMON Routines ----------------------------------------------*/



/* --- FindWS -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Return pointer to first whitespace in string passed */
char	 *FindWS( char  *String ) {

   while ( (*String != 32)&&(*String != 9)&&(*String != NULL)&&
		 (*String != '\n' ) ) {

	    String++;

   }; /* end while */

   return( String );

}; /* end FindWS */


/* --- FindNonWS -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
/* Return pointer to first non-whitespace in string passed */
char  *FindNonWS( char  *String ) {

   while ( (*String == 32)||(*String == 9) ) {

	 String++;

   }; /* end while */

   return( String );

}; /* end FindNonWS */


/* --- CalcAddr  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Will calculate an address doing any neccessary wraping			   */
unsigned int  CalcAddr( unsigned int  Addr, int  Offset ) {

   long	EAddr;

   /* Add the offset to address and put in long */
   EAddr = Addr + Offset;

   /* While it is negative, add coresize to wrap */
   while ( EAddr < 0 ) {
	 EAddr = CoreSize + EAddr;
   };

   /* While it is over coresize, subtract coresize to wrap */
   while ( EAddr >= CoreSize ) {
	 EAddr = EAddr - CoreSize;
   };

   /* Move EAddr into the Addr */
   Addr = EAddr;

   /* Return */
   return( Addr );

}; /* end CalcAddr */


/* --- CoreAddr  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Will convert an integer address into a core memory address            */
struct Instr   far *CoreAddr( unsigned int  Offset ) {

   struct Instr  far *CoreAddr;

   CoreAddr = Core + Offset; 

   return( CoreAddr );

}; /* end CoreAddr */                                                      


/* --- CoreAddrC    -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */
/* Will convert an integer address into a core memory address and will check
   for a wait set on that address.  If it does, it will set the wait
   pending for a PCB										   */
struct Instr   far *CoreAddrC( unsigned int  Offset ) {

   struct Instr  far *CoreAddr;

   CoreAddr = Core + Offset; 

   /* Check for a wait */
   if ( CoreAddr->OwnerHandl >= WAITFLAG ) {

	 /* Yes!!, Clear the wait */
	 CoreAddr->OwnerHandl = CoreAddr->OwnerHandl - WAITFLAG;

	 /* See if WaitPending is in use, if so use WaitPending2 */
	 if ( WaitPending == NOHANDLE )

	    /* Set the flag with the owner handle */
	    WaitPending = CoreAddr->OwnerHandl;

	 else

	    /* Set the flag with the owner handle, use second flag */
	    WaitPending2 = CoreAddr->OwnerHandl;

      /* end if */

   }; /* end if */

   return( CoreAddr );

}; /* end CoreAddrC */


/* --- ClearCore -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
void  ClearCore() {

   unsigned int  Step;

   struct Instr  far *Temp,
				 Dummy;

   /* Build dummy */
   Dummy.Token      = DAT;
   Dummy.OpToken    = 0;
   Dummy.Operand1   = 0;
   Dummy.Operand2   = 0;
   Dummy.OwnerHandl = NOHANDLE;

   /* Loop through entire core.  Make all 0 data instructions and
	 give no owner											    */
   for ( Step = 0; Step < CoreSize; Step++ ) {

	 Temp  = CoreAddr( Step );
	 *Temp = Dummy;

   }; /* end for */

}; /* end ClearCore */


/* --- FindProg  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */
int	FindProg( char *String ) {

   int    Step;
   char   BufferI[30],
		BufferL[15];


   /* Load the in buffer and insure lower case */
   Step = 0;
   while( *String != NULL ) {
	 BufferI[Step] = *String;
	 String++;
	 Step++;
   };

   BufferI[Step] = NULL;
   strlwr( BufferI );

   /* Find program by name */
   for ( Step = MINPCB; Step < MAXPCB; Step++ ) {

	 /* Move name into buffer */
	 strcpy( BufferL, PCBList[Step].ProgramName );

	 /* Lowercase the buffer */
	 strlwr( BufferL );

	 /* Does this PCB name match?  */
	 if ( strcmp( BufferL, BufferI ) == 0 )

	    /* Found */
	    return( Step);

   }; /* end FindProg */

   /* Never found */
   return( NOHANDLE );

}; /* end FindProg */


/* -------- COMMAND Routines ---------------------------------------------*/

/* --- ParseCommand ------------------------------------------------------*/
/* This function will parse the MMI command string.  If it finds an error
   it will return NOCMDTOKEN.									    */
int  ParseCommand() {

   #define	FIRSTCHAR		0
   #define     SECONDCHAR     1


   int	Step = 0;			/* Used to walk the MMI command text	    */
   int    Arg  = 0;			/* Current Arg                              */
   int    ArgS = 0; 		/* Used to move data into arg list		    */

   unsigned int	Token;    /* Command token					    */


   /* Check for a null in first two characters */
   if ( (MMICmnd[FIRSTCHAR] == NULL)||(MMICmnd[SECONDCHAR] == NULL) ) {

	 /* if so, the no command was entered */
	 BOSMsg("No command entered\n\r");
	 return( NOCMDTOKEN );

   }; /* end if */

   /* Now, lower case the mmi command text */
   strlwr( MMICmnd );

   /* Create value to commpare with command token. */
   Token = ( MMICmnd[FIRSTCHAR] * CTOKENHI ) + MMICmnd[SECONDCHAR];

   /* Find the end(NULL) or whitespace */
   while  ( (MMICmnd[Step] != ' ') && (MMICmnd[Step] != '\t') &&
		  (MMICmnd[Step] != NULL ) ) {
	 Step++;
   }; /* end while */

   /* Find the first non whitespace    */
   while  ( ((MMICmnd[Step] == ' ')||(MMICmnd[Step] == '\t'))&&
		   (MMICmnd[Step] != NULL ) ) {
	 Step++;
   }; /* end while */

   /* Peal off the arguments. If maxargs of first char is null, end */
   while ( (Arg < MAXARGS) && (MMICmnd[Step] != NULL ) ) {

	 ArgS = 0;
	 /* Move data into arg, move until limit, ws, or null */
	 while ( (MMICmnd[Step] != ' ') &&(MMICmnd[Step] != '\t')&&
		    (MMICmnd[Step] != NULL) ) {

	    /* Move the character */
	    MMIList[Arg][ArgS] = MMICmnd[Step];

	    /* Increment indexes */
	    Step++;
	    ArgS++;

	    /* If the maximum argument size has been reached, then find next
		  whitespace or null in command and break out of while			*/
	    if ( ArgS ==  MAXARGSIZE ) {

		  /* Find NULL or whitespace */
		  while  ( (MMICmnd[Step] != ' ') && (MMICmnd[Step] != '\t') &&
		           (MMICmnd[Step] != NULL ) ) {
	       Step++;
            }; /* end while */

		  /* break out of while */
		  break;

         }; /* end if */

      }; /* end while */

	 /* NULL terminate argument text */
	 MMIList[Arg][ArgS] = NULL;

	 /* Find next non whitespace */
	 while  ( ((MMICmnd[Step] == ' ')||(MMICmnd[Step] == '\t'))&&
			(MMICmnd[Step] != NULL) ) {
	    Step++;
	 }; /* end while */

	 /* Goto next argument */
	 Arg++;

   }; /* end while */

   /* Save number of arguments */
   MMICmndArgs = Arg;

   /* Return the token */
   return( Token );

}; /* end ParseCommand */


/* --- LoadProgram -------------------------------------------------------*/
/* This func will load a program into core. It returns the handle assigned
   the program.  Note if a prior instence of the program is loaded then
   it's text name will be appended with it's instance number (CFZ).  No
   more then 10 instances of the same program may be loaded			    */
unsigned char  LoadProgram( unsigned char  *String ) {

   char	FileName[14];  /* Programs file name */
   int	Step, Step1;

   FILE   *InFile;		/* File struct */

   char	Temp;		/* Temp character */
   int    ProgSize;      /* Program size   */
   char	ProgName[PROGNAMESIZE];      /* Program name */
   char   HandleText[PNameHandle + 1]; /* Program handle text */

   int    Handle = NOHANDLE;		
   unsigned int	Region, RegionT,	/* Location to place program */
                    FrontT, EndT;		/* Test locations		    */

   int    Found;                        /* Region found flag */

   struct Instr  far *CorePtr;          /* Pointer into the core     */


   /* If string points to an eoln or NULL then no filename, ERROR */
   if ( (*String == '\n') || (*String == NULL) ) {

	 /* Error message */
	 BOSMsg("'LOad program' ERROR: No filename specified.\n\r");

	 /* Return no handle, indicating an error */
	 return( NOHANDLE );

   }; /* end if */

   /* See if there is a PCB available */
   Step = 0;
   while ( Step < MAXPCB ) {

	 /* See if space available */
	 if ( PCBList[Step].State == UNUSED ) {
	    Handle = Step;
	    break;

	 };

	 /* Inc step */
	 Step++;

   }; /* end while */

   /* Was no place found, error if so */
   if ( Handle == NOHANDLE ) {

	 /* Send error message */
	 BOSMsg("'LOad program' ERROR: maximum programs already loaded.\n\r");

	 /* return no handle */
	 return( NOHANDLE );

   }; /* end if */

   /* Peal off the file name */
   Step = 0;
   while( (*String != '\n') && (*String != NULL) && (*String != '.') &&
		(Step < 8) ) {

		/* Tranfer char */
		FileName[Step] = *String;

		/* Increment character pointers */
		String++;
		Step++;

   }; /* end while */

   /* Add extention and NULL to file name */
   FileName[Step]   = '.';
   FileName[Step+1] = 'B';
   FileName[Step+2] = 'O';
   FileName[Step+3] = 'S';
   FileName[Step+4] = NULL;

   /* Open the file, if error then report and return */
   if (( InFile = fopen( FileName, "rb" ) ) == NULL ) {

	 /* Can't open file */
	 BOSMsg("'LOad program' ERROR: Bad filename\n\r");

	 /* Return no handle, indicating an error */
	 return( NOHANDLE );

   }; /* end if */

   /* Read the version discriptor byte.  If newer version then error */
   Temp = fgetc(InFile);
   if ( Temp > VERSIONDISC ) {

	 /* Can't open file */
	 BOSMsg("'LOad program' ERROR: Program needs newer version of BOS\n\r");

	 /* Close file */
	 fclose( InFile );

	 /* Return no handle, indicating an error */
	 return( NOHANDLE );

   }; /* end if */

   /* Move past copyright notice in file, check for error */
   if ( fseek( InFile, NOTICESIZE, SEEK_CUR ) ) {

	 /* Can't open file */
	 BOSMsg("'LOad program' ERROR: Program file is corrupted\n\r");

	 /* Close file */
	 fclose( InFile );

	 /* Return no handle, indicating an error */
	 return( NOHANDLE );

   }; /* end if */

   /* Get the program size */
   fread(&ProgSize, sizeof(int), 1, InFile );

   /* See if we can find a suitable region, pick a random spot and
	 make sure it isnt within ProgDistance of another program.  Try
	 this for DistanceTrys times before returning no can do	    */

   for ( Step = 0; Step < DistanceTrys; Step++ ) {

	 /* Assume it will be found */
	 Found = TRUE;

	 /* Get a random number */
	 Region  = random( CoreSize );
	 RegionT = CalcAddr( Region, ProgSize ); /* find end of program */

	 /* Loop and check against all other PCBs */
	 for ( Step1 = 0; Step1 < MAXPCB; Step1++ ) {

	    /* Only do for PCBs in use */
	    if ( PCBList[Step1].State != UNUSED ) {

	       /* Calculate this PCBs bounds */
	       FrontT = CalcAddr(  PCBList[Step1].InstrPointer,
					      -PCBList[Step1].ProgSize     );
		  FrontT = CalcAddr(  FrontT, -ProgDistance      );
		  EndT   = CalcAddr(  PCBList[Step1].InstrPointer,
						  PCBList[Step1].ProgSize     );
		  EndT   = CalcAddr(  EndT, ProgDistance         );

		  /* Which range check is neccessary?  */
		  if ( EndT > FrontT ) {

			/* Normal check */
			if ( ((Region  <= EndT)&&(Region >= FrontT))||
				((RegionT <= EndT)&&(RegionT >= FrontT)) ) {

				/* A conflict! */
				Found = FALSE;

				/* Break out of this loop */
				break;

			}; /* end if */


		  } else

			/* Program stratles the core boundries, check this way */
			if ( (Region  <= EndT) || (Region >= FrontT) ) {

				/* A conflict! */
				Found = FALSE;

				/* Break out of this loop */
				break;

			}; /* end if */

		  /* end if ( which check ) */

	    }; /* end if ( PCB in use? ) */

	 }; /* end for */

	 /* if Found is still TRUE, then we have found an appriate spot */
	 if ( Found == TRUE ) break;

   }; /* end for */

   /* Was region finally found? */
   if ( Found == FALSE ) {

	 /* Close file */
	 fclose( InFile );

	 /* Not found, send error message and return */
	 BOSMsg("'LOad program' ERROR: Unable to secure memory region.\n\r");
	 BOSMsg("'                   : Program '");
	 BOSMsgC( FileName );
	 BOSMsgC( "' not loaded.\n\r");
	 return( NOHANDLE );

   }; /* end if */

   /* Read the program name and taunt list */
   fread( &ProgName, sizeof( char ), PROGNAMESIZE, InFile );
   fread( &(PCBList[Handle].TauntList), TAUNTSIZE,  TAUNTSNUMBER, InFile );

  /* - Add handle to program name */
   Step = 0;
   while( (ProgName[Step] != NULL)&&(Step < PNameText) ) {

	 /* Find place to put handle text */
	 Step++;

   }; /* end while */

   /* Generate handle text and tack it to end of name text */
   itoa( Handle, &(ProgName[Step]), 10 );

   /* PutText into PCB */
   strcpy( PCBList[Handle].ProgramName, ProgName );

   /* Start setting the other values in the PCB */
   PCBList[Handle].InstrPointer = Region;
   PCBList[Handle].ProgSize     = ProgSize;
   PCBList[Handle].State        = NEW;
   PCBList[Handle].WaitPointer  = CoreSize; /* If it equils coresize,     */
   PCBList[Handle].TimerPointer = CoreSize; /* it is not in use           */
   PCBList[Handle].TickCount    = 0;
   PCBList[Handle].TotalClocks  = 0;
   PCBList[Handle].WhosCode     = Handle;   /* It's running it's own code */
   PCBList[Handle].CoreHeld     = ProgSize;

   /* Do the load of instructions */
   for ( Step = 0; Step < ProgSize; Step++ ) {

      /* Get the Address into the core */
	 CorePtr = CoreAddr( Region );

	 /* Did this location belong to another program? */
	 if ( CorePtr->OwnerHandl != NOHANDLE ) {

	    /* Yes!  Then decrement the previous owners CoreHeld */
	    PCBList[(CorePtr->OwnerHandl & 0x0f)].CoreHeld--;

	 }; /* end if */

	 /* Read instruction into core */
	 fread ( CorePtr, sizeof( struct Instr ), 1, InFile );
	 
	 /* Set owner of core location */
	 CorePtr->OwnerHandl = Handle;
	 
	 /* Increment region address for next read */
	 Region = CalcAddr( Region, 1 );  

   }; /* end for */

   /* Close file */
   fclose( InFile );

   /* No apparent problems doing the load! msg and return valid handle */
   BOSMsg("Program '");
   BOSMsgC( FileName );
   BOSMsgC( "' successfully loaded!\n\r");
   BOSMsg("Program '");
   BOSMsgC( FileName );
   BOSMsgC( "' assigned name : ");
   BOSMsgC( ProgName );
   BOSMsgC("\n\r");

   return( Handle );


}; /* end LoadProgram */


/* --- SimpleList -------------------------------------------------------*/
/* This function will produce a simple list of all programs loaded       */
void  SimpleList() {

   int	Step;


   /* Print the BOS ack to the command */
   BOSMsg("LIst programs : Simple list processed.\n\r\n\r");

   /* Print simple list header */
   BOSMsgC(">>> Simple list of programs loaded.\n\r");
   BOSMsgC("|Name         |State   |Next Instruction Addr\n\r");

   /* Go through PBC list and put info for each program */
   for ( Step = 0; Step < MAXPCB; Step++ ) {

	 /* Do only for loaded PCBs */
	 if ( PCBList[Step].State != UNUSED ) {

	    /* Name */
	    BOSMsgST(" %-13s ", PCBList[Step].ProgramName );

	    /* Call routine to display the state */
	    DisplayState( PCBList[Step].State );

	    /* Put next instruction */
	    BOSMsgU(" %u\r\n", PCBList[Step].InstrPointer);

	 }; /* end if */

   }; /* end for */

   /* Put character pads */
   cputs("\n\r");

}; /* end SimpleList */


/* --- NormalList -------------------------------------------------------*/
/* This function will produce a normal list of all programs loaded       */
void  NormalList() {

   int	Step;
   int    Items = 0;


   /* Print the BOS ack to the command */
   BOSMsg("LIst programs : Normal list processed.\n\r\n\r");

   /* Print list header */
   BOSMsgC(">>> List of programs loaded.\n\r");

   /* Go through PCB list and put info for each program */
   for ( Step = 0; Step < MAXPCB; Step++ ) {

	 /* Do only for loaded PCBs */
	 if ( PCBList[Step].State != UNUSED ) {

	    /* Name and size */
	    BOSMsgST(" Program Name  : %-13s   ", PCBList[Step].ProgramName );
	    BOSMsgU ("Load size: %u\n\r",		  PCBList[Step].ProgSize    );

	    /* Instruction pointer and State */
	    BOSMsgU(" Instr Pointer : %-5u           State : ",
			  PCBList[Step].InstrPointer );
	    DisplayState( PCBList[Step].State );
	    BOSMsgC("\n\r");

	    /* Clock timer pointer and tick count, if active */
	    if ( PCBList[Step].TimerPointer != CoreSize ) {
		  BOSMsgU(" Timer Pointer : %-5u", PCBList[Step].TimerPointer );
		  BOSMsgU(" TickCount : %u\r\n", 	PCBList[Step].TickCount );

	    } else
		  /* no timer */
		  BOSMsgC(" Timer Pointer : Timer not being used.\n\r");

	    /* end if */

	    /* Wait pointer and wait site, if active */
	    if ( PCBList[Step].WaitPointer != CoreSize ) {
		  BOSMsgU(" Wait Pointer  : %-5u", PCBList[Step].WaitPointer );
		  BOSMsgU("   Wait Site : %-5u\r\n",	PCBList[Step].WaitSite );

	    } else
		  /* no timer */
		  BOSMsgC(" Wait Pointer  : Wait not being used.\n\r");

	    /* end if */
         
	    /* Amount of core held */
	    BOSMsgU(" Core owned    : %-5u\n\r", PCBList[Step].CoreHeld );

	    /* Total clocks for this program */
	    BOSMsgL(" Total clocks  : %lu\n\r\n\r", PCBList[Step].TotalClocks );

	    /* See if we need to stop and let the user read */
	    Items++;
	    if ( Items == 3 ) {
		  Items = 0;
		  Continue();
	    }; /* end if */

	 }; /* end if */

   }; /* end for */

   /* Put line pads */
   BOSMsgC("\n\r");

}; /* end NormalList */


/* --- SystemList -------------------------------------------------------*/
/* Lists all the system settings								   */
void  SystemList() {

   /* Print the BOS ack to the command */
   BOSMsg("SList : System settings list processed.\n\n\r");

   /* Print simple list header */

   BOSMsgC(">>> BOS system settings.\n\r");

   /* Put core size */
   BOSMsgU(" Core Size                     : %u\n\r", CoreSize );

   /* Put minimum program load distance */
   BOSMsgS(" Minimum program load distance : %d\r\n", ProgDistance );

   /* Put maximum load trys */
   BOSMsgS(" Maximum program load tries    : %d\n\r", DistanceTrys );

   /* System clocks */
   BOSMsgL(" Total clocks executed         : %ld\n\n\r", TotalClocks );

}; /* end SystemList */


/* --- CoreList -----------------------------------------------------------*/
/* Do a list of core contents.  Arg0 will have the first location to list
   from.  Arg1 ( def 16 ) can have the number of addresses to display      */
void  CoreList() {

   long	 	  GetAddr;
   unsigned int  BaseAddr,	/* Base Addr to look at        */
			  Number;		/* Number of addresses to list */
   int		  Lines;		/* Number off lines displayed  */

   struct Instr  far *Addr;   /* Pointer into core           */
   unsigned char Byte, Byte2;


   /* Get the base address */
   GetAddr = atol( MMIList[0] );

   /* If the base address is outsize core range, ERROR! */
   if ( (GetAddr < 0 )||(GetAddr >= CoreSize) ) {
   
	 /* Not in range, Error */
	 BOSMsg("'COre list' ERROR: Invalid core address!\n\r");

	 /* DONE!, return. */
	 return;
   
   }; /* end if */

   /* Transfer to base address var */
   BaseAddr = GetAddr;

   /* Get the list length ( use GetAddr long ) */
   GetAddr  = atol( MMIList[1] );

   /* If it is 0, then default to 16.  Move it into Number var  */
   if ( GetAddr == 0 )
	 
	 /* Default to 16 */
	 Number = 16;

   else

	 /* Good number   */
	 Number = GetAddr;

   /* end if */


   /* Put the header writes */
   BOSMsg( "'COre list' : List processed.\n\r");
   BOSMsgC("\n\rADDR   | TYPE |OPTYPES|  OP1  |  OP2  |T|W| OWNER\n\r");

   /* Init lines number to 16 */
   Lines = 16;

   /* OK, ready to loop through */
   while( Number > 0 ) {

	/* -- Put the core address */
	 BOSMsgU( "%07u  ", BaseAddr );

	 /* Get the core pointer */
	 Addr = CoreAddr( BaseAddr );

	 /* Get the token */
	 Byte = Addr->Token;

	 /* Mask all but instruction */
	 Byte = Byte & 0x0F;

	/* -- Put the instruction type (token, name, whatever ) */
	 BOSMsgC( InstNames[Byte] );
	 BOSMsgC("    ");

	 /* Get the op token */
	 Byte = Addr->OpToken;

	 /* Shift down the first token and diplay it */
	 Byte = Byte >> 4;
	 DisplayOpToken( Byte );

	 /* Put the pad char */
	 BOSMsgC("-");

	 /* Get the op token */
	 Byte2 = Addr->OpToken;

	 /* Mask out the second token  and display it */
	 Byte2 = Byte2 & 0x0F;
	 DisplayOpToken( Byte2 );

	/* -- Put the first operand, if register, indicate that -- */
	 if ( Byte == REG ) {

	    switch( Addr->Operand1 ) {

		  case AREGISTER : BOSMsgC("    REG A  ");
					    break;
		  case BREGISTER : BOSMsgC("    REG B  ");
					    break;
		  case CREGISTER : BOSMsgC("    REG C  ");
					    break;
		  case AREGPTR   : BOSMsgC("    REG ^A ");
					    break;
		  case BREGPTR   : BOSMsgC("    REG ^B ");
					    break;
		  case CREGPTR   : BOSMsgC("    REG ^C ");
					    break;
	    }; /* end case */

	 } else

	    BOSMsgU( "   %6u  ", Addr->Operand1 );

	 /* end if */

	/* -- Put the second operand, if register, indicate that -- */
	 if ( Byte2 == REG ) {

	    switch( Addr->Operand2 ) {

		  case AREGISTER : BOSMsgC(" REG A  ");
					    break;
		  case BREGISTER : BOSMsgC(" REG B  ");
					    break;
		  case CREGISTER : BOSMsgC(" REG C  ");
					    break;
		  case AREGPTR   : BOSMsgC(" REG ^A ");
					    break;
		  case BREGPTR   : BOSMsgC(" REG ^B ");
					    break;
		  case CREGPTR   : BOSMsgC("REG ^C  ");
					    break;
	    }; /* end case */

	 } else

	    BOSMsgU( "%6u  ", Addr->Operand2 );

	 /* end if */

	/* -- If the instruction has a taunt put an X, otherwise a space */
	 if ( Addr->Token >= TauntPresent )

	    /* Yes, taunt */
	    BOSMsgC("X ");

	 else

	    /* No taunt!  */
	    BOSMsgC("  ");

	 /* end if */

	 /* Get the owner handle. */
	 Byte = Addr->OwnerHandl;

	/* -- If there is a wait flag, put X, otherwise space */
	 if ( Byte >= WAITFLAG ) 
	 
	    /* Yes, wait */
	    BOSMsgC("X  ");

	 else

	    /* No wait!  */
	    BOSMsgC("   ");

	 /* end if */

	 /* Clear off any possible wait flag */
	 Byte = Byte & ( 0xFF - WAITFLAG );

	/* -- If no handle put BOS, otherwise the owner name */
	 if ( Byte == NOHANDLE )
	 
	    /* No owner */
	    BOSMsgC("BOS");

	 else

	    /* Put owners name */
	    BOSMsgC(PCBList[Byte].ProgramName);

	/* -- Put return */
	 BOSMsgC("\n\r");

	 /* Next address */
	 BaseAddr = CalcAddr( BaseAddr, 1 );
	 
	 /* Decrement number lines left to do */
	 Number--;

	 /* Decrement number of lines done */
	 Lines--;

	 /* See if we need to put pause in */
	 if ( Lines == 0 ) {

	    /* Yes.  If Continue returns an [ESC] then abort rest of list */
	    if ( Continue() == 27 ) Number = 0;

	    /* Reset the lines count */
	    Lines = 16;

	    /* Put the header line again */
	    BOSMsgC("ADDR   | TYPE |OPTYPES|  OP1  |  OP2  |T|W| OWNER\n\r");


	 }; /* end if */

   }; /* end while */

   /* Done with list */
   BOSMsgC("\n\r");

}; /* end CoreList */


/* --- SetSystem -----------------------------------------------------------*/
/* Sets a variaty of system settings and parameters					 */
void  SetSystem() {

   int	SToken;	/* Setting type token */
   int	Temp;

   /* Its own token list for all the possible settings it can do         */
   #define   STAUNT	 ( 't' * CTOKENHI ) + 'a'  /* TAunt display          */
   #define   SSTAMP  ( 's' * CTOKENHI ) + 't'  /* STamp type for BOSMsg  */
   #define   SCLOCK  ( 'c' * CTOKENHI ) + 'l'  /* CLock count 		   */
   #define   SDIST   ( 'd' * CTOKENHI ) + 'i'  /* program DIstance       */
   #define   STRYS   ( 't' * CTOKENHI ) + 'r'  /* load TRys			   */
   #define   SLOG	 ( 'l' * CTOKENHI ) + 'o'  /* set LOg file options   */

   /* Create the setting type token */
   SToken = (MMIList[0][0] * CTOKENHI) + MMIList[0][1];

   /* Case through the possible setting types */
   switch ( SToken ) {

    case STAUNT  : /* Set whether to display the taunts or not */
			    /* Base on second character of second arg   */
			    switch( MMIList[1][1] ) {

				 case 'n' :
				 case 'N' : /* Turn them on */
						  BOSMsg("SET TAUNTS : Taunts set ON.\n\r");
						  DisplayTaunts = TRUE;
						  break;

				 case 'f' :
				 case 'F' : /* Turn them off */
						  BOSMsg("SET TAUNTS : Taunts set OFF.\n\r");
						  DisplayTaunts = FALSE;
						  break;

				 default  : /* What? */
						   BOSMsg("SET TAUNTS : Must be 'ON' or 'OFF'.\n\r");
						   break;

			    }; /* end case */
			    break;


    case SSTAMP  : /* Set which type of stamp to use for BOS messages */
			    /* base on first character of second arg           */
			    switch( MMIList[1][0] ) {

				 case 'b' :
				 case 'B' : /* Bare. No stamp */
						  MsgMode = BAREM;
						  BOSMsg("SET STAMP : Stamp is bare.\n\r");
						  break;

				 case 't' :
				 case 'T' : /* Put the time stamp */
						  MsgMode = TIMEM;
						  BOSMsg("SET STAMP : Stamp set to time.\n\r");
						  break;

				 case 's' :
				 case 'S' : /* Put the system count stamp */
						  MsgMode = SYSCNTM;
					BOSMsg("SET STAMP : Stamp set to system count.\n\r");
						  break;

				 default  : /* What? */
						   BOSMsg("SET STAMP : Unknown type!\n\r");
						   break;

			    }; /* end case */
			    break;


    case SCLOCK  : /* Clock set based on first character of second arg           */
			    switch( MMIList[1][0] ) {

				 case 'r' :
				 case 'R' : /* Reset the clock */
						  TotalClocks = 0;
						  BOSMsg("SET CLOCK : Clock reset to 0.\n\r");
						  break;

				 default  : /* What? */
						  BOSMsg("SET CLOCK : Unknown type!\n\r");
						  break;

			    }; /* end case */
			    break;

    case SDIST  : /* Set program load distance      */
			    Temp = atoi(MMIList[1]);	/* Get the size */

			   /* Is it valid? */
			   if ( (Temp < 20) || (Temp > (CoreSize/2)) )

				 /* No!, let the user know. */
				 BOSMsg("SET LOAD DISTANCE : Invalid distance!\n\r");

			   else {

				 /* Yes, change it and let user know */
				 ProgDistance = Temp;
				 BOSMsg("SET DISTANCE : Distance set.\r\n");

			   }; /* end if */
			   break;

    case STRYS  : /* Set program load trys      */
			    Temp = atoi(MMIList[1]);	/* Get the size */

			   /* Is it valid? */
			   if ( (Temp < 1) || (Temp > 100) )

				 /* No!, let the user know. */
				 BOSMsg("SET LOAD TRYS : Invalid number of trys!\n\r");

			   else {

				 /* Yes, change it and let user know */
				 DistanceTrys = Temp;
				 BOSMsg("SET LOAD TRYS : Number of trys set.\r\n");

			   }; /* end if */
			   break;

    case SLOG    : /* Set log options					   */
			    /* Base on second character of second arg   */
			    switch( MMIList[1][1] ) {

				 case 'n' :
				 case 'N' : /* Turn the log on.  Log must be opened */
						  if ( LogFile == NULL ) {
							/* Not open. Error */
							BOSMsg("SET LOG ON : Error! Log not open.\n\r");
						  } else {
							BOSMsg("SET LOG ON : Log set ON.\n\r");
							LogActive = TRUE;
						  };
						  break;

				 case 'f' :
				 case 'F' : /* Turn the log off.  Log must be on */
						  if ( LogActive == NULL ) {
							/* Not on. Error */
							BOSMsg("SET LOG OFF : Error! Log not on.\n\r");
						  } else {
							BOSMsg("SET LOG OFF : Log set OFF.\n\r");
							LogActive = FALSE;
						  };
						  break;

				 case 'p' :
				 case 'P' : /* Open a log file.   */
						  fopen;
						  DisplayTaunts = FALSE;
						  break;


				 default  : /* What? */
						   BOSMsg("SET TAUNTS : Must be 'ON' or 'OFF'.\n\r");
						   break;

			    }; /* end case */
			    break;

    default      : /*  Unknown set command */
			    BOSMsg("!ERROR! in SET command : Unknown set type!\n\r");


   }; /* end case */

}; /* end SetSystem */


/* NOTE: Addresses in registers are absolute!         */
/*       Addresses as memory pointers are relitive!   */


/* --- Spawn ---------------------------------------------------------------*/
/* Spawn program 											      */
void  Spawn( char  *Name  ) {

   int	PCB;

   unsigned int  Address,
			  Step;
   struct Instr  far *CAddr;


   /* Call routine to find the PCB, if no PCB, then error */
   PCB = FindProg( Name );

   /* If PCB = NOHANLDE then program is not loaded */
   if ( PCB == NOHANDLE ) {

	 /* Error */
	 BOSMsg("'SPawn' ERROR: Program ");
	 BOSMsgC(Name);
	 BOSMsgC(" not loaded!\n\r");

	 return;

   }; /* end if */

   /* Do different things for different states */
   switch( PCBList[PCB].State ) {

	 case  READY_RUN  : /* Program already spawned */
					BOSMsg("SPawn : Program ");
					BOSMsgC(Name);
					BOSMsgC(" already running.\r\n");
					break;

	 case  HELD       : /* Program held            */
					BOSMsg("SPawn : Program ");
					BOSMsgC(Name);
					BOSMsgC(" already spawned, but now HELD.\r\n");
					break;

	 case  WAIT	   : /* Program Waiting         */
					BOSMsg("SPawn : Program ");
					BOSMsgC(Name);
					BOSMsgC(" already spawned, but now WAITing.\r\n");
					break;

	 case  NEW        : /* Program can be spawned  */
					BOSMsg("SPawn : Program ");
					BOSMsgC(Name);
					BOSMsgC(" running.\r\n");
					PCBList[PCB].State = READY_RUN;
					NumProgScheduled++;
					break;

	 case  KILLED     : /* Program cannot be spawned */
					BOSMsg("SPawn : Program ");
					BOSMsgC(Name);
					BOSMsgC(" was KILLED.  Cannot be spawned\r\n");

   }; /* end case */

}; /* end Spawn */



/* --- HoldProg ------------------------------------------------------------*/
/* Hold program 											      */
void  HoldProg( char  *Name  ) {

   int	PCB;

   /* Call routine to find the PCB, if no PCB, then error */
   PCB = FindProg( Name );

   /* If PCB = NOHANLDE then program is not loaded */
   if ( PCB == NOHANDLE ) {

	 /* Error */
	 BOSMsgI("'HOld' ERROR: Program ");
	 BOSMsgC( Name );
	 BOSMsgC(" not loaded!\n\r");
	 BOSMsgIDone();

	 return;

   }; /* end if */

   /* Do different things for different states */
   switch( PCBList[PCB].State ) {


	 case  WAIT       : /* Do the hold, but for this dec number waiting */
					NumProgWait--;

	 case  READY_RUN  : /* OK. Hold it */
					BOSMsgI( "HOld : Program ");
					BOSMsgC(Name);
					BOSMsgC(" is held.");
					BOSMsgIDone();
					PCBList[PCB].State = HELD;

					/* Adjust the counts */
					NumProgScheduled--;
					NumProgHeld++;

					break;

	 case  HELD       : /* Program already held            */
					BOSMsgI( "HOld : Program ");
					BOSMsgC(Name);
					BOSMsgC(" is already HELD.");
					BOSMsgIDone();
					break;

	 case  KILLED     :
	 case  NEW        : /* Program not held  */
					BOSMsgI( "HOld : Program ");
					BOSMsgC(Name);
					BOSMsgC(" is not spawned.  Cannot hold it.");
					BOSMsgIDone();
					break;

   }; /* end case */

}; /* end HoldProg */


/* --- ReleaseProg ---------------------------------------------------------*/
/* Release program 											      */
void  ReleaseProg( char  *Name  ) {

   int	PCB;

   /* Call routine to find the PCB, if no PCB, then error */
   PCB = FindProg( Name );

   /* If PCB = NOHANLDE then program is not loaded */
   if ( PCB == NOHANDLE ) {

	 /* Error */
	 BOSMsg("'RElease' ERROR: Program ");
	 BOSMsgC( Name );
	 BOSMsgC(" not loaded!\n\r");

	 return;

   }; /* end if */

   /* If held, release it. */
   if ( PCBList[PCB].State == HELD ) {

	 /* Yes, it is held */
	 BOSMsg( "RElease : Program ");
	 BOSMsgC(Name);
	 BOSMsgC(" is released.\n\r");

	 /* Release it */
	 PCBList[PCB].State = READY_RUN;
	 NumProgScheduled++;
	 NumProgHeld--;

   } else {
	 
	 /* Otherwise, cant release it */
	 BOSMsg( "HOld : Program ");
	 BOSMsgC(Name);
	 BOSMsgC(" is not held.  It cannot be released.\n\r");

   }; /* end if */

}; /* end ReleaseProg */


/* --- CheckPoint ----------------------------------------------------------*/
/* Checkpoint the system.  Return TRUE on failure.					 */
int  CheckPoint( char  *String ) {

   FILE	  *CFile;
   char     FileName[13];

   int	  Step;


   /* Peal off the file name */
   Step = 0;
   while( (*String != '\n') && (*String != NULL) && (*String != '.') &&
		(Step < 8) ) {

		/* Tranfer char */
		FileName[Step++] = *String++;

   }; /* end while */

   /* Add extention and NULL to file name */
   FileName[Step]   = '.';
   FileName[Step+1] = 'C';
   FileName[Step+2] = 'H';
   FileName[Step+3] = 'K';
   FileName[Step+4] = NULL;

   /* Open the file, if error then report and return */
   if (( CFile = fopen( FileName, "wb" ) ) == NULL ) {

	 /* Can't open file */
	 BOSMsg("'CHeck point' ERROR: Cannot open file\n\r");

	 /* Return no handle, indicating an error */
	 return( TRUE );

   }; /* end if */

   /* Write BOS version discriptor and copyright */
   fputc( VERSIONDISC,     CFile );
   fputs( COPYRIGHTNOTICE, CFile );

   /* Save system variables */
   fwrite( &CoreSize,   sizeof( unsigned int ), 1, CFile );
   fwrite( &NumProgScheduled, sizeof( int ), 1, CFile );
   fwrite( &NumProgHeld, 	sizeof( int ), 1, CFile );
   fwrite( &NumProgWait,	     sizeof( int ), 1, CFile );
   fwrite( &TimersActive,     sizeof( int ), 1, CFile );
   fwrite( &WaitPending,      sizeof( int ), 1, CFile );
   fwrite( &WaitPending2,     sizeof( int ), 1, CFile );

   /* Save system settings */
   fwrite( &ProgDistance,  sizeof( int ), 1, CFile );
   fwrite( &DistanceTrys,  sizeof( int ), 1, CFile );
   fwrite( &MsgMode,       sizeof( int ), 1, CFile );
   fwrite( &TotalClocks,  sizeof( long ), 1, CFile );
   fwrite( &CriticalError, sizeof( int ), 1, CFile );
   fwrite( &DisplayTaunts, sizeof( int ), 1, CFile );

   /* Save all PCB that are not UNUSED.  Tag byte for position */
   for ( Step = MINPCB; Step < MAXPCB; Step++ )

	 if ( PCBList[Step].State != UNUSED ) {

		fputc( Step, CFile);
		fwrite( &(PCBList[Step]), sizeof( struct PCB ), 1, CFile );

	 }; /* end if */

   /* end for */

   /* Tag end of PCB list */
   fputc( MAXPCB, CFile);

   /* Write the core! */
   fwrite( Core, sizeof( struct Instr ), CoreSize, CFile );

   /* Close */
   fclose( CFile );

   /* Done! */
   return( FALSE );

}; /* end CheckPoint */

/* --- RestoreCP  ----------------------------------------------------------*/
/* Restore a checkpoint.  Return TRUE on failure.					      */
int  RestoreCP( char  *String ) {

   FILE	  *CFile;
   char     FileName[13];

   int	  Step;


   /* Peal off the file name */
   Step = 0;
   while( (*String != '\n') && (*String != NULL) && (*String != '.') &&
		(Step < 8) ) {

		/* Tranfer char */
		FileName[Step++] = *String++;

   }; /* end while */

   /* Add extention and NULL to file name */
   FileName[Step]   = '.';
   FileName[Step+1] = 'C';
   FileName[Step+2] = 'H';
   FileName[Step+3] = 'K';
   FileName[Step+4] = NULL;

   /* Open the file, if error then report and return */
   if (( CFile = fopen( FileName, "rb" ) ) == NULL ) {

	 /* Can't open file */
	 BOSMsg("'Check point Restore' ERROR: Cannot open file\n\r");

	 /* Return no handle, indicating an error */
	 return( TRUE );

   }; /* end if */

   Step = fgetc(CFile);
   if ( Step > VERSIONDISC ) {

	 /* Can't open file */
	 BOSMsg("'Check point Restore' ERROR: Incompatable checkpoint file.\n\r");

	 /* Close file */
	 fclose( CFile );

	 /* Return no handle, indicating an error */
	 return( TRUE );

   }; /* end if */

   /* Move past copyright notice in file, check for error */
   if ( fseek( CFile, NOTICESIZE, SEEK_CUR ) ) {  /* 1 is for the NULL */

	 /* Can't open file */
	 BOSMsg("'Check point Restore' ERROR: checkpoint file is corrupted\n\r");

	 /* Close file */
	 fclose( CFile );

	 /* Return no handle, indicating an error */
	 return( TRUE );

   };  /* end if */


   /* Kill the previous core */
   farfree( Core );

   /* Load system variables */
   fread( &CoreSize,   sizeof( unsigned int ), 1, CFile );
   fread( &NumProgScheduled, sizeof( int ), 1, CFile );
   fread( &NumProgHeld, 	    sizeof( int ), 1, CFile );
   fread( &NumProgWait,	    sizeof( int ), 1, CFile );
   fread( &TimersActive,     sizeof( int ), 1, CFile );
   fread( &WaitPending,      sizeof( int ), 1, CFile );
   fread( &WaitPending2,     sizeof( int ), 1, CFile );

   /* Save system settings */
   fread( &ProgDistance,  sizeof( int ), 1, CFile );
   fread( &DistanceTrys,  sizeof( int ), 1, CFile );
   fread( &MsgMode,       sizeof( int ), 1, CFile );
   fread( &TotalClocks,  sizeof( long ), 1, CFile );
   fread( &CriticalError, sizeof( int ), 1, CFile );
   fread( &DisplayTaunts, sizeof( int ), 1, CFile );


   /* While still PCBs, read them */
   Step = fgetc( CFile );
   while ( Step != MAXPCB ) {

	 fread( &(PCBList[Step]), sizeof( struct PCB ), 1, CFile );
	 Step = fgetc( CFile );

   }; /* end while */

   /* Build the new core.  If error, CRITICAL, KILL system! */
   Core = ( struct Instr  far * ) farcalloc( CoreSize, sizeof(struct Instr));
   if ( Core == NULL ) {

	 BOSMsg("!!!CRITICAL BOS ERROR!!!  Unable to allocate core memory!\n\r");
	 BOSMsg("BOS Aborted!\n\n\r");
	 exit(0);

   }; /* end if */

   /* Read the core! */
   fread( Core, sizeof( struct Instr ), CoreSize, CFile );

   /* Close the file */
   fclose( CFile );

   /* Done! */
   return( FALSE );

}; /* end RestoreCP */


/* --- GetValue ------------------------------------------------------------*/
/* Get the value of an operand and put it into pointer area */
struct Instr  GetValue( int  PCB, unsigned char  OpToken, int  Operand ) {

   struct Instr  Value,
			  *CAddr;

   int	Address;


   /* Case through the operand token options */
   switch ( OpToken ) {

	 case  REG	: /* Get it from register or where the register points */
				  /* Case through the different registers */
				  switch( Operand ) {
				  
					case AREGISTER : Value = PCBList[PCB].A;
								  return(Value);
					case BREGISTER : Value = PCBList[PCB].B;
								  return(Value);
					case CREGISTER : Value = PCBList[PCB].C;
								  return(Value);

					/* Indexed forms */
					case AREGPTR  : Address = PCBList[PCB].A.Operand1;
								 break;
					case BREGPTR  : Address = PCBList[PCB].B.Operand1;
								 break;
					case CREGPTR  : Address = PCBList[PCB].C.Operand1;
								 break;

				  }; /* end reg case */

				  /* If we are here, then it must be a ptr */
				  CAddr = CoreAddrC( Address );
				  Value = *CAddr;

				  break;

				  
	 case  PTR     : /* It's a pointer					  		    */
				  /* Get what is being pointed at				    */
				  Address = CalcAddr( PCBList[PCB].InstrPointer,
								  Operand );

 				  /* Get the Core Address and move data into value */
				  CAddr = CoreAddrC( Address );
				  Value = *CAddr;

				  break;


	 case  MEM     : /* Memory address */
				  /* Build a data instruction	with an operand of the */
				  /* memory address							   */

				  /* The memory address is the current address modified
					by the operand							     */
				  Address = CalcAddr( PCBList[PCB].InstrPointer,
								  Operand );

				  /* Build the instruction */
				    Value.Token      =  DAT;
				    Value.OpToken    =  0;
				    Value.Operand1	 =  Address;
				    Value.Operand2   =  Address;
				    Value.OwnerHandl =  PCB;

				  break;


	 case  IMMEDIATE : /* Build a data instruction, and make it the value */
				    Value.Token      =  DAT;
				    Value.OpToken    =  0;
				    Value.Operand1	 =  Operand;
				    Value.Operand2   =  Operand;
				    Value.OwnerHandl =  PCB;

				    break;

   }; /* end options */

   /* return the value */
   return( Value );

}; /* end GetValue */


/* --- GetAddr ------------------------------------------------------------*/
unsigned int   GetAddr( int  PCB, unsigned char  OpToken, int  Operand,
				    int  IFlag ) {
/* Extract an core address from the operand					*/

   struct Instr  *CAddr;

   int	Address,  /* will contain the final address when case is left */
		Offset;

   /* Case through the operand token options */
   switch ( OpToken ) {

	 case  REG	: /* Get it from register */
				  /* Results will only be valid if the register is an
					index type								*/
				  switch( Operand ) {
				  
					case AREGPTR : Address = PCBList[PCB].A.Operand1;
								break;
					case BREGPTR : Address = PCBList[PCB].B.Operand1;
								break;
					case CREGPTR : Address = PCBList[PCB].C.Operand1;
								break;

				  }; /* end reg case */
				  break;
				  
	 case  PTR     : /* Pointer 		   					 */
				  /* Get the address from the memory location */
				  Address = CalcAddr( PCBList[PCB].InstrPointer, Operand );

				  /* if IFlag is TRUE, do indirection */
				  if ( IFlag == TRUE ) {

				     CAddr   = CoreAddrC( Address );
					Address = CAddr->Operand1;

				  }; /* end if */

				  break;

	 case  MEM     : /* Memory address */
				  /* Address is the value in the operand modified by
				     the current address                              */  
				  /* Get the address  */
				  Address = CalcAddr( PCBList[PCB].InstrPointer, Operand );
				  break;


	 case  IMMEDIATE : /* Immediate offset */
				    /* Address is the current instruction modified
					  by the offset in the operand.  This is not technically
					  valid, but we will allow it here. 			 */

				    Address = CalcAddr( PCBList[PCB].InstrPointer, Operand);
				    break;

   }; /* end options */

   /* return the value */
   return( Address );

}; /* end GetAddr */


/* --- LoadRegister ------------------------------------------------------*/
void  LoadRegister( int  PCB, struct Instr  Value, int Register ) {
/* Put the value into a register					*/

   /* case through the registers */
   switch( Register ) {

	 case 'A' : /* A register */
			  PCBList[PCB].A = Value;
			  break;

	 case 'B' : /* B register */
			  PCBList[PCB].B = Value;
			  break;

	 case 'C' : /* C register */
			  PCBList[PCB].C = Value;

   }; /* end case */

}; /* end LoadRegister */


/* --- KillProgram ------------------------------------------------------*/
/* Do the neccesary steps to kill a program */
void  KillProgram( int PCB ) {

   struct Instr  *PutAddr;
   int 		  Step;


   /* Adjust counts as appropriate */
   if ( PCBList[PCB].State == WAIT ) --NumProgWait;
   if ( PCBList[PCB].State == HELD ) --NumProgHeld;
   if ( PCBList[PCB].State == READY_RUN ) --NumProgScheduled;

   /* OK, kill the program. */
   PCBList[PCB].State = KILLED;

   /* If a timer is set, adjust that count */
   if ( PCBList[PCB].TimerPointer != CoreSize ) --TimersActive;

   /* Clear any timer */
   PCBList[PCB].TickCount    = 0;
   PCBList[PCB].TimerPointer = CoreSize;

   /* Clear a wait, if there is one */
   if ( PCBList[PCB].WaitPointer != CoreSize ) {

   /* Yes, there is a wait.  Clear the flag from the size */
	 PutAddr = CoreAddr( PCBList[PCB].WaitPointer );
	 PutAddr->OwnerHandl = PutAddr->OwnerHandl && (0xFF - WAITFLAG);

	 /* Clear the pointer */
	 PCBList[PCB].WaitPointer = CoreSize;

   }; /* end if */

   /* Message the program's demise. Interupt */
   BOSMsgI("Program :");
   BOSMsgC( PCBList[PCB].ProgramName );
   BOSMsgC(" KILLED by BOS.");
   BOSMsgIDone();

   /* OK, is there only one active ( run or wait ) battle program */
   if ( (NumProgScheduled + NumProgWait) == 1 ) {

	 /* Yes, find who it is */
	 Step = 0;
	 while ((PCBList[Step].State != READY_RUN)&&
		   (PCBList[Step].State != WAIT)        ) {
	    Step++;
	 };

	 /* Now, is this an outright win, or are other programs held */
	 if ( NumProgHeld ) {

	    /* Other programs are held, so just hold this one */
	    BOSMsgI("Program :");
	    BOSMsgC( PCBList[Step].ProgramName );
	    BOSMsgC(" is the only RUNNING or WAITING program.");
	    BOSMsgIDone();

	 } else {

	    /* Out right win! */
	    BOSMsgI("Program :");
	    BOSMsgC( PCBList[Step].ProgramName );
	    BOSMsgC(" is the only surviving program!");
	    BOSMsgIDone();

	 }; /* end if */

	 /* Hold it, no matter what. */
	 HoldProg( PCBList[Step].ProgramName );

   }; /* end if */

}; /* end if */

/* --- ClearProgram ------------------------------------------------------*/
/* Clear a program from memory */
void  ClearProgram( char*  Name ) {

   int  			PCB;
   unsigned int     Address;
   struct Instr    *CAddr,
				Dummy;

   /* First find the name */
   PCB = FindProg( Name );

   /* Is it a valid program? */
   if ( PCB == NOHANDLE ) {

	 /* NO, error! */
	 BOSMsg("'CLear' ERROR: Program ");
	 BOSMsgC( Name );
	 BOSMsgC(" not loaded!\n\r");

	 return;


   }; /* end if */

   /* Build dummy */
   Dummy.Token      = DAT;
   Dummy.OpToken    = 0;
   Dummy.Operand1   = 0;
   Dummy.Operand2   = 0;
   Dummy.OwnerHandl = NOHANDLE;

   /* It must be new or killed */
   if ( (PCBList[PCB].State != NEW)&&(PCBList[PCB].State != KILLED) ) {

	 /* It is NOT, error! */
	 BOSMsg("'CLear' ERROR: Program ");
	 BOSMsgC( PCBList[PCB].ProgramName );
	 BOSMsgC(" not NEW or KILLED!\n\r");

	 return;

   }; /* end if */

   /* Build dummy */
   Dummy.Token      = DAT;
   Dummy.OpToken    = 0;
   Dummy.Operand1   = 0;
   Dummy.Operand2   = 0;
   Dummy.OwnerHandl = NOHANDLE;

   /* OK, mark its PCB as UNUSED */
   PCBList[PCB].State = UNUSED;

   /* OK, run the core and clear all it's memory */
   for ( Address = 0; Address < CoreSize; ++Address ) {

	 /* Get the address */
	 CAddr = CoreAddr( Address );

	 /* Does it belong to this PCB */
	 if ( CAddr->OwnerHandl == PCB )

	    /* Yes!, then return it to BOS */
	    *CAddr = Dummy;

	 /* end if */

   }; /* end for */

}; /* end ClearProgram */


/* --- DiskList --------------------------------------------------*/
/* Do an undetailed list of BOS programs on disk */
void  DiskList ( void ) {

   struct ffblk  fb;
   int           done;
   unsigned int  play;

   /* Put the processing header */
   BOSMsg("'DIsk list' : List processed.\n\n\r");
   BOSMsgC(" |- File Name -|-  Size  -|-   Date   -\n\r");

   /* Find the first */
   done = findfirst( "*.BOS", &fb, 0 );
   while( !done ) {

	 BOSMsgST( " %13s ", fb.ff_name );
	 BOSMsgL(  "%9ld    ", fb.ff_fsize   );

	 /* Do the date and time */
	 play =  (fb.ff_fdate >> 5) & 15;  /* Get the Month */
	 BOSMsgU( "%02u/", play );
	 play =  fb.ff_fdate & 31;  		/* Get the DAY */
	 BOSMsgU( "%02u/", play );
	 play =  fb.ff_fdate >> 9;         /* Get the Year 	 */
	 play += 1980;					/* Add the base year */
	 BOSMsgU( "%4u|", play );

	 play =  fb.ff_ftime >> 11;        /* Get the Hour 	 */
	 BOSMsgU( "%02u:", play );
	 play =  (fb.ff_ftime >> 5) & 63;  /* Get the Min */
	 BOSMsgU( "%02u\n\r", play );

	 done = findnext( &fb );

   }; /* end while */


}; /* end DiskList */


/* -------- MASTER Routines ----------------------------------------------*/

/* --- CreateCore --------------------------------------------------*/
/* This func will allocate the BOS core and store it in the global
   pointer.  It return the memory used to create the core			   */
unsigned int  CreateCore ( char *Size ) {

   unsigned int  SizeI;
   unsigned int  SizeB;

   SizeI = atoi( Size );	/* Get the size in instructions */

   /* Determine if Size is to long or to small*/
   if ( (SizeI < MINCORESPEC) || (SizeI > MAXCORESPEC) ) {
	 MsgBadCoreSpec();
	 exit(0);
   };

   /* Determine the number of bytes that will be needed and allocate it */
   SizeB = SizeI * sizeof( struct Instr );
   Core  = ( struct Instr  * ) farmalloc( SizeB );

   /* If Core is null, notify user and exit */
   if ( Core == NULL ) {
	 MsgNoCore();
	 exit(0);
   };

   /* Remember core size in instructions */
   CoreSize = SizeI;

   /* return bytes used */
   return ( SizeB );

}; /* end CreateCore */


/* -------- DoCommand ----------------------------------------------------*/
/* This function will interpret and dispatch a MMI command.			    */
int	DoCommand( unsigned int	Token ) {

int  Step;


/* --- Commands implemented ---

  COMMAND  CHARS TOKEN      DISCRIPTION

  exit     ex    CEXIT      Exit BOS

  disk	 di	  CDISK	   List available .BOS programs.

  load	 lo	  CLOAD	   Load a program.  Do not spawn.

  list     li    CLIST      List programs in core

  slist    sl    CSLIST     List system settings.

  set      se    CSET       Set system paramaters

  core     co    CCORE      Look into the core.  Undetailed list.

  spawn    sp    CSPAWN     Spawn a program ( run it ).

  hold     ho    CHOLD      Hold a program.  Put it in HELD state.

  release  re    CRELEASE   Release a program from a held state.
  
  check    ch    CCHECKP    Checkpoint the the system.

  crestore cr    CCRESTORE  Checkpoint restore.

  clear    cl    CCLEAR     Clear a program or the entire core

*/

   /* Case through possible tokens */
   switch ( Token ) {

	 case  CEXIT   : /* Exit BOS, return FALSE */
				  return( FALSE );


	 case  CLOAD   : /* Load each program on arg list */
				  for ( Step = 0; Step < MMICmndArgs; Step++ ) {

					LoadProgram( MMIList[Step] );

				  }; /* end for */
				  break;

	 case  CLIST   : /* List programs loaded.  's' in the first arg
					means a simple list is desired			*/
				  if ( MMIList[0][0] == 's' )

					/* Quick list */
					SimpleList();

				  else

					/* Normal list */
                         NormalList();


				  /* end if */
				  break;

	 case  CSLIST  : /* List the system definitions */
				  SystemList();
				  break;

	 case  CSET    : /* Do the set */
				  SetSystem();
				  break;

	 case  CCORE   : /* Do a simple core list */
				  CoreList();
				  break;

	 case  CSPAWN  : /* Spawn program  */

				  /* First, is it an '-a' command */
				  if ((MMIList[0][0] == '-')&&(MMIList[0][1] == 'a')) {

					/* Yes, loop and do all new */
					for ( Step = 0; Step < MAXPCB; Step ++ )
					   if ( PCBList[Step].State == NEW )
					      Spawn( PCBList[Step].ProgramName ); 

				  } else

					/* No, then do all on the list */
					for ( Step = 0; Step < MMICmndArgs; Step++ )
					   Spawn( MMIList[Step] );

				  break;

	 case  CHOLD   : /* Hold program */

				  /* First, is it an '-a' command */
				  if ((MMIList[0][0] == '-')&&(MMIList[0][1] == 'a')) {
                      
					/* Yes, loop and do all new */
					for ( Step = 0; Step < MAXPCB; Step ++ )
					   if ( PCBList[Step].State == READY_RUN )
					      HoldProg( PCBList[Step].ProgramName ); 

				  } else

                         /* No, then do all on the list */
					for ( Step = 0; Step < MMICmndArgs; Step++ )
					   HoldProg( MMIList[Step] );

				  /* Clean the BOS input line */
				  BOSMsgClean();

				  break;

	 case  CRELEASE  : /* Release program */

				  /* First, is it an '-a' command */
				  if ((MMIList[0][0] == '-')&&(MMIList[0][1] == 'a')) {

					/* Yes, loop and do all new */
					for ( Step = 0; Step < MAXPCB; Step ++ )
					   if ( PCBList[Step].State == HELD )
						 ReleaseProg( PCBList[Step].ProgramName );

				  } else

					/* No, then do all on the list */
					for ( Step = 0; Step < MMICmndArgs; Step++ )
					   ReleaseProg( MMIList[Step] );

				  break;

	 case  CSHOLD  : /* Spawn the hold each program on arg list */
				  for ( Step = 0; Step < MMICmndArgs; Step++ ) {

					Spawn( MMIList[Step] );
					HoldProg( MMIList[Step] );

				  }; /* end for */
				  break;

	 case  CCHECKP : /* Check point the program.  TRUE is error */
				  if ( CheckPoint( MMIList[0] ) )

					BOSMsg("Unable to perform checkpoint.\n\r");

				  else

					BOSMsg("Checkpoint successfull.\n\r");

				  /* end if */
				  break;


	 case  CCRESTORE : /* Restore checkpoint.  TRUE is error */
				  if ( RestoreCP( MMIList[0] ) )

					BOSMsg("Unable to restore checkpoint.\n\r");

				  else

					BOSMsg("Checkpoint restore successfull.\n\r");

				  /* end if */
				  break;


	 case  CCLEAR    : /* Clear programs */

				  /* First, is it an '-a' command */
				  if ((MMIList[0][0] == '-')&&(MMIList[0][1] == 'a')) {

					/* Yes, then clear the entire core and PCBList */
					for ( Step = 0; Step < MAXPCB; Step ++ )
					   PCBList[Step].State = UNUSED;
					ClearCore();

				  } else

					/* No, then clear all programs on the list */
					for ( Step = 0; Step < MMICmndArgs; Step++ )
					   ClearProgram( MMIList[Step] );

				  break;

	 case  CDISK     : /* Disk list. */
				    DiskList();

				    break;


	 default       : /* All other tokens all errors */
				  MsgBadCommand();

   }; /* end switch */

   /* return under normal conditions */
   return ( TRUE );

}; /* end DoCommand */


/* -------- --------------------------------------------------*/
/* Get and implement critical command */
int  CritCmndEntry( void ) {

   char	Buffer[MAXCMDSIZE + 2];
   int    Step,
		Continue = TRUE;
   unsigned int	CommandToken;

   cputs("\n\r");	/* Go to next line */
   MMICmndPtr = 0;	/* Kill previous entry 		 */

   /* Set the max chars */
   Buffer[0] = MAXCMDSIZE;

   /* Get the Buffer */
   cputs("!BOS!>>");
   cgets( Buffer );
   cputs("\n\r");

   /* Stuff into the MMI Command */
   Step = 0;
   while ( Buffer[Step+2] != NULL ) {
	 MMICmnd[Step] = Buffer[Step+2];
	 ++Step;
   };
   MMICmnd[Step] = NULL;

  /* -- Do the command */

   /* if the command parses w/o error(TRUE), then do  */
   CommandToken = ParseCommand();
   if ( CommandToken != NOCMDTOKEN )
	 Continue = DoCommand( CommandToken );

   /* Clear old command and put new prompt */
   MMICmndPtr = 0;
   BOSPrompt();

   /* NULL the MMIList */
   for ( Step = 0; Step < MAXARGS; Step++ )
	 MMIList[Step][0] = NULL;

   /* Return the Continue state */
   return Continue;

}; /* end CritCmndEntry */


/* -------- DoKeyPress ---------------------------------------------------*/
/* This function processes a key press.  If [ENTER] has been pressed then
   it sends the command string to DoCommand						    */
int  DoKeyPress() {

   /* defines used by this function */
   #define  ENTER   	13	/* The [ENTER] key     				*/
   #define  BACKSPACE    8    /* The [BACKSPACE] key 				*/
   #define  FUNCTIONKEY  0    /* For when function keys are pressed   */
   #define  ESCAPE		27   /* The [ESC] key					*/
   #define  ABORT		'`'  /* The abort key 					*/


   char			InKey, Continue = TRUE;
   unsigned int	CommandToken, Step;

   /* Get the key from BIOS */
   InKey = getch();

   /* case for significant keystrokes */
   switch( InKey ) {

    case ENTER 	: /* NULL terminate the command string and execute */
				  cputs("\n\r");
				  MMICmnd[MMICmndPtr] = NULL;

				  /* if the command parses w/o error(TRUE), then do  */
				  CommandToken = ParseCommand();
				  if ( CommandToken != NOCMDTOKEN )
					Continue = DoCommand( CommandToken );

				  /* Clear old command and put new prompt */
				  MMICmndPtr = 0;
				  BOSPrompt();

				  /* NULL the MMIList */
				  for ( Step = 0; Step < MAXARGS; Step++ )
					MMIList[Step][0] = NULL;
				  break;

    case BACKSPACE  : /* If we can move back, do so */
				  if ( MMICmndPtr > 0 ) {
					/* Process for a destructive backspace */
					MMICmnd[MMICmndPtr] = NULL;
					MMICmndPtr--;		/* Kill last char in buffer */

					putchar( InKey );	/* Move cursor back one	   */
					putchar( ' ' );	/* Destroy char on screen   */
					putchar( InKey );   /* Move back again		   */
				  };
				  break;

    case FUNCTIONKEY : /* Currently no function key processing, so just
					burn the key							   */
				   InKey = getch();
				   break;

    case ESCAPE	 : /*  Abort current command and do critical command */
				   Continue = CritCmndEntry();
				   break;


    case ABORT		 : /*  Abort current command  */
				   cputs("\n\r");	/* Go to next line */
				   BOSMsg("Command entry aborted.\n\r");
				   MMICmndPtr = 0;	/* Kill previous entry 		 */
				   BOSPrompt();     /* Put the prompt for the next */
				   break;


		   default : /* Add the key to the Command if it will fit */
				   if ( MMICmndPtr < MAXCMDSIZE ) {

					 /* It will fit */
					 MMICmnd[MMICmndPtr] = InKey;
					 MMICmndPtr++;
					 putch( InKey );	/* Echo the key */

				   };
				   break;


   }; /* end case */

   return( Continue );

}; /* end DoKeyPress */


/* -------- ExecInstruction  ---------------------------------------------*/
/* This executes an instruction for a particular PCB				    */
void  ExecInstruction ( int	PCB ) {

   struct Instr     far  *Instruction,
				far  *PutAddr,
				ValueR,		/* Value Right and Left ( Operands ) */
				ValueL;

   unsigned char	OpToken1, OpToken2, Handle, Token;
   unsigned int     Address, AddressV;



   /* Clear critical error flag */
   CriticalError = NOERROR;

   /* First, get the pointer to the instruction */
   Instruction = CoreAddrC( PCBList[PCB].InstrPointer );

   /* Get the token */
   Token = Instruction->Token;

   /* Does this instruction have a taunt */
   if ( Token >= TauntPresent ) {

	/* Yes, then put it and clean the token */

	 /* Mask out the Taunt number and shift it down */
	 Token = Token & 0x70;   /* Taunt number is in bits 64, 32, and 16 */
	 Token = Token >> 4;     /* Shift it to bits 4, 2, and 1           */

	 /* Print the taunt, if taunt diplaying is active */
	 if ( DisplayTaunts == TRUE )
	   BOSMsgTaunt( PCBList[PCB].ProgramName, PCBList[PCB].TauntList[Token] );

	 /* Load and clean the Token */
	 Token = Instruction->Token;
	 Token = Token & 0x0F;       /* 4 LSBs contain the actual token */

   }; /* end if */

   /* Are we running a different program's code? */
   if ( (Instruction->OwnerHandl != PCBList[PCB].WhosCode)&&
	   (Token != DAT) ) {    /* Don't worry about DAT instructions */

	 /* Yes, well log whos, and let the user know. */
	 BOSMsgI("Program :");
	 BOSMsgC(PCBList[PCB].ProgramName );
	 BOSMsgC(": is running the code of program ");
	 BOSMsgC(PCBList[Instruction->OwnerHandl].ProgramName);
	 BOSMsgIDone();

	 PCBList[PCB].WhosCode = Instruction->OwnerHandl;

   }; /* end if */
   /* NOTE: Running someone elses code does not transfer owner ship of that
		  code to the current PCB								*/

   /* Case through all instructions and act on them */
   switch( Token ) {

   /* -   -   -   -   -   -  MOV  -   -   -   -   -   -   -   -   -   -  */
	 case MOV : /* Do the move instruction */

			  /* Extract the value and address */
			  OpToken2    = Instruction->OpToken & 0x0F;
			  ValueR      = GetValue( PCB, OpToken2, Instruction->Operand2 );
   			  OpToken1    = Instruction->OpToken >> 4;

			  /* See if ValueR belonged to someone else */
			  if ( ValueR.OwnerHandl != PCB )

				/* if so, make this PCB the owner */
				ValueR.OwnerHandl = PCB;

			  /* Are we moving into a pure register? */
			  if ( (OpToken1 == REG)&&(Instruction->Operand1 < REGPTRBIT) )

				/* Ok, then do it.  */
				LoadRegister( PCB, ValueR, Instruction->Operand1);

			  else {

				/* Otherwise, we need to find the address to put it */
				Address     = GetAddr( PCB, OpToken1, Instruction->Operand1,
								   FALSE );

				/* Calcutate the memory address  */
				PutAddr     = CoreAddrC( Address );

				/* See if we are putting this Value on someone elses spot */
				if (PutAddr->OwnerHandl != PCB) {

				   /* Are we taking BOSs memory? */
				   if (PutAddr->OwnerHandl == NOHANDLE) 

					 /* Yes!, then give it to this pcb */
					 PCBList[PCB].CoreHeld++;

				   else {
				   
					 /* Otherwise, give credit to this PCB and take
					    credit from old owner					 */
					 PCBList[PCB].CoreHeld++;
					 PCBList[PutAddr->OwnerHandl].CoreHeld--;

				   }; /* end if */

 				}; /* end if */


				/* Move the value */
				*PutAddr    = ValueR;

			  }; /* end if */

			  /* Next instruction */
			  PCBList[PCB].InstrPointer =
			     CalcAddr(PCBList[PCB].InstrPointer, 1 );

                 break;


   /* -   -   -   -   -   -  CMP  -   -   -   -   -   -   -   -   -   -  */
	 case CMP : /* Do a compare instruction */

			  /* Extract the value of both sides */
			  OpToken2    = Instruction->OpToken & 0x0F;
			  ValueR      = GetValue( PCB, OpToken2, Instruction->Operand2 );
			  OpToken1    = Instruction->OpToken >> 4;
			  ValueL      = GetValue( PCB, OpToken1, Instruction->Operand1 );

			  /* If both are code then do straight compare, otherwise
				compare just the first operand					*/
			  if ( (ValueR.Token != DAT)&&(ValueL.Token != DAT) )

				/* -- Both code, do straight compare */
				PCBList[PCB].CMPResult =
				   memcmp( &ValueL, &ValueR, sizeof( struct Instr ) );

			  else

				/* -- One is data, compare the first operand by subtracting
					 the left from the right		  */
				PCBList[PCB].CMPResult =
				   ValueL.Operand1 - ValueR.Operand1;

			  /* end if */

			  /* Next instruction */
			  PCBList[PCB].InstrPointer =
			     CalcAddr(PCBList[PCB].InstrPointer, 1 );

                 break;


   /* -   -   -   -   -   -  JE   -   -   -   -   -   -   -   -   -   -  */
	 case JE  : /* Do a jump on equil instruction */

			  /* We will take this jump if this PCBs compare result is 0 */
			  if ( PCBList[PCB].CMPResult == 0 ) {

				/* -- Take jump */

				/* Extract the address from left sides */
				OpToken1  = Instruction->OpToken >> 4;
				Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
								 TRUE );

				/* Update the instruction pointer */
				PCBList[PCB].InstrPointer = Address;

			  } else

				/* Otherwise just go to the next instruction */
				PCBList[PCB].InstrPointer =
				   CalcAddr(PCBList[PCB].InstrPointer, 1 );

                 break;

   /* -   -   -   -   -   -  JB   -   -   -   -   -   -   -   -   -   -  */
	 case JB  : /* Do a jump on below instruction */

			  /* We will take this jump if this PCBs compare result is - */
			  if ( PCBList[PCB].CMPResult < 0 ) {

				/* -- Take jump */

				/* Extract the address from left sides */
				OpToken1  = Instruction->OpToken >> 4;
				Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
								 TRUE );

				/* Update the instruction pointer */
				PCBList[PCB].InstrPointer = Address;

			  } else

				/* Otherwise just go to the next instruction */
				PCBList[PCB].InstrPointer =
				   CalcAddr(PCBList[PCB].InstrPointer, 1 );

                 break;

   /* -   -   -   -   -   -  JA   -   -   -   -   -   -   -   -   -   -  */
	 case JA  : /* Do a jump on above instruction */

			  /* We will take this jump if this PCBs compare result is + */
			  if ( PCBList[PCB].CMPResult > 0 ) {

				/* -- Take jump */

				/* Extract the address from left sides */
				OpToken1  = Instruction->OpToken >> 4;
				Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
								 TRUE );

				/* Update the instruction pointer */
				PCBList[PCB].InstrPointer = Address;

			  } else

				/* Otherwise just go to the next instruction */
				PCBList[PCB].InstrPointer =
				   CalcAddr(PCBList[PCB].InstrPointer, 1 );

                 break;


   /* -   -   -   -   -   -  JMP  -   -   -   -   -   -   -   -   -   -  */
	 case JMP : /* Do an unconditional jump */

			  /* Extract the address from left sides */
			  OpToken1  = Instruction->OpToken >> 4;
			  Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
							   TRUE );

			  /* Update the instruction pointer */
			  PCBList[PCB].InstrPointer = Address;

                 break;


   /* -   -   -   -   -   -  INC  -   -   -   -   -   -   -   -   -   -  */
	 case INC : /* Increment the first operand */

			  /* Extract the value and the address of the first operand */
			  OpToken1  = Instruction->OpToken >> 4;
			  Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
							   FALSE );
			  ValueL    = GetValue( PCB, OpToken1, Instruction->Operand1 );

			  /* Treat the value as an address and increment it */
			  ValueL.Operand1 = CalcAddr( ValueL.Operand1, 1 );

			  /* See if ValueR belonged to someone else */
			  if ( ValueR.OwnerHandl != PCB )

				/* if so, make this PCB the owner */
				ValueR.OwnerHandl = PCB;

			  /* -- Put the value back */
			  /* Are we moving into a pure register? */
			  if ( ( OpToken1 == REG )&&(Instruction->Operand1 < REGPTRBIT) )

				/* Ok, then do it.  */
				LoadRegister( PCB, ValueL, Instruction->Operand1);

			  else {

				/* Calcutate the memory address  */
				PutAddr     = CoreAddrC( Address );

				/* See if we are putting this Value on someone elses spot */
				if (PutAddr->OwnerHandl != PCB) {

				   /* Are we taking BOSs memory? */
				   if (PutAddr->OwnerHandl == NOHANDLE) 

					 /* Yes!, then give it to this pcb */
					 PCBList[PCB].CoreHeld++;

				   else {
				   
					 /* Otherwise, give credit to this PCB and take
					    credit from old owner					 */
					 PCBList[PCB].CoreHeld++;
					 PCBList[PutAddr->OwnerHandl].CoreHeld--;

				   }; /* end if */

 				}; /* end if */

				/* Move the value */
				*PutAddr    = ValueL;

			  }; /* end if */

			  break;


   /* -   -   -   -   -   -  DEC  -   -   -   -   -   -   -   -   -   -  */
	 case DEC : /* Decrement the first operand */

			  /* Extract the value and the address of the first operand */
			  OpToken1  = Instruction->OpToken >> 4;
			  Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
							   FALSE );
			  ValueL    = GetValue( PCB, OpToken1, Instruction->Operand1 );

			  /* See if ValueR belonged to someone else */
			  if ( ValueR.OwnerHandl != PCB )

				/* if so, make this PCB the owner */
				ValueR.OwnerHandl = PCB;

			  /* Treat the value as an address and decrement it */
			  ValueL.Operand1 = CalcAddr( ValueL.Operand1, -1 );

			  /* -- Put the value back */
			  /* Are we moving into a pure register? */
			  if ( ( OpToken1 == REG )&&(Instruction->Operand1 < REGPTRBIT) )

				/* Ok, then do it.  */
				LoadRegister( PCB, ValueL, Instruction->Operand1);

			  else {

				/* Calcutate the memory address  */
				PutAddr     = CoreAddrC( Address );

				/* See if we are putting this Value on someone elses spot */
				if (PutAddr->OwnerHandl != PCB) {

				   /* Are we taking BOSs memory? */
				   if (PutAddr->OwnerHandl == NOHANDLE) 

					 /* Yes!, then give it to this pcb */
					 PCBList[PCB].CoreHeld++;

				   else {
				   
					 /* Otherwise, give credit to this PCB and take
					    credit from old owner					 */
					 PCBList[PCB].CoreHeld++;
					 PCBList[PutAddr->OwnerHandl].CoreHeld--;

				   }; /* end if */

 				}; /* end if */

				/* Move the value */
				*PutAddr    = ValueL;

			  }; /* end if */

			  break;


   /* -   -   -   -   -   -  ADD  -   -   -   -   -   -   -   -   -   -  */
	 case ADD : /* Do an add instruction */

			  /* Extract the value of both sides and the address
				of the left side							*/
			  OpToken2    = Instruction->OpToken & 0x0F;
			  ValueR      = GetValue( PCB, OpToken2, Instruction->Operand2 );
			  OpToken1    = Instruction->OpToken >> 4;
			  ValueL      = GetValue( PCB, OpToken1, Instruction->Operand1 );
			  Address     = GetAddr(  PCB, OpToken1, Instruction->Operand1,
								 FALSE );

			  /* See if ValueR belonged to someone else */
			  if ( ValueR.OwnerHandl != PCB )

				/* if so, make this PCB the owner */
				ValueR.OwnerHandl = PCB;

			  /* Treat the values as addresses and add them */
			  ValueL.Operand1 = CalcAddr( ValueL.Operand1, ValueR.Operand1 );

			  /* Put into the compare result */
			  PCBList[PCB].CMPResult = ValueL.Operand1;

			  /* -- Put the left value back */
			  /* Are we moving into a pure register? */
			  if ( ( OpToken1 == REG )&&(Instruction->Operand1 < REGPTRBIT) )

				/* Ok, then do it.  */
				LoadRegister( PCB, ValueL, Instruction->Operand1);

			  else {

				/* See if we are putting this Value on someone elses spot */
				if (PutAddr->OwnerHandl != PCB) {

				   /* Are we taking BOSs memory? */
				   if (PutAddr->OwnerHandl == NOHANDLE) 

					 /* Yes!, then give it to this pcb */
					 PCBList[PCB].CoreHeld++;

				   else {
				   
					 /* Otherwise, give credit to this PCB and take
					    credit from old owner					 */
					 PCBList[PCB].CoreHeld++;
					 PCBList[PutAddr->OwnerHandl].CoreHeld--;

				   }; /* end if */

 				}; /* end if */

				/* Calcutate the memory address  */
				PutAddr     = CoreAddrC( Address );

				/* Move the value */
				*PutAddr    = ValueL;

			  }; /* end if */

			  /* Next instruction */
			  PCBList[PCB].InstrPointer =
			     CalcAddr(PCBList[PCB].InstrPointer, 1 );

                 break;


   /* -   -   -   -   -   -  SUB  -   -   -   -   -   -   -   -   -   -  */
	 case SUB : /* Do a subtract instruction */

			  /* Extract the value of both sides and the address
				of the left side							*/
			  OpToken2    = Instruction->OpToken & 0x0F;
			  ValueR      = GetValue( PCB, OpToken2, Instruction->Operand2 );
			  OpToken1    = Instruction->OpToken >> 4;
			  ValueL      = GetValue( PCB, OpToken1, Instruction->Operand1 );
			  Address     = GetAddr(  PCB, OpToken1, Instruction->Operand1,
								 FALSE );

			  /* See if ValueR belonged to someone else */
			  if ( ValueR.OwnerHandl != PCB )

				/* if so, make this PCB the owner */
				ValueR.OwnerHandl = PCB;

			  /* Treat the values as addresses and subtract right
				from left */
			  ValueL.Operand1 = CalcAddr(  ValueL.Operand1,
								    -(ValueR.Operand1) );

			  /* Put into the compare result */
			  PCBList[PCB].CMPResult = ValueL.Operand1;

			  /* -- Put the left value back */
			  /* Are we moving into a pure register? */
			  if ( ( OpToken1 == REG )&&(Instruction->Operand1 < REGPTRBIT) )

				/* Ok, then do it.  */
				LoadRegister( PCB, ValueL, Instruction->Operand1);

			  else {

				/* Calcutate the memory address  */
				PutAddr     = CoreAddrC( Address );

				/* See if we are putting this Value on someone elses spot */
				if (PutAddr->OwnerHandl != PCB) {

				   /* Are we taking BOSs memory? */
				   if (PutAddr->OwnerHandl == NOHANDLE) 

					 /* Yes!, then give it to this pcb */
					 PCBList[PCB].CoreHeld++;

				   else {
				   
					 /* Otherwise, give credit to this PCB and take
					    credit from old owner					 */
					 PCBList[PCB].CoreHeld++;
					 PCBList[PutAddr->OwnerHandl].CoreHeld--;

				   }; /* end if */

 				}; /* end if */

				/* Move the value */
				*PutAddr    = ValueL;

			  }; /* end if */

			  /* Next instruction */
			  PCBList[PCB].InstrPointer =
				CalcAddr(PCBList[PCB].InstrPointer, 1 );

			  break;


   /* -   -   -   -   -   -  LOP  -   -   -   -   -   -   -   -   -   -  */
	 case LOP : /* Do a loop instruction */
			  /* Decrement REG C.  If 0 or negative, then jump to
				operand1									   */


			  /* Do the decrement, and see if greater than 0 */
			  if ( (PCBList[PCB].C.Operand1-- ) > 0 ) {

				/* -- Yes, do loop */

				/* Extract the address */
				OpToken1    = Instruction->OpToken >> 4;
				Address     = GetAddr( PCB, OpToken1, Instruction->Operand1,
								   TRUE );

				/* Update the instruction pointer */
				PCBList[PCB].InstrPointer = Address;

			  } else

				/* -- No, then fall out of loop.  Go to next instruction */
			     PCBList[PCB].InstrPointer =
			        CalcAddr(PCBList[PCB].InstrPointer, 1 );

			  /* end if */
			  break;


   /* -   -   -   -   -   -  RND  -   -   -   -   -   -   -   -   -   -  */
	 case RND : /* Do a random gereration instruction */
			  /* Generate a random number between 0 and coresize-1.   */

			  /* Build the right value */
			  ValueR.Token    = DAT;
			  ValueR.OpToken  = 0;	/* Doesnt matter, it's data. */
			  ValueR.Operand1 = random( CoreSize );
			  ValueR.Operand2 = ValueR.Operand1;
			  ValueR.OwnerHandl = PCB;

			  /* Now, find out where it will go */
 			  OpToken1  = Instruction->OpToken >> 4;
			  Address   = GetAddr( PCB, OpToken1, Instruction->Operand1,
							   FALSE );
			  ValueL    = GetValue( PCB, OpToken1, Instruction->Operand1 );

			  /* See if ValueR belonged to someone else */
			  if ( ValueR.OwnerHandl != PCB )

				/* if so, make this PCB the owner */
				ValueR.OwnerHandl = PCB;

			  /* -- Put the value into it */
			  /* Are we moving into a pure register? */
			  if ( ( OpToken1 == REG )&&(Instruction->Operand1 < REGPTRBIT) )

				/* Ok, then do it.  */
				LoadRegister( PCB, ValueR, Instruction->Operand1);

			  else {

				/* Calcutate the memory address  */
				PutAddr     = CoreAddrC( Address );

				/* See if we are putting this Value on someone elses spot */
				if (PutAddr->OwnerHandl != PCB) {

				   /* Are we taking BOSs memory? */
				   if (PutAddr->OwnerHandl == NOHANDLE) 

					 /* Yes!, then give it to this pcb */
					 PCBList[PCB].CoreHeld++;

				   else {
				   
					 /* Otherwise, give credit to this PCB and take
					    credit from old owner					 */
					 PCBList[PCB].CoreHeld++;
					 PCBList[PutAddr->OwnerHandl].CoreHeld--;

				   }; /* end if */

 				}; /* end if */

				/* Move the value */
				*PutAddr    = ValueR;

			  }; /* end if */
                 break;


   /* -   -   -   -   -   -  WT  -   -   -   -   -   -   -   -   -   -  */
	 case WT  : /* Do a wait set instruction */
			  /* First operand is the wait site, second is jump loc  */

			  /* Get the value and address of the location we will be
				putting the wait at							  */
			  OpToken1  = Instruction->OpToken >> 4;
			  OpToken2  = Instruction->OpToken & 0x0F;
			  Address   = GetAddr(  PCB, OpToken1, Instruction->Operand1,
							    TRUE  );
			  PutAddr   = CoreAddrC( Address );
			  ValueL    = *PutAddr;

			  /* See if this spot belonged to someone else */
			  if (ValueL.OwnerHandl != PCB) {

				/* Are we taking BOSs memory? */
				if (ValueL.OwnerHandl == NOHANDLE)

				   /* Yes!, then give it to this pcb */
				   PCBList[PCB].CoreHeld++;

				else {
    
				   /* Otherwise, give credit to this PCB and take
				   credit from old owner					 */
				   PCBList[PCB].CoreHeld++;
				   PCBList[ValueL.OwnerHandl].CoreHeld--;

				}; /* end if */

				/* But this PCBs handle into it */
				ValueL.OwnerHandl = PCB;

			  }; /* end if */

			  /* Add wait flag to it */
			  ValueL.OwnerHandl += WAITFLAG;

			  /* OK, put it back. */
			  *PutAddr = ValueL;

			  /* Put the wait address into the PCB */
			  PCBList[PCB].WaitSite = Address;

			  /* Now get the jump location */
			  Address   = GetAddr(  PCB, OpToken2, Instruction->Operand2,
							    TRUE );

			  /* Put the jump location into the PCB */
			  PCBList[PCB].WaitPointer = Address;

			  /* Next instruction */
			  PCBList[PCB].InstrPointer =
				CalcAddr(PCBList[PCB].InstrPointer, 1 );

			  /* DONE! */
			  break;


   /* -   -   -   -   -   -  TMR  -   -   -   -   -   -   -   -   -   -  */
	 case TMR : /* Set a timer instruction */
			  /* First operand is the timer length, second operand is
				the jump loc								   */

			  /* Extract the value from the left and the address
				from the right								   */
			  OpToken1    = Instruction->OpToken >> 4;
			  ValueL      = GetValue( PCB, OpToken1, Instruction->Operand1 );
			  OpToken2    = Instruction->OpToken & 0x0F;
			  Address     = GetAddr(  PCB, OpToken2, Instruction->Operand2,
								 TRUE );

			  /* Put the tick count in the PCB */
			  PCBList[PCB].TickCount = ValueL.Operand1;

			  /* OK, if the TickCount is 0 or negative, there is a
				critical error								*/
			  if ( ValueL.Operand1 < 1 ) {

				/* Set the critical error flag */
				CriticalError = ERROR;

				/* Message the error */
				BOSMsgI("Program :");
				BOSMsgC( PCBList[PCB].ProgramName );
				BOSMsgC(".  TMR instruction Critial Error!");
				BOSMsgIDone();

			  }; /* end if */

			  /* Put the jump location intot the PCB */
			  PCBList[PCB].TimerPointer = Address;

			  /* Indicate a timer is active */
			  ++TimersActive;

			  /* Next instruction */
			  PCBList[PCB].InstrPointer =
				CalcAddr(PCBList[PCB].InstrPointer, 1 );

			  break;


   /* -   -   -   -   -   -  HLT  -   -   -   -   -   -   -   -   -   -  */
	 case HLT : /* Halt program instruction */

			  /* Set the state of this PCB to WAIT */
			  PCBList[PCB].State = WAIT;
			  NumProgScheduled--;
			  NumProgWait++;

			  /* Check for endlesss loop, ie no timer or wait */
			  if ( (PCBList[PCB].WaitPointer == CoreSize )&&
				  (PCBList[PCB].TimerPointer == CoreSize )  ) {

				/* Yes! then it's a critical error! */
				/* Set the critical error flag */
				CriticalError = ERROR;

				/* Message the error */
				BOSMsgI("Program :");
				BOSMsgC( PCBList[PCB].ProgramName );
				BOSMsgC(".  Is in an endless loop. !Critial Error!");
				BOSMsgIDone();

				/* Compensate for critical error and reschedule this
				   program								  */
				NumProgScheduled++;
				NumProgWait--;

			  }; /* end if */

			  break;

   /* -   -   -   -   -   -  default  -   -   -   -   -   -   -   -   -   -  */
	 default : /* Default, must have been a data instruction */
			 /* Kill the program */

			 /* Set the critical error flag */
			 CriticalError = ERROR;

			 /* Message the error */
			 BOSMsgI("Program :");
			 BOSMsgC( PCBList[PCB].ProgramName );
			 BOSMsgC(".  Executed a DAT. !Critial Error!");
			 BOSMsgIDone();

   }; /* end Case */


   /* Increment total clocks of program */
   PCBList[PCB].TotalClocks++;

   /* Was there a critical error? If so, kill the program */
   if ( CriticalError != NOERROR ) KillProgram(PCB);

   /* Was a wait crossed? */
   if ( WaitPending != NOHANDLE ) {

	 /* YES! Ok, trip wait mechinism */
	 PCBList[WaitPending].InstrPointer = PCBList[WaitPending].WaitPointer;

	 /* Clear wait site */
	 PCBList[WaitPending].WaitPointer = CoreSize;

	 /* If the waiting program is in WAIT state, then READY it. */
	 if ( PCBList[WaitPending].State == WAIT ) {
	    PCBList[WaitPending].State = READY_RUN;
	    NumProgScheduled++;
	    NumProgWait--;

      }; /* end if */

	 /* Message a wait has been tripped */
	 BOSMsgI("Wait tripped for program ");
	 BOSMsgC( PCBList[WaitPending].ProgramName );
	 BOSMsgC(".");
	 BOSMsgIDone();

	 /* Clear this pending wait */
	 WaitPending = NOHANDLE;

	 /* Was a second wait crossed? */
	 if ( WaitPending2 != NOHANDLE ) {

	    /* YES! Ok, trip wait mechinism */
	    PCBList[WaitPending2].InstrPointer = PCBList[WaitPending2].WaitPointer;

	    /* Clear wait site */
	    PCBList[WaitPending2].WaitPointer = CoreSize;

	    /* If the waiting program is in WAIT state, then READY it. */
	    if ( PCBList[WaitPending2].State == WAIT ) {
	       PCBList[WaitPending2].State = READY_RUN;
	       NumProgScheduled++;
	       NumProgWait--;

         }; /* end if */

	    /* Message a wait has been tripped */
	    BOSMsgI("Wait tripped for program ");
	    BOSMsgC( PCBList[WaitPending2].ProgramName );
	    BOSMsgC(".");
	    BOSMsgIDone();

	    /* Clear this pending wait */
	    WaitPending2 = NOHANDLE;

	 }; /* end if, second wait */

   }; /* end if */


}; /* end ExecInstruction */


/* -------- Scheduler ----------------------------------------------------*/
/* This performs the program instruction execution				    */
void  Scheduler () {

   int  Step;


   /* Go through each PCB and execute an instruction for each one that
	 is ready.  Also check for timers  							*/
   for ( Step = MINPCB; Step < MAXPCB; Step++ ) {

	 /* Is this PCB ready? */
	 if ( PCBList[Step].State == READY_RUN ) 
	    ExecInstruction( Step );

	 /* Does this PCB have a timer set? */
	 if ( PCBList[Step].TimerPointer != CoreSize ) {

	    /* Yes, the decrement the counter */
	    PCBList[Step].TickCount--;

	    /* Is it now Zero?, if so then trip the timer */
	    if (PCBList[Step].TickCount == 0) {

		  /* Next instruction */
		  PCBList[Step].InstrPointer = PCBList[Step].TimerPointer;

		  /* Clear timer */
		  PCBList[Step].TimerPointer = CoreSize;
		  TimersActive--;

		  /* Insure the program is ready run, if it is waiting */
		  if (PCBList[Step].State == WAIT) {

			PCBList[Step].State = READY_RUN;
               NumProgScheduled++;
			NumProgWait--;

		  }; /* end if */

	    }; /* end if */

	 }; /* end if */

   }; /* end while */


   /* Increment total clocks run */
   TotalClocks++;

}; /* end Scheduler */


/* -------- Master -------------------------------------------------------*/
/* This fuction accepts key input, executes instructions of spawned
   programs, and dispatchs MMI commands on [ENTER]				    */
void  Master () {

   unsigned char	Continue;	/* Keep track of when to quit */


   /* -- Put the first prompt to the screen -- */
   BOSPrompt();

   /* -- Loop until quit BOS -- */
   do {

	    /* If there are programs ready to run OR timer(s) are active,
		  then call scheduler. */
	    if ( NumProgScheduled + TimersActive ) Scheduler();

	    /* See if keypress from the user, if so, process it */
	    if ( bioskey(1) ) Continue = DoKeyPress();

   } while ( Continue );

}; /* end Master */


/* -------- MAIN ---------------------------------------------------------*/

void main ( int argc, char *argv[] ) {

   int		  Step;


   /* Initialize any globals */
   for ( Step = BASEPCB; Step < MAXPCB; Step++ ) { /* Clear the PCB List */
	 PCBList[Step].State = UNUSED;
   }; /* end for */

   /* Seed random number generator */
   randomize();

   memset( MMICmnd, MAXCMDSIZE, sizeof( char ) );  /* Clear command line */
   MMICmndPtr = 0;							 /* Empty line         */


   /* Determine if core size might be specified in command line */
   if ( argc < 2 ) {
	 /* if only 1 entry on command line then core size was definitly not
	    specified.  Message user that the core size must be specified and
	    exit													*/
	 MsgNoCoreSpec();
	 exit(0);
   };

   /* Allocate core memory, success will hold memory needed for core */
   MemoryUsed = CreateCore( argv[1] );

   /* Clear the core */
   ClearCore();

   /* Message battleos init */
   MsgInitBOS(); 

   /* Call Master */
   Master();

}; /* end main */