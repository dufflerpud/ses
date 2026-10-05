#indx#	Makefile - Makefile for building ses and sesd
#@HDR@	$Id$
#@HDR@
#@HDR@	Copyright (c) 2024-2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
#@HDR@
#@HDR@	Permission is hereby granted, free of charge, to any person
#@HDR@	obtaining a copy of this software and associated documentation
#@HDR@	files (the "Software"), to deal in the Software without
#@HDR@	restriction, including without limitation the rights to use,
#@HDR@	copy, modify, merge, publish, distribute, sublicense, and/or
#@HDR@	sell copies of the Software, and to permit persons to whom
#@HDR@	the Software is furnished to do so, subject to the following
#@HDR@	conditions:
#@HDR@	
#@HDR@	The above copyright notice and this permission notice shall be
#@HDR@	included in all copies or substantial portions of the Software.
#@HDR@	
#@HDR@	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
#@HDR@	KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
#@HDR@	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
#@HDR@	AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#@HDR@	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#@HDR@	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#@HDR@	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#@HDR@	OTHER DEALINGS IN THE SOFTWARE.
#
#hist#	2026-02-10 - Christopher.M.Caldwell0@gmail.com - Created
########################################################################
#doc#	Makefile - Makefile for building ses and sesd
########################################################################
PROJECTSDIR?=$(shell echo $(CURDIR) | sed -e 's+/projects/.*+/projects+')
#PROGRAMS=ses sesd info uud stty2c
PROGRAMS=ses sesd
include $(PROJECTSDIR)/common/Makefile.std

L_NSL=`[ ! -r /usr/lib/libnsl.so ] || echo -lnsl`
L_SOCK=`[ ! -r /usr/lib/libsocket.a ] || echo -lsocket`
L_POSIX=`[ ! -r /usr/lib/libposix.a ] || echo -lposix`
L_CRYPT=`[ ! -r /usr/lib/libcrypt.a ] || echo -lcrypt`
STREAMSPTY=`[ ! -c /dev/ptmx ] || echo -DSTREAMSPTY`
SYS5=`[ ! -c /dev/ptmx ] || echo -DSYS5`
CFLAGS=-g -I$(SRCDIR) -DCONFIG='"$(SYSTEMETC)/ses.cfg"'
LIBS=$(L_NSL) $(L_SOCK) $(L_CRYPT)
XTMPS=/etc /var/adm /usr/adm
UTMP=`for d in $(XTMPS); do \
    [ ! -f $$d/utmp ] || echo -DUTMP=\"$$d/utmp\" && break ; \
    done`
WTMP=`for d in $(XTMPS); do \
    [ ! -f $$d/wtmp ] || echo -DWTMP=\"$$d/wtmp\" && break ; \
    done`

TARGETS=$(ABINS) $(ASCRIPTS)

all:		$(ABINS)
		rm -f /tmp/sesdebug*

install:	$(TARGETS)
		[ -d $(EXEBIN) ] || mkdir -p $(EXEBIN)
		chmod 755 $(EXEBIN)
		cd $(EXEBIN) ; rm -f $(LBINS)
		[ "$(AEXEBIN)" = "$(EXEBIN)" ] || \
		    for f in $(LBINS); do \
			ln -s $(AEXEBIN)/$$f $(EXEBIN)/$$f ; \
		    done
		[ -d $(AEXEBIN) ] || mkdir -p $(AEXEBIN)
		chmod 755 $(AEXEBIN)
		( cd $(AEXEBIN); rm -f $(ABINS) )
		cp $(ABINS) $(AEXEBIN)
		cd $(AEXEBIN) ; chmod 755 $(ABINS)
		[ -d $(MAN8) ] || mkdir -p $(MAN8)
		chmod 755 $(MAN8)
		cp $(MAN8S) $(MAN8)
		cd $(MAN8) ; chmod 644 $(MAN8S)
		[ -d $(MAN1) ] || mkdir -p $(MAN1)
		chmod 755 $(MAN1)
		cp $(MAN1S) $(MAN1)
		cd $(MAN1) ; chmod 644 $(MAN1S)

$(OBJDIR)/sesd.o: \
		$(SRCDIR)/sesd.c $(SRCDIR)/ses.h $(SRCDIR)/optional.h
		@$(MKDIR) -p $(OBJDIR)
		$(CC) $(CFLAGS) $(SYS5) -c $< -o $@

$(OBJDIR)/ses.o: \
		$(SRCDIR)/ses.c $(SRCDIR)/ses.h $(SRCDIR)/optional.h
		@$(MKDIR) -p $(OBJDIR)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/unixpty.o: \
		$(SRCDIR)/unixpty.c $(SRCDIR)/ses.h
		@$(MKDIR) -p $(OBJDIR)
		$(CC) $(CFLAGS) $(STREAMSPTY) $(UTMP) $(WTMP) -c $< -o $@

$(OBJDIR)/%.o:	$(SRCDIR)/%.c
		@$(MKDIR) -p $(OBJDIR)
		$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/sesd:	$(OBJDIR)/sesd.o $(OBJDIR)/unixpty.o $(OBJDIR)/io.o \
		$(OBJDIR)/err.o $(OBJDIR)/misc.o
		@$(MKDIR) -p $(BINDIR)
		$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
		rm -f core

$(BINDIR)/ses:	$(OBJDIR)/ses.o $(OBJDIR)/unixpty.o $(OBJDIR)/io.o \
		$(OBJDIR)/err.o $(OBJDIR)/misc.o
		@$(MKDIR) -p $(BINDIR)
		$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
		rm -f core

$(BINDIR)/%:	$(SRCDIR)/%.c
		@$(MKDIR) -p $(BINDIR)
		$(CC) $(CFLAGS) $^ -o $@

clean:
		rm -f $(BINDIR)/* $(OBJDIR)/*.o core /tmp/sesdebug*

pkg:		clean
		tar cf - . | compress > ../ses.tar.Z
		
%:
		@echo "Invoking std_$@ rule:"
		@$(MAKE) ORIGINAL_TARGET=$@ std_$@
