/************************************************************************
 *
 *indx#	info.c - Software to show information about a pty or tty
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
 *doc#	Software to show information about a pty or tty
 ************************************************************************/
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(NeXT) || defined(linux)
#include <sys/termios.h>
#define termio termios
#else
#include <sys/termio.h>
#endif
#include <sys/file.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <stdlib.h>

/************************************************************************/
int main( int argc, char *argv[] )
/************************************************************************/
/*	Print out termio struct for standard input.			*/
/************************************************************************/
    {
    static struct termio sbuf;
    int ttypgrp;

    if( ioctl( 0, TCGETA, (char *)&sbuf) < 0 )
	perror( "TCGETA" );

    printf("sbuf.c_iflag=0%o;\n",sbuf.c_iflag);
    printf("sbuf.c_oflag=0%o;\n",sbuf.c_oflag);
    printf("sbuf.c_cflag=0%o;\n",sbuf.c_cflag);
    printf("sbuf.c_lflag=0%o;\n",sbuf.c_lflag);
    printf("sbuf.c_line=0%o;\n",sbuf.c_line&0xff);
#ifdef TIOCGETPGRP
    if( ioctl( 0, TIOCGETPGRP, &ttypgrp ) < 0 )
	perror( "TIOCGETPGRP" );
#else
    if( ioctl( 0, TIOCGPGRP, &ttypgrp ) < 0 )
	perror( "TIOCGPGRP" );
#endif
    printf("pgrp=%d ttypgrp=%d\n",getpgrp(),ttypgrp);
    exit(0);
    }
