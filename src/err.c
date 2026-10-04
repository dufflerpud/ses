/************************************************************************
 *
 *indx#	err.c - Error handling for ses/sesd
 *@HDR@	$Id$
 *@HDR@
 *@HDR@	Copyright (c) 1994-2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
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
 *doc#	Error handling for ses/sesd
 ************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "ses.h"

#ifdef NEED_LIBC_DEFS
extern int getpid();
char *strcpy( char *s, const char *f );
extern int strlen(const char *s);
extern void abort();
extern void exit( int );
#endif

/************************************************************************/
void fatal( char *fnam, int fnum, char *fmt, ... )
/************************************************************************/
/*	Print error message and exit.					*/
/************************************************************************/
    {
    va_list ap;
    va_start( ap, fmt );
    char buf[1024], *cp;

    if( fnam )
        fprintf(stderr,"%s(%s/%d):  ",progname,fnam,fnum);
    else
        fprintf(stderr,"%s:  ",progname);
    for( cp=buf; *fmt; fmt++ )
	if( fmt[0] != '%' || fmt[1] != 'e' )
	    *cp++ = *fmt;
	else
	    {
	    strcpy( cp, STRERROR(errno) );
	    cp += strlen( STRERROR(errno) );
	    fmt++;
	    }
    *cp = 0;
    vfprintf(stderr,buf,ap);
    va_end( ap );
    fflush(stderr);
    if( fnam )
	abort();
    else
	exit(fnum);
    }

/************************************************************************/
void gothere(char *fnam, int fnum, char *fmt, ... )
/************************************************************************/
/*	Used for debugging.  Called like printf except first two	*/
/*	arguments are filename (string) and line number (int).		*/
/*	Information written to DEBUGFILE.				*/
/************************************************************************/
    {
    va_list ap;
    va_start(ap,fmt);
    char buf[1024], *cp;
    FILE *ochan;

    sprintf( buf, "%s.%d", DEBUGFILE, getpid() );
    if( (ochan = fopen( buf, "a" )) == NULL )
	fatal(F,"fopen(%s,%s) failed:  %e\n",buf,"a");

    fprintf(ochan,"%s(%s/%d):  ",progname,fnam,fnum);
    for( cp=buf; *fmt; fmt++ )
	if( fmt[0] != '%' || fmt[1] != 'e' )
	    *cp++ = *fmt;
	else
	    {
	    strcpy( cp, STRERROR(errno) );
	    cp += strlen( STRERROR(errno) );
	    fmt++;
	    }
    *cp = 0;
    vfprintf(ochan,buf,ap);
    fclose(ochan);
    va_end( ap );
    }
