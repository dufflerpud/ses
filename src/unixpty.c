/************************************************************************
 *
 *indx#	unixpty.c - PTY handler for ses and sesd
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
 *doc#	PTY handler for ses and sesd
 ************************************************************************/

#define _XOPEN_SOURCE
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#ifndef _AUX_SOURCE
#ifndef _AIX
#ifndef ultrix
#include <sys/fcntl.h>
#endif
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>
#if defined( NeXT ) || defined( __bsdi__ ) || defined( linux )
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

#ifdef hpux
#include <syslog.h>
#else
#include <sys/syslog.h>
#endif

#ifdef STREAMSPTY
#ifndef __DGUX__
#ifndef __osf__
#include <utmpx.h>
#endif
#endif
#ifndef __osf__
#ifndef linux
#include <sac.h>
#endif
#endif
#ifndef linux
#include <sys/stropts.h>
#endif
#endif

#include "ses.h"

int	debugmode = 0;
int	watchmode = 0;
char	*sttysettings = NULL;

#include <sys/wait.h>

extern int grantpt( int fd );
extern int unlockpt( int fd );
extern char *ptsname(int fd);
extern int ptsname_r(int fd, char buf[], size_t size);
extern int gethostname(char *name, size_t size);

const char *UTMPS[] = { "/etc/utmp", "/var/run/utmp", "/run/utmp", NULL };
const char *WTMPS[] = { "/var/adm/wtmp", "/var/log/wtmp", NULL };

