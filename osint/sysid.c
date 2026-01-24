
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
/    File:  SYSID.C        Version:  01.02
/    ---------------------------------------
/
/    Contents:    Function zysid
*/

/*
/    zysid - identify system
/
/    zysid returns two strings identifying the Spitbol system.
/
/    Parameters:
/        None
/    Returns:
/        XR - pointer to SCBLK containing suffix to Spitbol header
/        XL - pointer to SCBLK containing 2nd header line
/    Exits:
/        None
*/

#include "port.h"
#include <string.h>
#include <time.h>
#include <sys/utsname.h>

/*
/   define actual headers elsewhere to overcome problems in initializing
/   the two SCBLKs.  Use id2blk instead of tscblk because tscblk may
/    be active with an error message when zysid is called.
*/
#include "gitversion.h"
int
zysid(struct minimal_regs *minregs)
{
    time_t now;
    char *s;
	char timebuf[26];
	int timel;

    struct utsname unamebuf;
    uname(&unamebuf);

    id1blk->schar[0] = 0;
    id1blk->sclen = 0;

    now = time(NULL);
    id2blk->schar[0] = 0;
    strcpy(id2blk->schar, unamebuf.machine);
    strcat(id2blk->schar, "  build:");
    s = asctime_r(localtime(&now), timebuf);
	timel = strlen(s);
	s[timel-1] = '\0';
    strcat(id2blk->schar, s);
	strcat(id2blk->schar," ");
	strcat(id2blk->schar,source_version);
    id2blk->sclen = strlen(id2blk->schar);

    SET_XR(id1blk);
    SET_XL(id2blk);
    return NORMAL_RETURN;
}
