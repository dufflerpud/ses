/************************************************************************
 *
 *indx#	ses.h - Constants used throughout ses and sesd
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
 *doc#	Constants used throughtout ses and sesd
 ************************************************************************/
#define TTY_PRINT	1
#define TTY_SETUP	2
#define	TTY_GET		3
#define TTY_RESET	4
#define TTY_RAW		5
#define TTY_BIN		6
#define TTY_ROWS	7
#define TTY_COLS	8
#define TTY_SIZE	9

#define C_QUIT		1
#define C_DAEMON	2
#define C_EXPECT	3
#define C_SEND		4
#define C_INTERACTIVE	5
#define C_LOCAL2REMOTE	6
#define C_REMOTE2LOCAL	7

#ifndef SERVICENAME
#define SERVICENAME	"ses"
#endif
#ifndef SERVICEPORT
#define SERVICEPORT	3050
#endif
#ifndef CONFIG
#define CONFIG		"/etc/ses.cfg"
#endif
#ifndef DEBUGFILE
#define DEBUGFILE	"/tmp/sesdebug"
#endif

#define SES_IN		"\nses -fi\n"
#define SES_OUT		"\nses -fo\n"
#define ESCCHAR		004

#define DEFTIME		10
#define DEFESCAPE	"\035"

#define MAXCAP		100

#define F		__FILE__,__LINE__
#define USER_ERROR	NULL,1

extern int		debugging;
extern char		*progname;
extern char		*caplist[MAXCAP];
extern char		*bestcap(char *s);
extern char		*printable( int c );

extern void fatal( char *fname, int lnum, char *fmt, ... );
extern void gothere( char *fname, int lnum, char *fmt, ... );

#ifdef linux
#include <errno.h>
#define STRERROR(c)	strerror(c)
#else
#ifndef __bsdi__
extern char *sys_errlist[];
#endif
extern int errno;
#define STRERROR(c)	sys_errlist[c]
#endif

/* Turns out this breaks things */
#ifdef UNDEF
#define void
#endif
