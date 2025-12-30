
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
/   gethost( scptr, maxlen )
/
/   gethost() reads the first line from the host file into the passed SCBLK.
/
/   Parameters:
/    scptr    pointer to SCBLK to receive host string
/    maxlen    max length of string area in SCBLK
/   Returns:
/    Nothing.
/   Side Effects:
/    Modifies contents of passed SCBLK (scptr).
*/

#include "port.h"

#include <fcntl.h>
#include <sys/utsname.h>

void
gethost(struct scblk *scptr, word maxlen)

{
    struct scblk *pheadv = GET_DATA_OFFSET(headv, struct scblk *);
    int cnt = 0;
    word fd;

    if ((fd = spit_open(HOST_FILE, O_RDONLY, IO_PRIVATE | IO_DENY_WRITE,
                        IO_OPEN_IF_EXISTS)) >= 0) {
        cnt = read(fd, scptr->schar, maxlen);
        close(fd);
    }

    if (cnt > 0 && scptr->schar[cnt - 1] == EOL) {
        scptr->schar[--cnt] = 0;
    }

    if (cnt == 0) {
        /* Could not read spithost file.  Construct string instead */
        struct utsname unamebuf;
        uname(&unamebuf);
        scptr->schar[0] = 0;
        strcpy(scptr->schar, unamebuf.machine);
        strcat(scptr->schar, ":");
        strcat(scptr->schar, unamebuf.sysname);
        strcat(scptr->schar, " ");
        strcat(scptr->schar, unamebuf.release);
        strcat(scptr->schar, ":Macro SPITBOL ");
        strncat(scptr->schar, pheadv->schar, pheadv->sclen);
        strncat(scptr->schar, id1blk->schar, id1blk->sclen);
        strcat(scptr->schar, " #");
        cnt = strlen(scptr->schar);
    }

    scptr->sclen = cnt;
}