/**************************************************************************/
int tty_settings( int func, int nl, int nc )
/**************************************************************************/
    {
    static struct termio tty_b;
#ifdef TIOCGWINSZ
    static struct winsize tty_win;
#else
    char *env;
#endif
    struct termio sbuf;

#ifdef DEBUG
    printf("sttysettings=%d tty_settings(%d,%d,%d).\r\n",sttysettings,func,nl,nc);	fflush(stdout);
#endif
    if( sttysettings ) return 0;

    switch( func )
	{
	case TTY_PRINT:
	    printf(
		"iflag=0%0o oflag=0%0o cflag=0%0o lflag=0%0o line=0%0o\r\n",
		tty_b.c_iflag,tty_b.c_oflag,tty_b.c_cflag,tty_b.c_lflag
#if !defined( NeXT ) && !defined( __bsdi__ )
		,tty_b.c_line
#endif
		);
#ifdef TIOCGWINSZ
	    printf("rows=%d cols=%d\r\n",tty_win.ws_row,tty_win.ws_col);
#endif
	    return 0;
	case TTY_SETUP:
	    memset( &tty_b, 0, sizeof( struct termio ) );
#ifdef TIOCGWINSZ
	    memset( &tty_win, 0, sizeof(tty_win) );
	    tty_win.ws_row = 24;
	    tty_win.ws_col = 80;
#endif
	    return 0;
	case TTY_GET:
	    if( ioctl( fileno(stdin), TCGETA, (char *)&tty_b) < 0 )
		fatal(F,"RESET TCGETA failed:  %e\n");
#ifdef TIOCGWINSZ
	    if( ioctl( fileno(stdin), TIOCGWINSZ, (char *)&tty_win) < 0 )
		{
		/* fatal(F,"GET TIOCSWINSZ failed:  %e\n"); */
		tty_win.ws_row = 24;	/* Solaris defines it but returns */
		tty_win.ws_col = 80;	/* EINVAL */
		}
#endif
	    return 0;
	case TTY_RESET:
	    if( tty_b.c_iflag == 0 )
	        {
		system("stty sane 38400");
		}
	    else if( ioctl( fileno(stdout), TCSETA, (char *)&tty_b) < 0 )
		fatal(F,"RESET TCSETA failed:  %e\n");
#ifdef TIOCGWINSZ
	    if( ioctl( fileno(stdout), TIOCSWINSZ, (char *)&tty_win) < 0 )
		{
		/* fatal(F,"RESET TIOCSWINSZ failed:  %e\n"); */
		/* Can't do anything under Solaris */
		;
		}
#endif
	    return 0;
	case TTY_RAW:
	    sbuf = tty_b;
	    sbuf.c_lflag  =  0;
	    sbuf.c_iflag  =  ( BRKINT );
	    sbuf.c_oflag &= ~( OPOST );
#ifdef CBAUD
	    sbuf.c_cflag &=  ( CBAUD );
#else
	    sbuf.c_cflag &=  ( 017 );
#endif
	    sbuf.c_cflag |=  ( CS8 | CREAD );
	    sbuf.c_cc[VMIN] = 1;
	    sbuf.c_cc[VTIME] = 0;
#ifdef linux
	    system("stty raw -echo");
#else
	    if( ioctl( fileno(stdout), TCSETA, (char *)&sbuf) < 0 )
		fatal(F,"RAW TCSETA failed:  %e\n");
#endif
	    return 0;
	case TTY_BIN:
	    sbuf = tty_b;
	    sbuf.c_lflag &= ~( ECHO );
	    sbuf.c_oflag &= ~( ONLCR );
	    if( ioctl( fileno(stdout), TCSETA, (char *)&sbuf) < 0 )
		fatal(F,"BIN TCSETA failed:  %e\n");
	    return 0;
#ifdef TIOCGWINSZ
	case TTY_ROWS:
	    return tty_win.ws_row;
	case TTY_COLS:
	    return tty_win.ws_col;
	case TTY_SIZE:
	    tty_win.ws_row = nl;
	    tty_win.ws_col = nc;
	    return 1;
#else
	case TTY_ROWS:
	    if( env = getenv("LINES") )
		return atoi(env);
	    else
		return 24;
	case TTY_COLS:
	    if( env = getenv("LINES") )
		return atoi(env);
	    else
		return 24;
	case TTY_SIZE:
	    ;
#endif
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
    struct utmp ut;
    int utfile;
    char *ptyname;
    const char *utmpname = NULL;
    int found = 0;

    struct stat stb;
    for( const char **utmp_ptr=UTMPS; utmpname=*utmp_ptr++; )
        if( stat( utmpname, &stb ) == 0 )
	    break;

    if( ! utmpname )
	{
	fprintf(stderr,"No utmp file, skipping name removal.\n");
	return;
	}

    if( (ptyname = strrchr( line, '/' )) == NULL )
	ptyname = line;
    else
	ptyname++;
    if( (utfile = open( utmpname, 2 )) < 0 )
	fatal(F,"Cannot open %s:  %e\n",utmpname);
    while( !found && read(utfile,&ut,sizeof(ut)) == sizeof(ut) )
	if( strncmp( ut.ut_line, ptyname, sizeof(ut.ut_line) ) == 0 )
	    {
	    memset( ut.ut_name, 0, sizeof(ut.ut_name) );
#ifdef ultrix
	    memset( ut.ut_host, 0, sizeof(ut.ut_host) );
#else
#ifndef EMPTY
	    memset( ut.ut_host, 0, sizeof(ut.ut_host) );
#else
	    memset( &ut.ut_type, 0, sizeof(ut.ut_type) );
#endif
#endif
	    time( (time_t*)&ut.ut_time );
	    if( lseek( utfile, -(size_t)sizeof(ut), 1 ) < 0 )
		fatal(F,"lseek of %s failed:  %e\n",utmpname);
	    if( write( utfile, &ut, sizeof(ut) ) < sizeof(ut) )
		fatal(F,"write of %s failed:  %e\n",utmpname);
	    found = 1;
	    }
    close( utfile );
    if( found )
	{
	const char *wtmpname = NULL;
	for( const char **wtmp_ptr=WTMPS; wtmpname=*wtmp_ptr++; )
	    if( stat( wtmpname, &stb ) == 0 )
		break;
	if( ! wtmpname )
	    { fprintf(stderr,"Cannot find wtmp file, skipping.\n"); }
	else
	    {
	    if( (utfile = open( wtmpname, 2 )) < 0 )
		fatal(F,"Cannot open %s:  %e\n",wtmpname);
	    if( lseek( utfile, (size_t)0, 2 ) < 0 )
		fatal(F,"lseek of %s failed:  %e\n",wtmpname);
	    if( write( utfile, &ut, sizeof(ut) ) < sizeof(ut) )
		fatal(F,"write of %s failed:  %e\n",wtmpname);
	    close( utfile );
	    }
	}
    }

/**************************************************************************/
int find_pty( char *ptyname )
/**************************************************************************/
    {
    int master;

#ifdef STREAMSPTY
    if( (master=open("/dev/ptmx",O_RDWR)) < 0 )
	fatal(F,"Out of pty's:  %e\n");
    grantpt( master );
    unlockpt( master );
    strcpy( ptyname, ptsname(master) );
    return master;

#else

    char bank, *pty;
    struct stat stb;
    int myuid = getuid();
    int ptyuid = myuid;

    for (bank='p'; bank<='z'; bank++ )
	for (pty = "0123456789abcdef"; *pty; pty++)
	    {
	    sprintf( ptyname, "/dev/pty%c%c", bank, *pty );
	    if (stat(ptyname, &stb) < 0)
		fatal(F,"Out of pty's:  %e\n");

	    if( (master = open(ptyname, O_RDWR)) < 0 ) continue;

	    ptyname[strlen("/dev/")] = 't';

	    chown( ptyname, ptyuid, 0 );
	    chmod( ptyname, 0600 );
	    return master;
	    }
    fatal(F,"Out of pty's\n");
#endif
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
    int master, slave, chan;
    int kidpid;

    master = find_pty( line );

    if( (kidpid=fork()) < 0 )
	fatal(F,"Fork failed:  %e\n");
    if( kidpid > 0 )
	return master;
    for( chan=0; chan<20; chan++ )
	(void)close( chan );

#ifdef TIOCNOTTY
    if( (chan = open("/dev/tty", O_RDWR)) >= 0 )
	{
	if( ioctl(chan, TIOCNOTTY, (char *)0) < 0 )
	    ;
	(void) close(chan);
	}
#endif

    if( (slave = open( line, O_RDWR )) < 0 )
	fatal(F,"open(%s) failed:  %e\n",line);

#if defined( sun ) || defined( i860 ) || defined( __bsdi__ )
    setsid();
#else
#ifdef TIOCSPGRP
    {
#ifdef linux
    int newpgrp = getpgrp();
#else
    int newpgrp = getpgrp(0);
#endif
    if( ioctl( slave, TIOCSPGRP, &newpgrp ) < 0 )
#if defined( linux ) || defined( i860 )
	gothere( F, "ioctl(%d,TIOCSPGRP,%d) failed with %e\n",slave,newpgrp);
#else
	fatal(F,"TIOCSPGRP failed:  %e\n");
#endif
    }
#endif
#endif

#ifdef STREAMSPTY
#ifndef linux
    if (ioctl(slave, I_PUSH, "ptem") < 0)
	fatal(F,"I_PUSH ptem failed:  %e\n");
    if (ioctl(slave, I_PUSH, "ldterm") < 0)
	fatal(F,"I_PUSH ldterm failed:  %e\n");
#endif
#endif

#ifdef TIOCSCTTY
    if( ioctl( slave, TIOCSCTTY, NULL ) < 0 )
#if defined( linux ) || defined( i860 ) || defined( __bsdi__ )
	gothere(F,"ioctl(%d,TIOCSCTTY) failed with:  %e\n",slave);
#else
	fatal(F,"TIOCSCTTY failed:  %e\n");
#endif
#endif /* TIOCSCTTY */

    if( slave!=0 && (chan=dup2(slave, 0)) < 0 )
	fatal(F,"dup2(%d,0) failed:  %e\n",slave);
    if( slave!=1 && (chan=dup2(slave, 1)) < 0 )
	fatal(F,"dup2(%d,1) failed:  %e\n",slave);
    if( slave!=2 && (chan=dup2(slave, 2)) < 0 )
	fatal(F,"dup2(%d,2) failed:  %e\n",slave);
    if( slave >= 3 ) (void) close(slave);

    if( !sttysettings )
	tty_settings(TTY_RESET,0,0);
    else
	if( (kidpid=fork()) < 0 )
	    fatal(F,"fork for stty failed:  %e\n");
	else if( kidpid > 0 )
#ifdef __NeXT__
	    wait4( kidpid, NULL, NULL, NULL );
#else
#ifdef titan
	    wait( NULL );
#else
	    waitpid( kidpid, NULL, 0 );
#endif
#endif
	else
	    {
	    execlp("stty","stty",sttysettings,NULL);
	    perror("stty");
	    exit(1);
	    }
    if( strcmp( e1, "login" ) == 0 )
	{
	char hostname[100];
	gethostname( hostname, 99 );
	printf( "\r\n\n[ %s session on %s ]\r\n\n", hostname, line );
	fflush(stdout);

#ifndef linux
#ifndef titan
#ifndef _AUX_SOURCE
#ifndef __DGUX__
#ifndef hpux
#ifndef _AIX
#ifndef __osf__
#ifdef LOGIN_PROCESS
	    {
	    /* System V login expects a utmp entry to already be there */
	    struct utmpx ut;

	    memset ((char *) &ut, 0, sizeof (ut));
	    (void) strncpy(ut.ut_user, ".telnet", sizeof(ut.ut_user));
	    (void) strncpy(ut.ut_line, line, sizeof(ut.ut_line));
	    ut.ut_pid = (o_pid_t)getpid();
	    ut.ut_id[0] = 't';
	    ut.ut_id[1] = 'n';
	    ut.ut_id[2] = SC_WILDC;
	    ut.ut_id[3] = SC_WILDC;
	    ut.ut_type = LOGIN_PROCESS;
	    ut.ut_exit.e_termination = 0;
	    ut.ut_exit.e_exit = 0;
	    (void) time (&ut.ut_tv.tv_sec);
	    if (makeutx(&ut) == NULL)
		syslog(LOG_INFO, "in.sesd:\tmakeutx failed %m");
	    }
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
	}
    /* e0="sh"; e1="sh"; e2=NULL; */
#ifdef linux
    small_env();
#endif

    execlp( e0, e1, e2, e3, e4, e5 );
    system( e0 );
    fatal(F,"Cannot run %s:  %e\n",e0);
    }

#ifdef MAIN
/**************************************************************************/
main(argc, argv)
int argc;
char *argv[];
/**************************************************************************/
    {
    int i;
    int ptychan;
    char *shell;

    if( (shell = getenv("SHELL")) == NULL ) shell = "/bin/sh";

    for( i=1; i<argc; i++ )
	if( strcmp(argv[i],"-d") == 0 )
	    debugmode++;
	else if( strcmp(argv[i],"-w") == 0 )
	    watchmode++;
	else if( strchr(argv[i],':') )
	    sttysettings = argv[i];
    
    tty_settings(TTY_GET,0,0);

    ptychan = setup_pty( shell, shell, 0, 0, 0, 0 );
    tty_settings(TTY_RAW,0,0);
    xfer( 2, fileno(stdin), ptychan, -1, ptychan, fileno(stdout), -1 );
    close( ptychan );
    tty_settings(TTY_RESET,0,0);
    exit(0);
    }
#endif
