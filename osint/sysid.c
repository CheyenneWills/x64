
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

int
zysid()
{
    time_t now;
    char *s;

    struct utsname unamebuf;
    uname(&unamebuf);

    id1blk->schar[0] = 0;
    id1blk->sclen = 0;

    now = time(NULL);
    id2blk->schar[0] = 0;
    strcpy(id2blk->schar, unamebuf.machine);
    strcat(id2blk->schar, "  ");
    s = ctime(&now);
    strcat(id2blk->schar, s);
    id2blk->sclen = strlen(id2blk->schar);

    SET_XR(id1blk);
    SET_XL(id2blk);
    return NORMAL_RETURN;
}
