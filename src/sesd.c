/************************************************************************
 *
 *indx#	sesd.c - Server for doing encrypted remote session
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
 *doc#	Server for doing encrypted remote session
 ************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <sys/un.h>
#if defined( NeXT ) || defined( __bsdi__ ) || defined( linux )
#include <sys/termios.h>
#define termio termios
#else
#include <sys/termio.h>
#endif
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <stdarg.h>
#include "ses.h"

#include "optional.h"

struct sockaddr_in frominet;

#ifdef _AIX
extern char *malloc(int), *getenv(char*), *getptyname(), *inet_ntoa();
extern char *strchr(char*,int);
#else
/* extern char *malloc(), *getenv(), *getptyname(), *inet_ntoa(), *strchr(); */
#include <unistd.h>
#include <stdlib.h>
#endif

extern void read_dec( int, void *, int );
extern void write_enc( int, void *, int );
extern int tty_settings( int, int, int );
extern void setup_psmsg( char **argv );
extern void set_psmsg( char *msg );
extern char *getptyname();
extern void setpass( const char *, int );
extern int setup_pty(char*,char*,char*,char*,char*,char*);
extern int xfer( int towatch, ... );
extern void cleanup_utmp();

char *progname = NULL;
char *caplist[MAXCAP];
int debugging = 0;

/**************************************************************************/
void usage( char *fmt, ... )
/**************************************************************************/
    {
    va_list ap;
    va_start( ap, fmt );
    fprintf(stderr,fmt,ap);
    fprintf(stderr,"\nUsage:  %s <argument>\n\n",progname);
    fprintf(stderr,"Where <argument> is one or more of:\n");
    fprintf(stderr,"    -D                 Turn on debugging\n");
    exit(1);
    }

/**************************************************************************/
void negotiate( int chan )
/**************************************************************************/
    {
    static char buf[100];
    char nbuf[10];
    char *newcap;
    int size;
    int newlen;
    char *clist[MAXCAP];
    char *capenv = getenv("SESCAP");
    int i, j, k;
    char *cp;

    read_dec( chan, nbuf, 5 );
    size = atoi( nbuf );
    if( size < 5 || size > 10000 )
	{
	fprintf(stderr,"You are not the correct version of ses.  Go away.\n");
	exit(1);
	}
    read_dec( chan, buf, size );
    newcap = malloc( strlen(capenv) + 1 );
    newlen = 0;
    for(clist[i=0]=strtok(capenv,",");
	clist[i];
	clist[++i]=strtok(NULL,",") )
	;
    for( k=0,cp=strtok(buf,","); cp; cp=strtok(NULL,",") )
	{
	if( debugging ) fprintf(stderr,"[ses client can %s]\n",cp);
	for( j=0; clist[j]; j++ )
	    if( strcmp( cp, clist[j] ) == 0 )
		{
		caplist[k++] = cp;
		sprintf( newcap+newlen, "%s,", cp );
		newlen = newlen + strlen(cp) + 1;
		break;
		}
	}
    caplist[k] = NULL;
    if( newlen > 0 )
	newcap[newlen-1] = 0;
    sprintf( nbuf, "%d", newlen );
    write_enc( chan, nbuf, 5 );
    write_enc( chan, newcap, newlen );
    }

/**************************************************************************/
void readenv( int chan )
/**************************************************************************/
    {
    char numbuf[5], *envbuf, *var, *endbuf;
    int size;
    int nlines= -1;
    int ncols= -1;

    read_dec( chan, numbuf, 5 );
    size = atoi( numbuf );
    read_dec( chan, envbuf=malloc(size), size );
    endbuf = envbuf + size;

    for( var=envbuf; var<endbuf; var=strchr(var,0)+1 )
	{
	char *val = strchr( var, '=' );
	putenv( var );
	if( debugging ) fprintf(stderr,"putenv(%s)\n", var );
	*val++ = 0;
	if( strcmp(var,"LINES") == 0 )
	    nlines = atoi(val);
	else if( strcmp(var,"COLUMNS") == 0 )
	    ncols = atoi(val);
	*--val = '=';
	}

    if( nlines >= 0 )
	tty_settings( TTY_SIZE, nlines, ncols );
    }

