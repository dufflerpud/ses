/************************************************************************
 *
 *indx#	optional.h - Glue software for making ses work on lots of machines
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
 *doc#	Glue software for making ses work on lots of machines
 ************************************************************************/

#ifdef NeXT
#define NEED_STRDUP
#define NEED_PUTENV
#endif

#ifdef _AUX_SOURCE
#define NEED_STRDUP
#endif

#ifdef ultrix
#define NEED_STRDUP
#endif

/************************************************************************/
/*	I can't find these routines on a NeXT box.  They perform the	*/
/*	standard UNIX functionality.					*/
/************************************************************************/

#ifdef NEED_STRDUP

extern char *malloc();

/************************************************************************/
char *strdup( char *s )
/************************************************************************/
/*	Allocate enough memory for a string and copy it in place.	*/
/************************************************************************/
    {
    char *res = malloc( strlen( s ) + 1 );
    strcpy( res, s );
    return res;
    }

#endif

#ifdef NEED_PUTENV
/************************************************************************/
void putenv( char *s )
/************************************************************************/
/*	Put string in string array called environment.			*/
/************************************************************************/
    {
    extern char **environ, *realloc();
    static char **e; 
    int c, ec;

    for( c=0; s[c]; c++ )
	if( s[c] == '=' )
	    break;

    for( ec=0; environ[ec]; ec++ )
	if( strncmp( environ[ec], s, c ) == 0 )
	    {
	    environ[ec] = s;
	    return;
	    }

    if( e == NULL )
	{
	e = (char **)malloc( sizeof(char*) * (ec + 2) );
	memcpy( e, environ, sizeof(char*) * (ec + 2) );
	environ=e;
	}
    else
	environ = (char**)realloc( environ, sizeof(char*) * (ec + 2) );
    environ[ec] = s;
    environ[ec+1] = NULL;
    }
#endif
