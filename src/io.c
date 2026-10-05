/************************************************************************
 *
 *indx#	io.c - Software to reading/writing between ses and sesd
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
 *doc#	Software for reading/writing between ses and sesd
 ************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#ifdef unix
#include <sys/ioctl.h> 
#endif
#include <stdarg.h>

#ifndef FIONREAD
#ifdef unix
#include <sys/filio.h>
#endif
#endif

#include "ses.h"
#include <string.h>

#ifdef HAVE_CRYPT
#include <crypt.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>

extern void fatal( char *fn, int ln, char *fmt, ... );

#ifdef NEED_LIBC_DEFS
extern char *strncpy( char *, const char *, int );
extern int strlen( const char * );
extern void exit( int );
extern int read( int, void *, int );
extern void *malloc( int );
extern void free( void * );
extern int write( int, void *, int );
extern int select( int, int*, int, int, int );
extern int ioctl( int, int, void * );

#ifdef HAVE_CRYPT
extern char *crypt( const char *, const char * );
#endif

#endif

extern int abbrev( const char *, const char ** );

static FILE *logfile = NULL;
static int logchan = -1;
/************************************************************************/
void setlog( char *name, char *func, int chan )
/************************************************************************/
/*	Open the file name for writing (func="w") or appending ("a").	*/
/*	Remember specified channel for read_dec which does actual	*/
/*	logging.							*/
/************************************************************************/
    {
    if( (logfile = fopen( name, func )) == NULL )
	fatal(F,"Cannot fopen(%s,%s) for logging:  %e\n",name,func);
    logchan = chan;
    }

static const char *enclist[] = { "none", "primitive", "enigma", NULL };
#define ENC_NONE		0
#define ENC_PRIM		1
#define ENC_ENIGMA		2

static char *key = NULL;
static int encchan = -1;
static int encmethod = ENC_PRIM;

/************************************************************************/
/*	A one-rotor machine designed along the lines of Enigma but	*/
/*	considerably trivialized.					*/
/************************************************************************/
#ifdef ENC_ENIGMA
#define ROTORSZ 256
#define MASK 0377
char	t1[ROTORSZ];
char	t2[ROTORSZ];
char	t3[ROTORSZ];

#define ENIGMA(c,n1,n2)					\
    {							\
    c = t2[(t3[(t1[(c+n1)&MASK]+n2)&MASK]-n2)&MASK]-n1;	\
    if( ++n1 >= ROTORSZ )				\
	{						\
	n1 = 0;						\
	if( ++n2 >= ROTORSZ )				\
	    n2 = 0;					\
	}						\
    }

#ifndef INT32
#define INT32	int
#endif

/************************************************************************/
void setup_enigma(char *pw)
/************************************************************************/
/*	Setup the rotor table.						*/
/************************************************************************/
    {
    int ic, i, k, temp;
    unsigned INT32 random;
    char buf[13];
    INT32 seed;

    strncpy(buf, pw, 8);
    buf[8] = buf[0];
    buf[9] = buf[1];
#ifdef HAVE_CRYPT
    strncpy(buf, crypt(buf, &buf[8]), 13);
#else
    strncpy( buf, "*************", 13 );
#endif
    seed = 123;
    for (i=0; i<13; i++)
	seed = seed*buf[i] + i;
    for(i=0;i<ROTORSZ;i++)
	{
	t1[i] = i;
	t3[i] = 0;
	}
    for(i=0;i<ROTORSZ;i++)
	{
	seed = 5*seed + buf[i%13];
	random = (unsigned INT32)(seed % 65521);
	k = ROTORSZ-1 - i;
	ic = (random&MASK)%(k+1);
	random >>= 8;
	temp = t1[k];
	t1[k] = t1[ic];
	t1[ic] = temp;
	if(t3[k]!=0) continue;
	ic = (random&MASK) % k;
	while(t3[ic]!=0) ic = (ic+1) % k;
	t3[k] = ic;
	t3[ic] = k;
	}
    for(i=0;i<ROTORSZ;i++)
	t2[t1[i]&MASK] = i;
    }
#endif

