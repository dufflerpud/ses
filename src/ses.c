/************************************************************************
 *
 *indx#	ses.c - Client for ses encryption session software
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
 *doc#	Client for ses encryption session software
 ************************************************************************/
#include <stdio.h>
#include "ses.h"

#ifdef _AIX
#define unix		/* Dear god, you have GOT to be kidding */
#endif

#ifndef unix
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned long u_long;
typedef void * caddr_t;
#endif

#ifndef qunix
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#ifdef stellar
#include <netinet/net.h>
#endif
#include <netinet/in.h>
#include <sys/un.h>
#include <signal.h>
#include <sys/time.h>
#endif
#include <sys/ioctl.h>
#ifndef FIONREAD
#include <sys/filio.h>
#endif

#include "optional.h"
#include <stdarg.h>
#include <netdb.h>


#ifdef NOTDEF
extern char *getenv(), *strdup();
extern struct servent *getservbyname();
extern char *strrchr(), *malloc(), *strtok();
#endif

extern int errno;

#ifndef __bsdi__
extern char *sys_errlist[];
#endif

char *progname = NULL;
char *caplist[MAXCAP];
int debugging = 0;

#define READY	"[!!!ready!!!]"

#define BUFSIZE	2048
#define ETX	004

#ifdef NEED_LIBC_DEFS
extern int strlen( const char * );
extern int strcmp( const char *, const char * );
extern int strncmp( const char *, const char *, int );
extern void exit( int );
extern char *strcpy( char *, const char * );
extern int atoi( const char * );
extern void free( void * );
extern char *malloc( int );
extern int chdir( const char * );
extern void memcpy( void *, const void *, int );
extern int socket( int, int, int );
extern int connect( int, void *, int );
extern int bind( int, void *, int );
extern int close( int );
extern int open( const char *, int );
extern int fork();
extern int dup2( int, int );
extern int execvp( const char *, const char ** );
extern int waitpid( int, void *, int );
extern time_t time( time_t * );
extern int select( int, int *, int *, int *, struct timeval * );
extern int chmod( const char *, int );
extern int listen( int, int );
extern int accept( int, void *, void * );
extern int read( int, void *, int );
extern int write( int, void *, int );
extern FILE *popen( const char *, const char * );
extern int pclose( FILE * );
extern int system( const char * );
extern int ioctl( int, int, void * );
extern int gethostname( char *, int );
extern void putenv( const char * );
extern int inet_addr( const char * );
extern const char *inet_ntoa( struct in_addr );
#endif

extern void setpass( const char *, int );
extern void setlog( const char *, const char *, int );
extern void write_enc( int, void *, int );
extern void read_dec( int, void *, int );
extern int tty_settings( int );
extern int xfer( int, int, int, int, int, int, int );
extern int abbrev( const char *, const char ** );
extern int setup_pty(char *e0,char *e1, char *e2, char *e3, char *e4, char *e5 );

/************************************************************************/
void usage( char *fmt, ... )
/************************************************************************/
/*	This routine prints out the UNIX usage and then exits.		*/
/************************************************************************/
    {
    va_list ap;
    va_start( ap, fmt );
    vfprintf(stderr,fmt,ap);
    fprintf(stderr,"\nUsage:  %s <argument>\n\n",progname);
    fprintf(stderr,"Where <argument> is one or more of:\n");
    fprintf(stderr,"    <hostname>         Create sesson on remote host\n");
    fprintf(stderr,"    <special_file>     Open session on named file\n");
    fprintf(stderr,"    -p <socketfile>    Name socket to talk to ses with\n");
    fprintf(stderr,"    -d                 Enter daemon mode\n");
    fprintf(stderr,"    -q                 Quit daemon mode\n");
    fprintf(stderr,"    -l <logfile>       Copy output to logfile\n");
    fprintf(stderr,"    -a <logfile>       Append output to logfile\n");
    fprintf(stderr,"    -s <command>       Send specified command to pty\n");
    fprintf(stderr,"    -e <secs> <expect> Wait specified secs for string\n");
    fprintf(stderr,"    -i <escapechar>    Enter interactive mode\n");
    fprintf(stderr,"    -D                 Turn on debugging\n");
    fprintf(stderr,"    -c 'local command' Pipe to/from local command\n");
    fprintf(stderr,"    -fi                (Used for file transmission\n");
    fprintf(stderr,"    -fo                (Used for file transmission\n");
    exit(1);
    }

