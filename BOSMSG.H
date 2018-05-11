/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED

	Version  : 1.0

	File     : BOSMSG.H
	Purpose  : Header file for Battle Operating System console messaging

*/


/* Functions available for console messaging */
void  TimeStamp( void );
void  BOSMsg ( char  *Message );
void  BOSMsgI ( char *Message );
void  BOSMsgIDone ( void );
void  BOSMsgTaunt ( char  *ProgName, char  *Message );
void  BOSMsgU ( char  *String, unsigned int  Number );
void  BOSMsgS ( char  *String, int  Number );
void  BOSMsgL ( char  *String, long  Number );
void  BOSMsgST ( char *String, char  *String2 );
void  BOSMsgC ( char  *String );
void  BOSPrompt ( void );
void  MsgInitBOS ( void );
void  MsgBadCoreSpec ( void );
void  MsgNoCoreSpec ( void );
void  MsgNoCore ( void );
void  MsgBadCommand ( void );
void  DisplayState( int  State );
void  DisplayOpToken( unsigned char  Byte );
char  Continue( void );
void  BOSMsgClean( void );
