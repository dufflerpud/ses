/************************************************************************
 *
 *indx#	uud.c - VERY simple version of uudecode
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
 *doc#	VERY simple version of uudecode
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SP	' '

/************************************************************************/
int main( int argc, char *argv[] )
/************************************************************************/
/*	This routine does what uudecode SHOULD do.  It acts as a filter	*/
/*	taking output from uuencode and converting it back to binary.	*/
/************************************************************************/
    {
    char b[100];

    while( fgets(b,99,stdin) && strncmp(b,"begin",5) )
	;
    if( strncmp(b,"begin",5) != 0 )
	exit( 0 );
    while( fgets(b,99,stdin) )
        {
	int n = b[0] - SP;
	int i, j;
	int c0, c1, c2, c3;

	if( n == 0 ) break;

	for( j=i=0; i<n; i++ )
	    switch( i%3 )
		{
		case 0:	c0=b[++j]-SP; c1=b[++j]-SP; c2=b[++j]-SP; c3=b[++j]-SP;
			putchar( (c0<<2)	| (c1>>4)	);	break;
		case 1:	putchar( (c1&017)<<4	| (c2>>2)	);	break;
		case 2:	putchar( (c2&03)<<6	| c3		);	break;
		}
        }
    exit( 0 );
    }