/************************************************************************/
void setpass( char *pass, int chan )
/************************************************************************/
/*	Remember encryption password and channel whose i/o is supposed	*/
/*	to be encrypted.  When called, it checks the capability list	*/
/*	to see what sort of encrytion is used so that read_dec and	*/
/*	write_enc can just switch off of an int instead of		*/
/*	continually calling bestcap().					*/
/************************************************************************/
    {
    char *encstr = bestcap("enc=");
    if( encstr == NULL )
	{
	fprintf(stderr,"No encryption technique.  Will not continue.\n");
	exit(1);
	}
    else
	{
        encchan = chan;
        encmethod = abbrev( encstr, enclist );
        if(debugging)fprintf(stderr,"[using %s = %d]\n",encstr,encmethod);
	if( encmethod < ENC_PRIM )	/* CMC:  Should be ENC_PRIM */
	    {
	    fprintf(stderr,"Insufficient encryption.  Will not continue.\n");
	    exit(1);
	    }
	key = pass;
	switch( encmethod )
	    {
	    case ENC_ENIGMA:	setup_enigma( pass );		break;
	    }
	}
    }

/************************************************************************/
/*	To debug encryption/decryption.					*/
/************************************************************************/
#ifdef NOTDEF
void pretty_buf( char *msg, char *bufp, int sz )
    {
    fprintf( stderr, "%s {",msg);
    while( sz-- > 0 )
        {
	if( *bufp >= ' ' && *bufp <= '~' )
	    fputc( *bufp, stderr );
	else
	    fprintf( stderr, "[%03o]", *bufp );
	bufp++;
	}
    fprintf( stderr, "}\n" );
    }
#else
#define pretty_buf( a, b, c )
#endif

/************************************************************************/
static int enc_ind = 0;
static int enc_last = 0;
static int enc_n1 = 0;
static int enc_n2 = 0;
void write_enc( int chan, char *buf, int len )
/************************************************************************/
/*	write_enc acts just like write() unless the channel was the	*/
/*	one handed to setpass().  In that case, switch on the		*/
/*	encryption type to different known encryption methods.		*/
/*	All errors are fatal ones.					*/
/************************************************************************/
    {
    int elen = len;
    char *ebuf = buf;

    pretty_buf( "write_enc", buf, len ); 
    if( key && chan==encchan )
	switch( encmethod )
	    {
	    case ENC_PRIM:
		while( elen-- > 0 )
		    {
		    int toxor = key[ enc_ind++ ];
		    if( toxor == 0 ) toxor = key[ enc_ind = 0 ];
		    toxor ^= enc_last;
		    enc_last = *ebuf;
		    *ebuf++ ^= toxor;
		    }
		break;

	    case ENC_ENIGMA:
		while( elen-- > 0 )
		    {
		    register int c = (*ebuf)&0xff;
		    ENIGMA( c, enc_n1, enc_n2 );
		    *ebuf++ = c;
		    }
		break;
	    }
    pretty_buf( "encodes to", buf, len );
    if( write( chan, buf, len ) < len )
	fatal(F,"write(%d,%d) failed:  %e\n",chan,len);
    }

/************************************************************************/
static int dec_ind = 0;
static int dec_last = 0;
static int dec_n1 = 0;
static int dec_n2 = 0;
void read_dec( int chan, char *buf, int len )
/************************************************************************/
/*	read_dec acts just like read() unless the channel was the	*/
/*	one handed to setpass().  In that case, switch on the		*/
/*	encryption type to different known decryption methods.		*/
/*	All errors are fatal ones.  If we reading from the logging	*/
/*	channel, then write what we've read to the logfile.		*/
/************************************************************************/
    {
    int elen = len;
    char *ebuf = buf;

    int ret = read( chan, buf, len );
    pretty_buf( "read_dec", buf, len );
    if( ret < len ) fatal(F,"read(%d,%d) returned %d:  %e\n",chan,len,ret);
    if( key && chan==encchan )
	switch( encmethod )
	    {
	    case ENC_PRIM:
		while( elen-- > 0 )
		    {
		    int toxor = key[ dec_ind++ ];
		    if( toxor == 0 ) toxor = key[ dec_ind = 0 ];
		    toxor ^= dec_last;
		    *ebuf ^= toxor;
		    dec_last = *ebuf++;
		    }
		break;

	    case ENC_ENIGMA:
		while( elen-- > 0 )
		    {
		    register int c = (*ebuf)&0xff;
		    ENIGMA( c, dec_n1, dec_n2 );
		    *ebuf++ = c;
		    }
		break;
	    }
    pretty_buf( "decodes to", buf, len );
    if( logfile && logchan == chan )
	{
	fwrite( buf, 1, len, logfile );
	if( debugging ) fflush(logfile);
	}
    }