/**************************************************************************/
int setup_dsocket()
/**************************************************************************/
    {
    struct servent *sp;
    int finet;
    int options = 0;
    struct sockaddr_in sin;
    u_short hport;
    int one = 1;
    int fromlen = sizeof(frominet);
    int boundchan;

    if( (sp = getservbyname(SERVICENAME,"tcp")) != NULL )
	hport = ntohs(sp->s_port);
    else
	hport = SERVICEPORT;

    if( (finet = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	fatal(F,"socket(AF_INET,SOCK_STREAM,0) failed:  %e\n");

#ifdef notdef
    if (options & SO_DEBUG)
#ifndef NOT43
	if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, &one, sizeof(one)) < 0)
#else NOT43
	if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, 0, 0 ) < 0)
#endif
	    fatal(F,"setsockopt (SO_DEBUG): %e\n");
#endif
    
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(hport);
    if( bind(finet, (struct sockaddr *)&sin, sizeof(sin) ) < 0 )
	fatal(F,"bind(%d) failed:  %e\n",finet);

    if( listen(finet, 5) < 0 )
	fatal(F,"listen(%d) failed:  %e\n",finet);

    if( (boundchan = accept( finet, (struct sockaddr *)&frominet, &fromlen )) < 0 )
	fatal(F,"accept(%d) failed:  %e\n",boundchan);

    return boundchan;
    }

/**************************************************************************/
int main( int argc, char **argv)
/**************************************************************************/
    {
    int dsocket, dpty;
    char buf[100];
    FILE *infile;
    char *pass = NULL;
    int i;
    int inetd_mode = ! isatty( fileno(stdin) );
    char *envarg = "-p";
    char *extaddr;

    setup_psmsg( argv );
    putenv(strdup("SESCAP=enc=none,enc=primitive,enc=enigma"));
    /* putenv(strdup("SESCAP=enc=none")); */

    if( (progname=strrchr(argv[0],'/')) == NULL )
	progname = argv[0];
    else
	progname++;

    for( i=1; i<argc; i++ )
	if( strcmp(argv[i],"-D") == 0 )
	    {
	    debugging = 1;
	    close( 1 );
	    close( 2 );
	    dup( creat( DEBUGFILE, 0666 ) );
	    }
	else
	    usage("Unknown argument:  %s\n",argv[i]);

    if( (infile=fopen(CONFIG,"r")) == NULL )
	fatal(F,"Cannot open %s:  %e\n",CONFIG);
    while( fgets( buf, 100, infile ) )
	{
	char *cp;
	if( cp = strchr( buf, '\n' ) ) *cp = 0;
	if( cp = strchr( buf, '#' ) ) *cp = 0;
	if( cp = strchr( buf, '=' ) )
	    {
	    int i;
	    *cp++ = 0;
	    if( strcmp( buf, "password" ) == 0 )
	    if( (i = strlen(cp)) > 8 )
		i = 8;
	    pass = malloc( i+1 );
	    for( i=0; cp[i]; i++ )
		if( i < 8 )
		    pass[i] = cp[i];
		else
		    {
		    pass[i] = 0;
		    break;
		    }
	    }
	}
    fclose( infile );

    if( ! inetd_mode )
        dsocket = setup_dsocket();
    else
	{
	int fromlen = sizeof(frominet);
	dsocket = fileno( stdin );
	if( getpeername( dsocket, (struct sockaddr *)&frominet, &fromlen ) < 0 )
	    fatal(F,"Cannot getpeername(%d):  %e\n",dsocket);
	}
    negotiate( dsocket );
    setpass( pass, dsocket );
    readenv( dsocket );
#ifdef SYS5
	{
	static char *enp;
	enp = getenv("TERM");
	envarg = ( enp ? enp-5 : NULL );
	}
#endif
    tty_settings( TTY_SETUP, 0, 0 );
    extaddr = inet_ntoa(frominet.sin_addr);
    dpty = setup_pty("/bin/login","login","-h",extaddr,envarg,0);
    
    sprintf( buf, "%s %s %s", argv[0], extaddr, getptyname() );
    set_psmsg( buf );

    if( debugging )
	fprintf(stderr,"dsocket=%d dpty=%d\n",dsocket,dpty);
    xfer( 2, dsocket, dpty, -1, dpty, dsocket, -1 );
    cleanup_utmp();
    exit(0);
    }