/************************************************************************/
void negotiate( int chan )
/************************************************************************/
/*	Exchange information about capabilities with the remote sesd.	*/
/************************************************************************/
    {
    static char capbuf[100];
    char *sesptr = getenv("SESCAP");
    int len = strlen(sesptr) + 1;
    int ind;

    sprintf( capbuf, "%d", len );
    strcpy( capbuf+5, sesptr );
    write_enc( chan, capbuf, len+5 );
    read_dec( chan, capbuf, 5 );
    len = atoi(capbuf);
    read_dec( chan, capbuf, len );
    for(caplist[ind=0]=strtok(capbuf,",");
	caplist[ind];
	caplist[++ind]=strtok(NULL,",") )
	if( debugging )
	    fprintf(stderr,"[ses server can %s]\r\n",caplist[ind]);
    caplist[++ind] = NULL;
    }

/************************************************************************/
void expenv( int chan )
/************************************************************************/
/*	Malloc enough space for the contents of the variables in the	*/
/*	strdup(), create one huge string with all of them and send the	*/
/*	contents to the daemon.						*/
/************************************************************************/
    {
    char *vars = strdup( "DISPLAY,TERM,LINES,COLUMNS" );
    char *xfrenv = NULL;
    char *var, *val;
    char numbuf[10];
    int size = 5;
    int ind = 5;
    int pass;
    for( pass=0; pass<2; pass++ )
	{
	char *vl = strdup(vars);
	for( var=strtok(vl,","); var; var=strtok(NULL,",") )
	    {
	    if( (val = getenv( var )) == NULL )
		if( strcmp(var,"LINES") == 0 )
		    sprintf( val=numbuf, "%d", tty_settings(TTY_ROWS) );
		else if( strcmp(var,"COLUMNS") == 0 )
		    sprintf( val=numbuf, "%d", tty_settings(TTY_COLS) );
		else
		    val="";
	    if( pass == 0 )
		size += ( strlen(var) + strlen(val) + 2 );
	    else
		{
		sprintf( xfrenv+ind, "%s=%s", var, val );
		ind += ( strlen( xfrenv+ind ) + 1 );
		}
	    }
	if( pass == 0 )
	    {
	    xfrenv = malloc( size );
	    sprintf( xfrenv, "%04d", size-5 );
	    }
	free( vl );
	}
    write_enc( chan, xfrenv, size );
    }

/************************************************************************/
struct hostent *string_to_host( char *hname )
/************************************************************************/
/*	Convert IP address or hostname to hostent.			*/
/************************************************************************/
    {
    static struct hostent res;
    static long taddr;

    if( (taddr = inet_addr( hname )) != (unsigned long) -1 )
	{
	static char *block[2];
	res.h_name = hname;
	res.h_aliases = NULL;
	res.h_addrtype = AF_INET;
	res.h_length = 4;
#ifndef NOT43
	res.h_addr_list = (char**)block;
	block[0] = (char*)&taddr;
	block[1] = NULL;
#else
	res.h_addr = temp;
#endif
	return &res;
	}
	
    return gethostbyname(hname);
    }

