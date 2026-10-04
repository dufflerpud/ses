/************************************************************************
 *
 *indx#	stty2c.c - Software for testing setting terminal characteristics
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
 *doc#	Software for testing setting terminal characteristics
 ************************************************************************/
#include <stdio.h>
#include <signal.h>
#ifndef _AUX_SOURCE
#include <sys/fcntl.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#if defined NeXT || defined linux
#include <sys/termios.h>
#define termio termios
#define TCGETA	TIOCGETA
#define TCSETA	TIOCSETA
#else
#include <sys/termio.h>
#endif
#include <sys/file.h>
#include <sys/ioctl.h>
#include <utmp.h>

#ifndef UTMP_FILE
#define UTMP_FILE	"/etc/utmp"
#endif

#ifndef WTMP_FILE
#define WTMP_FILE	"/var/adm/wtmp"
#endif

#include <sys/syslog.h>
#include <stdlib.h>

#ifdef STREAMSPTY
#include <utmpx.h>
#include <sac.h>
#include <sys/stropts.h>
#endif

void dobit(
	struct termio *bp,
	int field,
	int value,
	char *string )
    {
    int res = 0;
    int i;

    /* Screen out multiple bit or no bit constants */
    if( value == 0 ) return;
    for( i=1; (value&i)==0; i<<=1 )	;
    if( value != i )
	return;

    switch( field )
	{
	case 'i':	res=(bp->c_iflag&value);	break;
	case 'o':	res=(bp->c_oflag&value);	break;
	case 'c':	res=(bp->c_cflag&value);	break;
	case 'l':	res=(bp->c_lflag&value);	break;
	}
    printf("#ifdef %s\n",string);
    printf("    p->c_%cflag %s%s;\n",field,(res?"|= ":"&= ~"),string);
    printf("#endif\n");
    }

/**************************************************************************/
void pbuf( struct termio *tb )
/**************************************************************************/
    {
    printf(
	"iflag=0%0o oflag=0%0o cflag=0%0o lflag=0%0o line=0%0o\r\n",
	tb->c_iflag,tb->c_oflag,tb->c_cflag,tb->c_lflag
#ifndef NeXT
	,tb->c_line
#endif
    );
    }