/************************************************************************/
int xfer( int towatch, ... )
/************************************************************************/
/*	This routine was written to be able to handle i/o between lots	*/
/*	of different channels.  In practice, it will probably only ever	*/
/*	be used to to exchange data between two channels.  		*/
/*	The first argument specifies the number of channels to be	*/
/*	watching for i/o (probably 2).  For each input channel, there	*/
/*	are three subsequent arguments:  Channel to read, where to	*/
/*	write to and what character to exit if we see.  For instance,	*/
/*	if we wanted to exchange data between channels 4 and 5 and	*/
/*	we terminate on ^D from 4, we could call xfer with:		*/
/*	    xfer( 2, 4, 5, 004, 5, 4, -1 )				*/
/*	xfer returns the number of the channel that caused it to	*/
/*	exit (that is, saw the stop character or saw EOF).		*/
/*									*/
/*	Since we don't know which channel i/o is going to come from	*/
/*	next, we need to do a select() on all the possible input	*/
/*	channels and then figure out where the i/o was.  		*/
/************************************************************************/
    {
    va_list ap;
    fd_set mask;

    FD_ZERO( &mask );

    int maxchan = 0;

    struct xstruct
	{
	int	x_from;
	int	x_to;
	int	x_stop;
	fd_set	x_mask;
	} *xps, *xp;
    
    va_start( ap, towatch );
    xps = (struct xstruct *)malloc( sizeof(struct xstruct) * towatch );
    for( xp=xps; xp<&xps[towatch]; xp++ )
	{
	if( (xp->x_from = va_arg( ap, int )) > maxchan )
	    maxchan = xp->x_from;
	xp->x_to = va_arg( ap, int );
	xp->x_stop = va_arg( ap, int );
	FD_ZERO( &(xp->x_mask) );
	FD_SET( xp->x_from, &mask );
	}
    va_end( ap );

    if( debugging )
	{
#ifdef CAN_PRINT_MASK
	fprintf(stderr,"towatch=%d maxchan=%d mask=%o\n",towatch,maxchan,
	    (unsigned int)mask);
#else
	fprintf(stderr,"towatch=%d maxchan=%d\n",towatch,maxchan);
#endif
	for( xp=xps; xp<&xps[towatch]; xp++ )
#ifdef NOTDEFCAN_PRINT_MASK
	    fprintf(stderr,"xp:  %d->%d %o %d\n",
		xp->x_from,xp->x_to,(unsigned int)(xp->x_mask),xp->x_stop);
#else
	    fprintf(stderr,"xp:  %d->%d %d\n",
		xp->x_from,xp->x_to,xp->x_stop);
#endif
	}

    while( 1 )
	{
	fd_set readfds = mask;
	int selects = select( maxchan+1, &readfds, 0, 0, 0 );
	if( selects < 0 )
	    fatal(F,"select(%d/%d) failed:  %e (maxchan=%d)\n",
		readfds,mask,maxchan);
	for( xp=xps; selects>0 && xp<&xps[towatch]; xp++ )
	    {
#if PRE_LINUX
	    if( readfds & xp->x_mask )			/* Faster */
#else
	    if( FD_ISSET( xp->x_from, &readfds ) )	/* More portable */
#endif
		{
#define BIGBUF	4096
		char buf[BIGBUF];
		int numchars;

		selects--;

#ifdef FIONREAD
		size_t ioctl_numchars;
		if( ioctl( xp->x_from, FIONREAD, &ioctl_numchars ) < 0 )
		    fatal(F, "FIONREAD(%d) failed:  %e\n",
			xp->x_from);
		numchars = ioctl_numchars;
#else
		numchars = 1;
#endif
		
		if( numchars == 0 )
		    {
		    if( debugging )
			fprintf(stderr,"xfer %d->%d read %d\n",
			    xp->x_from, xp->x_to, (int)numchars );
		    free( xps );
		    return -1;
		    }

		if( numchars > BIGBUF )
		    numchars = BIGBUF;

		read_dec( xp->x_from, buf, (int)numchars );
		/* printf("Read %d [%.*s]\n",numchars,numchars,buf); fflush(stdout); */
		if( (buf[0]&0xff) == xp->x_stop )
		    return xp->x_from;
		write_enc( xp->x_to, buf, (int)numchars );
		}
	    }
	}
    }