/************************************************************************/
int setup_net_connect( char *hname )
/************************************************************************/
/*	Setup connection to daemon.					*/
/************************************************************************/
    {
    struct servent *sp;
    struct sockaddr_in sin;
    int debug = 1;
    register struct hostent *host = 0;
    int net;
    int connected = 0;
    u_short hport;

    if( (sp = getservbyname( SERVICENAME, "tcp" )) != NULL )
	hport = ntohs(sp->s_port);
    else
	hport = SERVICEPORT;

    if( (host = string_to_host(hname)) == NULL )
	fatal(USER_ERROR,"\"%s\" is a unknown host\n", hname );
    sin.sin_family = host->h_addrtype;
    memcpy((caddr_t)&sin.sin_addr,
#ifndef	NOT43
	host->h_addr_list[0],
#else	/* NOT43 */
	host->h_addr,
#endif	/* NOT43 */
	host->h_length);

    sin.sin_port = htons( hport );

    do  {
	net = socket(AF_INET, SOCK_STREAM, 0);
	if (net < 0)
	    fatal(F,"Socket call failed:  %e\n");
#ifdef notdef
	if( debug &&
#ifndef	NOT43
	    setsockopt(net, SOL_SOCKET, SO_DEBUG, (char *)&debug, sizeof(debug))
#else	/* NOT43 */
	    setsockopt(net, SOL_SOCKET, SO_DEBUG, 0, 0)
#endif	/* NOT43 */
	    < 0 )
	    fatal(F,"setsockopt (SO_DEBUG) failed:  %e\n");
#endif

	if (connect(net, (struct sockaddr *)&sin, sizeof (sin)) < 0)
	    {
#ifndef	NOT43
	    if (host && host->h_addr_list[1])
		{
		fprintf(stderr, "connect to address %s:  %s\n",
		    inet_ntoa(sin.sin_addr),STRERROR(errno));
		host->h_addr_list++;
		memcpy((caddr_t)&sin.sin_addr,
		    host->h_addr_list[0], host->h_length);
		fprintf(stderr, "Trying %s...\n", inet_ntoa(sin.sin_addr));
		(void) close(net);
		continue;
		}
#endif	/* NOT43 */
	    fatal(USER_ERROR,"connect %s(%d/tcp) failed:  %e\n",
		hname,ntohs(sin.sin_port));
	    }
	connected++;
	} while (connected == 0);
    return net;
    }

/************************************************************************/
int setup_usock( char *unixsock )
/************************************************************************/
/************************************************************************/
    {
    int debug = 1;
    int sock;
    struct sockaddr_un sunx;

    if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 )
	fatal(F,"Socket call failed:  %e\n");
#ifdef notdef
    if( debug &&
#ifndef	NOT43
        setsockopt(sock, SOL_SOCKET, SO_DEBUG, (char *)&debug, sizeof(debug))
#else	NOT43
        setsockopt(sock, SOL_SOCKET, SO_DEBUG, 0, 0)
#endif	NOT43
	< 0)
	fatal(F,"setsockopt (SO_DEBUG) failed:  %e\n");
#endif

    sunx.sun_family = AF_UNIX;
    strcpy( sunx.sun_path, unixsock );
    if (connect(sock, (struct sockaddr *)&sunx, sizeof(sunx) ) < 0)
	fatal(USER_ERROR,"connect %s failed:  %e\n",unixsock);
    return sock;
    }

/************************************************************************/
int setup_dev( char *devspec )
/************************************************************************/
    {
    int kidpid;
    int chan;
    char *args[100], *dev, *buf = strdup( devspec );
    printf("setup_dev(%s) called.\n",devspec); fflush(stdout);
    strcpy( buf, devspec );
    dev = strtok(buf,", ");
    printf("open(%s) called\r\n",dev);	fflush(stdout);
    if( (chan = open( dev, 2 )) < 0 )
	fatal(USER_ERROR,"Can't open device %s:  %e\n",dev);
    if( (kidpid = fork()) < 0 )
	fatal(F,"Cannot fork():  %e\n");
    else if( kidpid == 0 )
	{
	int i = 0;
	args[i++] = "stty";
	args[i++] = "raw";
	args[i++] = "-echo";
	while( args[i]=strtok(NULL,", ") )
	    i++;
	printf("stty(%s %s %s %s) being invoked.\r\n",args[0],args[1],args[2],args[3]); fflush(stdout);
	close( 0 );
	close( 1 );
	dup2( chan, 0 );
	dup2( chan, 1 );
	execvp( args[0], args );
	fatal(F,"exec(%s) failed:  %e\n",args[0]);
	}
#ifdef __NeXT__
    wait4( kidpid, NULL, NULL, NULL );
#else
#ifdef titan
    wait( NULL );
#else
    waitpid( kidpid, NULL, 0 );
#endif
#endif
    return chan;
    }