/**************************************************************************/
int main( int argc, char *argv[] )
/**************************************************************************/
    {
    static struct termio tty_b;
    int field;

    if( ioctl( fileno(stdin), TCGETA, (char *)&tty_b) < 0 )
	perror("TCGETA");

    printf("setup_default( p )	struct termio *p;\n    {\n");
    printf("    memset( p, 0, sizeof( struct termio ) );\n");

    field='i';
    dobit( &tty_b, field, IGNBRK, "IGNBRK" );
    dobit( &tty_b, field, BRKINT, "BRKINT" );
    dobit( &tty_b, field, IGNPAR, "IGNPAR" );
    dobit( &tty_b, field, PARMRK, "PARMRK" );
    dobit( &tty_b, field, INPCK, "INPCK" );
    dobit( &tty_b, field, ISTRIP, "ISTRIP" );
    dobit( &tty_b, field, INLCR, "INLCR" );
    dobit( &tty_b, field, IGNCR, "IGNCR" );
    dobit( &tty_b, field, ICRNL, "ICRNL" );
    dobit( &tty_b, field, IUCLC, "IUCLC" );
    dobit( &tty_b, field, IXON, "IXON" );
    dobit( &tty_b, field, IXANY, "IXANY" );
    dobit( &tty_b, field, IXOFF, "IXOFF" );
    dobit( &tty_b, field, IMAXBEL, "IMAXBEL" );
#ifdef DOSMODE
    dobit( &tty_b, field, DOSMODE, "DOSMODE" );
#endif

    field='o';
    dobit( &tty_b, field, OPOST, "OPOST" );
    dobit( &tty_b, field, OLCUC, "OLCUC" );
    dobit( &tty_b, field, ONLCR, "ONLCR" );
    dobit( &tty_b, field, OCRNL, "OCRNL" );
    dobit( &tty_b, field, ONOCR, "ONOCR" );
    dobit( &tty_b, field, ONLRET, "ONLRET" );
    dobit( &tty_b, field, OFILL, "OFILL" );
    dobit( &tty_b, field, OFDEL, "OFDEL" );
    dobit( &tty_b, field, NLDLY, "NLDLY" );
    dobit( &tty_b, field, NL0, "NL0" );
    dobit( &tty_b, field, NL1, "NL1" );
    dobit( &tty_b, field, CRDLY, "CRDLY" );
    dobit( &tty_b, field, CR0, "CR0" );
    dobit( &tty_b, field, CR1, "CR1" );
    dobit( &tty_b, field, CR2, "CR2" );
    dobit( &tty_b, field, CR3, "CR3" );
    dobit( &tty_b, field, TABDLY, "TABDLY" );
    dobit( &tty_b, field, TAB0, "TAB0" );
    dobit( &tty_b, field, TAB1, "TAB1" );
    dobit( &tty_b, field, TAB2, "TAB2" );
    dobit( &tty_b, field, TAB3, "TAB3" );
    dobit( &tty_b, field, XTABS, "XTABS" );
    dobit( &tty_b, field, BSDLY, "BSDLY" );
    dobit( &tty_b, field, BS0, "BS0" );
    dobit( &tty_b, field, BS1, "BS1" );
    dobit( &tty_b, field, VTDLY, "VTDLY" );
    dobit( &tty_b, field, VT0, "VT0" );
    dobit( &tty_b, field, VT1, "VT1" );
    dobit( &tty_b, field, FFDLY, "FFDLY" );
    dobit( &tty_b, field, FF0, "FF0" );
    dobit( &tty_b, field, FF1, "FF1" );
#ifdef PAGEOUT
    dobit( &tty_b, field, PAGEOUT, "PAGEOUT" );
#endif
#ifdef WRAP
    dobit( &tty_b, field, WRAP, "WRAP" );
#endif

    field='c';
    dobit( &tty_b, field, CBAUD, "CBAUD" );
    dobit( &tty_b, field, CSIZE, "CSIZE" );
    dobit( &tty_b, field, CS5, "CS5" );
    dobit( &tty_b, field, CS6, "CS6" );
    dobit( &tty_b, field, CS7, "CS7" );
    dobit( &tty_b, field, CS8, "CS8" );
    dobit( &tty_b, field, CSTOPB, "CSTOPB" );
    dobit( &tty_b, field, CREAD, "CREAD" );
    dobit( &tty_b, field, PARENB, "PARENB" );
    dobit( &tty_b, field, PARODD, "PARODD" );
    dobit( &tty_b, field, HUPCL, "HUPCL" );
    dobit( &tty_b, field, CLOCAL, "CLOCAL" );
#ifdef RCV1EN
    dobit( &tty_b, field, RCV1EN, "RCV1EN" );
    dobit( &tty_b, field, XMT1EN, "XMT1EN" );
#endif
#ifdef LOBLK
    dobit( &tty_b, field, LOBLK, "LOBLK" );
#endif
#ifdef XCLUDE
    dobit( &tty_b, field, XCLUDE, "XCLUDE" );
#endif
    dobit( &tty_b, field, CRTSCTS, "CRTSCTS" );
    dobit( &tty_b, field, CIBAUD, "CIBAUD" );
#ifdef PAREXT
    dobit( &tty_b, field, PAREXT, "PAREXT" );
#endif

    field='l';
    dobit( &tty_b, field, ISIG, "ISIG" );
    dobit( &tty_b, field, ICANON, "ICANON" );
    dobit( &tty_b, field, XCASE, "XCASE" );
    dobit( &tty_b, field, ECHO, "ECHO" );
    dobit( &tty_b, field, ECHOE, "ECHOE" );
    dobit( &tty_b, field, ECHOK, "ECHOK" );
    dobit( &tty_b, field, ECHONL, "ECHONL" );
    dobit( &tty_b, field, NOFLSH, "NOFLSH" );
    dobit( &tty_b, field, TOSTOP, "TOSTOP" );
    dobit( &tty_b, field, ECHOCTL, "ECHOCTL" );
    dobit( &tty_b, field, ECHOPRT, "ECHOPRT" );
    dobit( &tty_b, field, ECHOKE, "ECHOKE" );
#ifdef DEFECHO
    dobit( &tty_b, field, DEFECHO, "DEFECHO" );
#endif
    dobit( &tty_b, field, FLUSHO, "FLUSHO" );
    dobit( &tty_b, field, PENDIN, "PENDIN" );
    dobit( &tty_b, field, IEXTEN, "IEXTEN" );

    for( field=0; field<6; field++ )
	printf("    p->c_cc[%d] = %d;\n",field,tty_b.c_cc[field]);

    printf("    }\n");
    exit(0);
    }
