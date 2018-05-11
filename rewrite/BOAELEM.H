/*
	BattleOS : Engage battling programs.

	Copyright (C) 1993 Erich P Gatejen    ALL RIGHTS RESERVED


	File     : BOAELEM.H
	Purpose  : BOA elements of a program source line
*/

/* Preclude multiple declarations */
#ifndef __BOAELEM_H
#define	__BOAELEM_H

// Define typical elements
#define	COMMENT		'/'		// Comment line character
#define PROGNAME	'!'		// Program name character
#define	ASSIGN		'='		// Assignment statement
#define	REFERENCE	'['		// Open reference
#define	IMMEDIATEV	'$'		// Immediate value
#define	ADDRESSM	'-'		// A negative address offset
#define	ADDRESSP	'+'		// A positive address offset
#define LABEL		'@'		// A label
#define	TEXT		1		// Text has no specific identity
#define	TAUNTDEFINE	'"'		// A taunt define
#define	TAUNTIMBED	'<'		// A taunt imbed


#endif