/************************************************************************/
int doexpect( int chan, int etime, char *str, int size )
/************************************************************************/
    {
    int ind = 0;
    int cind1, cind2;
    int totalread = 0;
    char *cmpbuf = malloc( size );
    int found = 0;
    time_t now, whenstop;
    time( &now );
    whenstop = now + etime;
    while( !found )
	{
	fd_set mask;
	FD_ZERO( &mask );
	FD_SET( chan, &mask );
	struct timeval tmv;
	time( &now );
	if( (tmv.tv_sec = whenstop - now) <= 0 ) break;
	tmv.tv_usec = 0;
	if( select(10,&mask,0,0,&tmv) )
	    {
	    read_dec( chan, &cmpbuf[ind], 1 );
#ifdef debug
	    printf("%s",printable(cmpbuf[ind]&0xff));
	    fflush(stdout);
#endif
	    ind = (ind+1)%size;
	    if( ++totalread >= size )
		{
		cind1 = ind;
		for( cind2=0; cind2<size; cind2++ )
		    if( cmpbuf[cind1] == str[cind2] )
			cind1 = (cind1+1)%size;
		    else
			break;
		if( cind2 >= size ) found=totalread;
		}
	    }
	}
    free( cmpbuf );
    return found;
    }

/************************************************************************/
void daemonmode( char *unixsock, int other )
/************************************************************************/
    {
    struct sockaddr_un sunx;
    int debug = 1;
    int sock;
    char cmd[100];
    int sizecmd;
    int inloop = 1;
    int boundchan;
    char res;

    if( (sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 )
	fatal(F,"Socket call failed:  %e\n");
#ifdef notdef
    if( debug &&
#ifndef	NOT43
        setsockopt(sock, SOL_SOCKET, SO_DEBUG, (char *)&debug, sizeof(debug))
#else	NOT43
        setsockopt(sock, SOL_SOCKET, SO_DEBUG, 0, 0)
#endif	NOT43
	< 0 )
	fatal(F,"setsockopt (SO_DEBUG) failed:  %e\n");
#endif

    sunx.sun_family = AF_UNIX;
    strcpy( sunx.sun_path, unixsock );
    if (bind( sock, (struct sockaddr *)&sunx, sizeof(sunx)) < 0)
	fatal(F,"bind(%s) failed:  %e\n",unixsock);
    if( chmod( unixsock, 0600 ) < 0 )
	fatal(F,"chmod(%s,0600) failed:  %e\n",unixsock);
    
#ifdef undef
    if( fork() != 0 )
	exit(0);
#endif

    while( inloop )
	{
	int acceptsize = sizeof(sunx);
	if( listen( sock, 5 ) < 0 )
	    fatal(F,"listen(%d) failed:  %e\n",sock);
	if( (boundchan = accept( sock, (struct sockaddr *)&sunx, &acceptsize ) ) < 0 )
	    fatal(F,"accept(%d) failed:  %e\n",sock);
	while( inloop )
	    {
	    if( read( boundchan, cmd, 1 ) < 1 ) break;
	    sizecmd = cmd[0];
	    if( read( boundchan, cmd, sizecmd ) < sizecmd ) break;
	    switch( cmd[0] )
		{
		case C_EXPECT:
		    res = (doexpect(other,(int)(cmd[1]),cmd+2,sizecmd-2)>0);
		    write( boundchan, &res, 1 );
		    break;
		case C_SEND:
		    write_enc( other, cmd+1, sizecmd-1 );
		    break;
		case C_INTERACTIVE:
		    if( xfer(2,boundchan,other,-1,other,boundchan,-1) < 0 )
			inloop = 0;
		    break;
		case C_QUIT:
		    inloop = 0;
		    break;
		default:
		    fatal(F,"Bad function (%d)\n",cmd[0]);
		}
	    }
	close( boundchan );
	}
    close( boundchan );
    close( other );
    unlink( unixsock );
    exit(0);
    }

/************************************************************************/
char *compactstr( char *str )
/************************************************************************/
    {
    int len;
    int c;
    int numlen;
    char *res = malloc( strlen(str)+1 );
    for( len=0; *str; str++ )
	{
	if( *str != '\\' )
	    c = *str;
	else
	    switch( *++str )
		{
		case 'r':	c='\r';		break;
		case 'n':	c='\n';		break;
		case 't':	c='\t';		break;
		case 'b':	c='\b';		break;
		default:
		    c = 0;
		    for( numlen=0; numlen<3 && *str>='0' && *str<'8'; numlen++ )
			c = 8*c + *str++ - '0';
		    str--;
		    break;
		}
	res[len++] = c;
	}
    res[len] = 0;
    return res;
    }

/************************************************************************/
FILE *namecmd( char *fmt, char *namelist, char *rwflag )
/************************************************************************/
    {
    char buf[BUFSIZ];
    char *cp;

    for( cp=namelist; *cp; cp++ )
	switch( *cp )
	    {
	    case '>': case '<': case '|':
	    case ';': case '`': case '"': case '\'':
		*cp = ' ';
		break;
	    }

    sprintf( buf, fmt, namelist );
    if( rwflag )
	return popen( buf, rwflag );
    else
	{
	system( buf );
	return NULL;
	}
    }

/************************************************************************/
void uud( int c, FILE *outfile )
/************************************************************************/
    {
    static char buf[100];
    static int ind = 0;
    int n;
    int i, j;
    int c0, c1, c2, c3;

    if( c == 0400 )
	{
	ind = 0;
	return;
	}
    else if( c == '\r' )
	return;
    else if( c != '\n' )
	{
	buf[ind++] = c;
	return;
	}
    ind = 0;
    if( strncmp(buf,"begin",5) == 0 )
	return;

    n = buf[0] - ' ';
    if( n == 0 ) return;

    for( j=i=0; i<n; i++ )
	switch( i%3 )
	    {
	    case 0:	c0=buf[++j]-' ';
			c1=buf[++j]-' ';
			c2=buf[++j]-' ';
			c3=buf[++j]-' ';
			putc( ((c0<<2) | (c1>>4)), outfile );		break;
	    case 1:	putc( ((c1&017)<<4 | (c2>>2)), outfile );	break;
	    case 2:	putc( ((c2&03)<<6 | c3), outfile );		break;
	    }
    }

/************************************************************************/
void stream_in()
/************************************************************************/
    {
    FILE *outfile;
    char buf[BUFSIZ];
    int i, j;
    tty_settings( TTY_GET );
    tty_settings( TTY_BIN );

    printf("%s",READY);
    fflush(stdout);
    
    if( (outfile = popen("cpio -idumc","w")) == NULL )
	fatal(F,"cpio failed:  %e\n");
    
    while( (i=read(0,buf,BUFSIZ)) > 0 && strcmp(buf,"end\n") )
	{
	for( j=0; j<i; j++ )
	    if( buf[j] == ETX )
		i = 0;
	    else
	        uud( buf[j], outfile );
	if( i == 0 ) break;
	}
    tty_settings( TTY_RESET );
    exit( 0 );
    }

/************************************************************************/
void stream_out()
/************************************************************************/
    {
    char buf[BUFSIZ];
    int stderrchan;
    printf("%s",READY);
    fflush(stdout);
    fgets( buf, BUFSIZE-1, stdin );
    tty_settings( TTY_GET );
    tty_settings( TTY_BIN );
    printf("%s",READY);
    fflush(stdout);
    fclose(stderr);
    stderrchan = open("/dev/null",2);
    if( stderrchan != 2 ) dup2( stderrchan, 2 );
    namecmd( "find %s -print | cpio -oc |  uuencode a", buf, NULL );
    putchar( ETX );
    fflush( stdout );
    tty_settings( TTY_RESET );
    exit( 0 );
    }

/************************************************************************/
int do_get( int other, char *namelist )
/************************************************************************/
    {
    FILE *outfile;
    int state = 1;
    char buf[BUFSIZ];
    time_t start, finish, since, now, diff;
    long size = 0;

    strcpy( buf, SES_OUT );
    write_enc( other, buf, sizeof(SES_OUT)-1 );
    if( ! doexpect(other,10,READY,sizeof(READY)-1) )
	{
	printf("[Cannot synchronize with remote server SES_OUT command]\n");
	return 0;
	}
    sprintf( buf, "%s\n", namelist );
    write_enc( other, buf, strlen(buf) );
    if( ! doexpect(other,10,READY,sizeof(READY)-1) )
	{
	printf("[Cannot synchronize with remote server file list]\n");
	return 0;
	}
    
    time( &start );

    if( (outfile = popen("cpio -idumc","w")) == NULL )
	fatal(F,"cpio failed:  %e\n");

    uud( 0400, outfile );
    since = start;
    while( state )
	{
        struct timeval tmv;
	fd_set mask;
	FD_ZERO( &mask );
	FD_SET( other, &mask );
	tmv.tv_sec = 2;
	tmv.tv_usec = 0;
	if( select(other+1,&mask,0,0,&tmv) != 0 )
	    {
	    size_t i;
	    size_t numchars;
#ifdef FIONREAD
	    if( ioctl( other, FIONREAD, &numchars ) < 0 )
		fatal(F,"ioctl(%d,FIONREAD) failed:  %e\n",other);
#else
	    numchars = 1;
#endif
	    if( numchars > BUFSIZ ) numchars = BUFSIZ;
	    read_dec(other,buf,(int)numchars);
	    size += numchars;
	    for( i=0; i < numchars; i++ )
		if( buf[i] == ETX )
		    break;
		else
		    uud( buf[i] & 0xff, outfile );
	    if( i < numchars )
		break;
	    time( &now );
	    if( now - since > 3 )
		{
		fprintf(stderr,"%ld/%ld = %ld BPS          \r",
		    size*3/4, now-start, (size*3/4)/(now-start) );
		since = now;
		}
	    }
	}
    pclose( outfile );

    time( &finish );
    diff = finish - start;
    if( diff <= 0 ) diff = 1;
    size = 3*size / 4;
    fprintf(stderr,"[Received %ld bytes in %ld seconds or %ld BPS]\r\n",
	size,diff,size/diff);
    }

/************************************************************************/
int do_put( int other, char *namelist )
/************************************************************************/
    {
    FILE *infile;
    char buf[BUFSIZ];
    int buflen = 0;
    time_t start, finish, diff, now, since;
    long size = 0;

    strcpy( buf, SES_IN );
    write_enc( other, buf, sizeof(SES_IN)-1 );
    if( ! doexpect(other,10,READY,sizeof(READY)-1) )
	{
	printf("[Cannot synchronize with remote server]\n");
	return 0;
	}

    time( &start );
    since = start;

    infile = namecmd( "find %s -print | cpio -oc | uuencode a", namelist, "r" );
    while( (buflen=fread(buf,1,BUFSIZ,infile)) > 0 )
	{
	size += buflen;
	write_enc( other, buf, buflen );
	time( &now );
	if( now - since > 3 )
	    {
	    fprintf(stderr,"%ld/%ld = %ld BPS          \r",
		size*3/4, now-start, (size*3/4)/(now-start) );
	    since = now;
	    }
	}
    buf[0] = ETX;
    write_enc( other, buf, 1 );
    pclose( infile );

    time( &finish );
    diff = finish - start;
    if( diff <= 0 ) diff = 1;
    size = 3*size / 4;
    fprintf(stderr,"[Sent %ld bytes in %ld seconds or %ld BPS]\r\n",
	size,diff,size/diff);
    }

/************************************************************************/
void interhelp( char *fmt, ... )
/************************************************************************/
    {
    va_list ap;
    va_start( ap, fmt );
    vfprintf(stderr,fmt,ap);
    fprintf(stderr,"\nCommands are:\n");
    fprintf(stderr,"    quit\n");
    fprintf(stderr,"    cd directory\n");
    fprintf(stderr,"    get files\n");
    fprintf(stderr,"    put files\n");
    fprintf(stderr,"    (normal shell commands)\n");
    }

const char *cmdlist[] = {"?","quit","cd","get","put",NULL};
#define COM_HELP	0
#define COM_QUIT	1
#define COM_CD		2
#define COM_GET		3
#define COM_PUT		4

/************************************************************************/
int docommand( int other )
/************************************************************************/
    {
    char cmdbuf[BUFSIZ], parsebuf[BUFSIZ], hostname[100], *cp;
    int com;

    gethostname( hostname, 99 );
    printf("\n");
    while( 1 )
	{
	printf("%s ses>  ",hostname);
	fflush(stdout);

	if( fgets(cmdbuf,BUFSIZE-1,stdin) == NULL ) return 1;
	strcpy( parsebuf, cmdbuf );
	if( (cp = strtok( parsebuf, " \t\n" )) == NULL ) return 0;
	switch( com = abbrev( cp, cmdlist ) )
	    {
	    case COM_HELP:
		interhelp("");
		break;
	    case COM_QUIT:
		return 1;
	    case COM_CD:
		if( (cp = strtok( NULL, " \t\n" )) == NULL )
		    interhelp("Must specify directory to '%s'.",cmdlist[com]);
		else
		    if( chdir( cp ) < 0 )
			fprintf(stderr,"Cannot cd to %s:  %s\n",
			    cp,STRERROR(errno));
		break;
	    case COM_GET:
		if( (cp = strtok( NULL, "\377" )) == NULL )
		    interhelp("Must specify files to '%s'.",cmdlist[com]);
		else
		    do_get( other, cp );
		break;
	    case COM_PUT:
		if( (cp = strtok( NULL, "\377" )) == NULL )
		    interhelp("Must specify files to '%s'.",cmdlist[com]);
		else
		    do_put( other, cp );
		break;
	    default:
		system( cmdbuf );
		break;
	    }
	}
    }

/************************************************************************/
int main( int argc, char *argv[] )
/************************************************************************/
    {
    int i;
    int other = -1;
    char *conto = NULL;
    char *logname = NULL;
    char *logfunc = NULL;
    char *unixsock = NULL;
    int ind, maxind = 0;
    int exchan = -1;
    char cmdbuf[100];
    int working = 1;

    struct commands
	{
	char		*c_value;
	int		c_func, c_time;
	} cmds[100];
    
    putenv("SESCAP=enc=none,enc=primitive,enc=enigma");
    /* putenv("SESCAP=enc=none,enc=primitive"); */
    
    if( (progname=strrchr(argv[0],'/')) == NULL )
	progname = argv[0];
    else
	progname++;

    for( i=1; i<argc; i++ )
	if( strcmp(argv[i],"-D")==0 )
	    debugging++;
	else if( strcmp(argv[i],"-p")==0 )
	    unixsock = argv[++i];
	else if( strcmp(argv[i],"-h")==0 )	/* Historical */
	    conto = argv[++i];
	else if( strcmp(argv[i],"-l")==0 )
	    {
	    logname = argv[++i];
	    logfunc = "w";
	    }
	else if( strcmp(argv[i],"-a")==0 )
	    {
	    logname = argv[++i];
	    logfunc = "a";
	    }
	else if( strcmp(argv[i],"-d")==0 )
	    {
	    cmds[maxind].c_func = C_DAEMON;
	    cmds[maxind].c_value = NULL;
	    cmds[maxind].c_time = 0;
	    maxind++;
	    }
	else if( strcmp(argv[i],"-q")==0 )
	    {
	    cmds[maxind].c_func = C_QUIT;
	    cmds[maxind].c_value = NULL;
	    cmds[maxind].c_time = 0;
	    maxind++;
	    }
	else if( strcmp(argv[i],"-s")==0 )
	    {
	    cmds[maxind].c_func = C_SEND;
	    cmds[maxind].c_value = compactstr( argv[++i] );
	    cmds[maxind].c_time = 0;
	    maxind++;
	    }
	else if( strcmp(argv[i],"-e")==0 )
	    {
	    int itime;
	    cmds[maxind].c_func = C_EXPECT;
	    cmds[maxind].c_value = compactstr( argv[++i] );
	    if( (itime = atoi( argv[i+1] )) == 0 )
		cmds[maxind].c_time = DEFTIME;
	    else
		{
		cmds[maxind].c_time = itime;
		i++;
		}
	    fprintf(stderr,"%s/%d maxind=%d itime=%d\n",F,maxind,itime);
	    maxind++;
	    }
	else if( strcmp(argv[i],"-i")==0 )
	    {
	    cmds[maxind].c_func = C_INTERACTIVE;
	    if( i < argc-1 )
	        cmds[maxind].c_value = argv[++i];
	    else
	        cmds[maxind].c_value = DEFESCAPE;
	    cmds[maxind].c_time = 0;
	    maxind++;
	    }
	else if( strcmp(argv[i],"-fi")==0 )
	    stream_in();
	else if( strcmp(argv[i],"-fo")==0 )
	    stream_out();
	else if( argv[i][0] == '-' )
	    usage( "Unknown argument:  %s\n", argv[i] );
	else if( conto )
	    usage( "Device specified multiple times.\n" );
	else
	    conto = argv[i];

    if( maxind == 0 )
	{
	cmds[maxind].c_func = C_INTERACTIVE;
	cmds[maxind].c_value = DEFESCAPE;
	cmds[maxind].c_time = 0;
	maxind++;
	}
    
    tty_settings(TTY_GET);

    for( ind=0; working && ind<maxind; ind++ )
	{
	int vlen = 0;
	if( cmds[ind].c_value )
	    vlen = strlen( cmds[ind].c_value );
	if( other <= 0 )
	    if( cmds[ind].c_func != C_DAEMON && unixsock )
		{
		other = setup_usock( unixsock );
		}
	    else
		{
		if( conto == NULL )
		    {
		    char *shell = getenv("SHELL");
		    char *shortshell;
		    if( shell == NULL ) shell = "/bin/sh";
		    if( (shortshell=strrchr(shell,'/')) == NULL )
			shortshell = shell;
		    else
			shortshell++;
		    if( strcmp(shortshell,"csh")==0 || 
			strcmp(shortshell,"sh")==0 )
			other = setup_pty( shell, shortshell, 0, 0, 0, 0 );
		    else
			other = setup_pty( shell, shortshell, "-i", 0, 0, 0 );
		    }
		else if( conto[0] != '/' )
		    {
		    char *inpass, buf[30];
		    sprintf( buf, "Enter %s password:  ", conto );
		    inpass = getpass( buf );
		    other = setup_net_connect( conto );
		    negotiate( other );
		    setpass( inpass, other );
		    expenv( other );
		    }
		else
		    {
		    other = setup_dev( conto );
		    }
		if( logname ) setlog( logname, logfunc, other );
		}
	else if( cmds[ind].c_func == C_DAEMON )
	    usage("PTY connection already established before -d(%d).\n",
		ind);
	switch( cmds[ind].c_func )
	    {
	    case C_DAEMON:
		if( unixsock == NULL )
		    usage("Must specify unix socket with -d.\n");
		daemonmode( unixsock, other );
		break;

	    case C_EXPECT:
		if( !unixsock )
		    working =
			doexpect(other,cmds[ind].c_time,cmds[ind].c_value,vlen);
		else
		    {
		    cmdbuf[0] = strlen( cmds[ind].c_value ) + 2;
		    cmdbuf[1] = cmds[ind].c_func;
		    cmdbuf[2] = cmds[ind].c_time;
		    strcpy( &cmdbuf[3], cmds[ind].c_value );
		    write_enc( other, cmdbuf, strlen(cmds[ind].c_value) + 3 );
		    read_dec( other, cmdbuf, 1 );
		    working = cmdbuf[0];
		    }
		break;

	    case C_SEND:
		if( !unixsock )
		    write_enc(other,cmds[ind].c_value,vlen);
		else
		    {
		    cmdbuf[0] = strlen( cmds[ind].c_value ) + 1;
		    cmdbuf[1] = cmds[ind].c_func;
		    strcpy( &cmdbuf[2], cmds[ind].c_value );
		    write_enc( other, cmdbuf, strlen(cmds[ind].c_value) + 2 );
		    }
		break;

	    case C_INTERACTIVE:
		if( unixsock )
		    {
		    cmdbuf[0] = 1;
		    cmdbuf[1] = cmds[ind].c_func;
		    write_enc( other, cmdbuf, 2 );
		    }
		while( 1 )
		    {
		    printf("[Interactive mode, escape character is '%s']\n",
			printable(cmds[ind].c_value[0]&0xff));
		    fflush(stdout);
		    tty_settings(TTY_RAW);
		    exchan=xfer(2,fileno(stdin),other,cmds[ind].c_value[0],
			other,fileno(stdout),-1);
		    tty_settings(TTY_RESET);
		    if( exchan < 0 ) break;
		    if( docommand( other ) ) break;
		    }
		printf("\n[Connection closed]\n");
		break;

	    case C_QUIT:
		if( unixsock != NULL )
		    {
		    cmdbuf[0] = 1;
		    cmdbuf[1] = cmds[ind].c_func;
		    write( other, cmdbuf, 2 );
		    }
		break;

	    default:
		fatal(F,"Bad function (%d) for argument %d\n",
		    cmds[ind].c_func,ind);
	    }
	if( other == exchan ) break;
	}

    exit( working==0 );
    return( working == 0 );
    }
