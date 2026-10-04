/************************************************************************
 *
 *indx#	pty.c - Stub routines for doing pty i/o (not used)
 *@HDR@	$Id$
 *@HDR@
 *@HDR@	Copyright (c) 2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
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
 *doc#	Stub routines for doing pty i/o (not used)
 ************************************************************************/
#include <stdio.h>
#include "ses.h"
#include <string.h>

int	debugmode = 0;
int	watchmode = 0;
char	*sttysettings = NULL;

#ifdef NEED_LIBC_DEFS
extern int strncmp( const char *, const char *, int );
extern int strlen( const char * );
#endif

/**************************************************************************/
int tty_settings( int func, int nl, int nc )
/**************************************************************************/
    {
    switch( func )
	{
	case TTY_PRINT:	return 0;
	case TTY_SETUP:	return 0;
	case TTY_GET:	return 0;
	case TTY_RESET:	return 0;
	case TTY_RAW:	return 0;
	case TTY_BIN:	return 0;
	case TTY_ROWS:	return 0;
	case TTY_COLS:	return 0;
	case TTY_SIZE:	return 0;
	}
    }

static char line[100];

/**************************************************************************/
char *getptyname()
/**************************************************************************/
    {
    return line;
    }

/**************************************************************************/
void cleanup_utmp()
/**************************************************************************/
    {
    }

/**************************************************************************/
int find_pty( char *ptyname )
/**************************************************************************/
    {
    int master = 0;
    return master;
    }

static char *keepenv[] =
    {
    "TERM=",
    NULL
    };

/**************************************************************************/
void small_env()
/**************************************************************************/
    {
    extern char **environ;
    int i, src;
    int dst = 0;
    for( src=0; environ[src]; src++ )
	{
	for( i=0; keepenv[i]; i++ )
	    if( strncmp( environ[src], keepenv[i], strlen(keepenv[i]) ) == 0 )
		break;
	if( keepenv[i] )
	    environ[dst++] = environ[src];
	}
    environ[dst] = NULL;
    }

/**************************************************************************/
int setup_pty(char *e0,char *e1,char *e2,char *e3,char *e4,char *e5)
/**************************************************************************/
    {
    int master = find_pty( line );
    return master;
    }
