/************************************************************************
 *
 *indx#	ptytest.c - Software for testing pty i/o routines
 *@HDR@	$Id$
 *@HDR@
 *@HDR@	Copyright (c) 2026 Christopher Caldwell (Christopher.M.Caldwell0@gmail.com)
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
 *doc#	Software for testing pty i/o routines
 ************************************************************************/
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

extern int grantpt( int fd );
extern int unlockpt( int fd );
extern char *ptsname(int fd);
extern int ptsname_r(int fd, char buf[], size_t size);

int main() {
    // 1. Open the master pseudoterminal
    int masterfd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
    if (masterfd < 0) {
        perror("open /dev/ptmx");
        exit(1);
    }

    // 2. Grant access to the slave (changes owner/mode)
    if (grantpt(masterfd) < 0) {
        perror("grantpt");
        exit(1);
    }

    // 3. Unlock the slave (required on some systems)
    if (unlockpt(masterfd) < 0) {
        perror("unlockpt");
        exit(1);
    }

    // 4. Get the name of the slave device
    char slavepath[64];
    if (ptsname_r(masterfd, slavepath, sizeof(slavepath)) < 0) {
        perror("ptsname_r");
        exit(1);
    }

    printf("Slave device: %s\n", slavepath);

    // ... perform I/O ...

    close(masterfd);
    return 0;
}   
