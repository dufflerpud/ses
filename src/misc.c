/************************************************************************
 *
 *indx#	misc.c - Miscellaneous routines for ses and sesd
 *@HDR@	$Id$
 *@HDR@
 *@HDR@	Copyright (c) 1995-2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
 *@HDR@
 *@HDR@	Permission is hereby granted, free of charge, to any person
 *@HDR@	obtaining a copy of this software and associated documentation
 *@HDR@	files (the "Software"), to deal in the Software without
 *@HDR@	restriction, including without limitation the rights to use,
 *@HDR@	copy, modify, merge, publish, distribute, sublicense, and/or
 *@HDR@	sell copies of the Software, and to permit persons to whom
 *@HDR@	the Software is furnished to do so, subject to the following
 *@HDR@	conditions:
 *@HDR@	
 *@HDR@	The above copyright notice and this permission notice shall be
 *@HDR@	included in all copies or substantial portions of the Software.
 *@HDR@	
 *@HDR@	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 *@HDR@	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *@HDR@	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 *@HDR@	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *@HDR@	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *@HDR@	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *@HDR@	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 *@HDR@	OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *hist#	2026-10-04 - Christopher.M.Caldwell0@gmail.com - Created
 ************************************************************************
 *doc#	Miscellaneous routines for ses and sesd
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ses.h"

#ifdef NEED_LIBC_DEFS
extern int strlen( const char * );
extern int strncmp( const char *, const char *, int );
#endif

static char *begargv = NULL;
static char *endargv = NULL;
/************************************************************************/
void setup_psmsg( char **argv )
/************************************************************************/
    {
    extern char ** environ;
    char **traversep = ( environ ? environ : argv );

    begargv = argv[0];

    while( *traversep )
	{
	endargv = traversep[0] + strlen( traversep[0] );
	traversep++;
	}
    }

/************************************************************************/
void set_psmsg( char *msg )
/************************************************************************/
    {
    char *cp;
    for( cp=begargv; cp!=endargv; cp++ )
	if( *msg )
	    *cp = *msg++;
	else
	    *cp = ' ';
    }

/************************************************************************/
int abbrev( char *cmd, char *cmdlist[] )
/************************************************************************/
/*	Search for the string cmd in the array of strings cmdlist	*/
/*	looking for the closest match.					*/
/************************************************************************/
    {
    int foundone = -1;
    int i;
    int lcmd = strlen(cmd);
    char *cp1, *cp2;
    for( i=0; cp1=cmdlist[i]; i++ )
	while( *cp1 )
	    {
	    for( cp2=cmd; *cp2 && *cp1==*cp2; cp1++,cp2++ )	;
	    if( *cp2 == 0 )
		if( *cp1=='|' || *cp1==0 )
		    return i;
		else if( foundone == -1 )
		    foundone = i;
		else
		    foundone = -2;
	    else
		while( *cp1 && *cp1!='|' ) cp1++;
	    if( *cp1 == '|' ) cp1++;
	    }
    return foundone;
    }

/************************************************************************/
char *bestcap(char *s)
/************************************************************************/
/*	Search through the string array caplist for the string matching	*/
/*	the argument s.  This looks for the last entry (presumably the	*/
/*	best entry) that matches.					*/
/************************************************************************/
    {
    int l = strlen(s);
    char **capl;
    char *found = NULL;
    for( capl=caplist; *capl; capl++ )
	if( strncmp(s,*capl,l) == 0 )
	    found = (*capl) + l;
    return found;
    }

/**************************************************************************/
char *printable(int c)
/************************************************************************/
/*	Return a string with a printable version of the specified	*/
/*	character.  That is, non-printable characters are represented	*/
/*	by \nnn or \c notation.						*/
/************************************************************************/
    {
    static char retbuf[10], *str;
    switch( c )
	{
	case '\r':	str="\\r";		break;
	case '\n':	str="\\n";		break;
	case '\t':	str="\\t";		break;
	default:
	    if( c > ' ' && c <= '~' )
		sprintf( str=retbuf, "%c", c );
	    else
		sprintf( str=retbuf, "\\%03o", c&0xff );
	    break;
	}
    return str;
    }
