
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
/   arg2scb( req, argc, argv, scptr, maxs )
/
/   arg2scb() makes a copy of the req-th argument in the argv array.
/   The copy is appended to the string in the SCBLK provided.
/
/   Parameters:
/    req    number of argument to copy
/    argc    number of arguments
/    argv    pointer to array of pointers to strings (arguments)
/    scptr    pointer to SCBLK to receive copy of argument
/    maxs    maximum number of characters to append.
/   Returns:
/    Length of argument copied or -1 if req is out of range.
/   Side Effects:
/    Modifies contents of passed SCBLK (scptr).
/    SCBLK length field is incremented.
*/

#include "port.h"
extern char *lowspminx;
static int
arg2scb(int req, int argc, char *argv[], struct scblk *scptr, int maxs)
{
    word i;
    char *argcp, *scbcp;

    if (req < 0 || req >= argc) {
        return -1;
    }

    argcp = argv[req];
    scbcp = scptr->schar + scptr->sclen;
    for (i = 0; i < maxs && ((*scbcp++ = *argcp++) != 0); i++)
        ;
    scptr->sclen += i;
    return i;
}

/*
/    zyshs - host specific functions
/
/    zyshs is the catch-all function in the interface.  Any actions that
/    are host specific should be placed here.
/
/    zyshs determines what function to preformed by examining the value
/    of argument 1.  Current functions:
/
/    HOST()
/        returns the host string identifying the host environment
/
/    HOST( 0 )
/        returns -u argument from command line
/
/    HOST( 1, "command" )
/        executes 2nd argument as a Unix command
/
/    HOST( 2, n )
/        returns command line argument "n"
/
/    HOST( 3 )
/        returns the command count
/
/    HOST( 4, "v" )
/        returns the value of environment variable "v"
/
/    Other HOST functions may be provided by system specific modules.
/
/    Parameters:
/        WA - argument 1
/        XL - argument 2
/        XR - argument 3
/        WB - argument 4
/        WC - argument 5
/    Returns:
/        See exits
/    Exits:
/        1 - erroneous argument
/        2 - execution error
/        3 - pointer to SCBLK or 0 in XL
/        4 - return NULL string
/        5 - return result in XR
/        6 - cause statement failure
/        7 - return string in XL, length in WA (may be 0)
/        8 - return copy of result in XR
*/

/*
 *  checkstr - check if scblk is a valid string.  Returns 1 if so, else 0.
 */
int
checkstr(struct scblk *scp)
{
    return scp != NULL && scp->scget == TYPE_SCL &&
           scp->sclen < tscblk_length;
}

/*  check2str - check first two argument strings in XL, XR.
 *
 * Returns 1 if both OK, else 0.
 */
int
check2str(void)
{
    return checkstr(XL(struct scblk *)) && checkstr(XR(struct scblk *));
}

/*
 *  savestr - convert an scblk to a valid C string.  Returns pointer to
 *        start of string, or 0 if fail.  The char replaced by the
 *        '\0' terminator is returned in *cp.
 */
char *
savestr(struct scblk *scp, char *cp)
{
    *cp = scp->schar[scp->sclen];
    scp->schar[scp->sclen] = '\0';
    return scp->schar;
}

/*
 *  save2str - convert first two argument strings in XL, XR.
 */
void
save2str(char **s1p, char **s2p)
{
    *s1p = savestr(XL(struct scblk *), &savexl);
    *s2p = savestr(XR(struct scblk *), &savexr);
}

/*
 *  getstring - verify and convert an scblk to a valid C string.
 *   Returns pointer to start of string, or 0 if fail.  The char
 *   replaced by the '\0' terminator is returned in *cp.
 */
char *
getstring(struct scblk *scp, char *cp)
{
    return checkstr(scp) ? savestr(scp, cp) : NULL;
}

/*
 *  restorestring - restore an scblk after a getstring call.
 *
 *  when making multiple getstring calls, call restorestring in the reverse
 *  order from getstring, in case two arguments point to the same source string.
 */
void
restorestring(struct scblk *scp, word c)
{
    if (scp) {
        scp->schar[scp->sclen] = c;
    }
}

/*
 *  restore2str - restore two argument strings in XL, XR.
 */
void
restore2str()
{
    restorestring(XR(struct scblk *), savexr);
    restorestring(XL(struct scblk *), savexl);
}

/*
 *  getint - fetch an integer from either an icblk, rcblk or non-null scblk.
 *
 *  returns 1 if successful, 0 if failed.
 */
int
getint(struct icblk *icp, long *pword)
{
    char *p, c;
    struct scblk *scp;
    word i;
    long result;
    int sign;

    sign = 1;
    if (icp->icget == TYPE_ICL) {
        result = icp->icval;
    }
#ifdef REAL_ARITH
    else if (icp->icget == TYPE_RCL) {
# if sparc
        union {
            mword rcvals[2];
            double rcval;
        } val;
        val.rcvals[0] = ((struct rcblk *)icp)->rcvals[0];
        val.rcvals[1] = ((struct rcblk *)icp)->rcvals[1];
        result = (long)val.rcval;
# else
        result = (long)(((struct rcblk *)icp)->rcval);
# endif
    }
#endif
    else {
        scp = (struct scblk *)icp;
        if (!checkstr(scp)) {
            return 0;
        }
        i = scp->sclen;
        p = scp->schar;
        result = (long)0;
        while (i && *p == ' ') { /* remove leading blanks */
            p++;
            i--;
        }
        if (i && (*p == '+' || *p == '-')) { /* process optional sign char */
            if (*p++ == '-') {
                sign = -1;
            }
            i--;
        }
        while (i--) {
            c = *p++;
            if (c < '0' || c > '9') { /* not handling trailing blanks */
                return 0;
            }
            result = result * 10 + (c - '0');
        }
    }
    *pword = sign * result;
    return 1;
}

int
zyshs()
{
    word retval;
    long val;
    struct icblk *icp = WA(struct icblk *);
    struct scblk *scp;

    /*
       /   if argument one is null...
     */
    scp = WA(struct scblk *);
    if (scp->scget == TYPE_SCL && !scp->sclen) {
        gethost(ptscblk, tscblk_length);
        if (ptscblk->sclen == 0) {
            return EXIT_4;
        }
        SET_XL(ptscblk);
        return EXIT_3;
    }

    /*
       /   If argument one is an integer ...
     */
    if (getint(icp, &val)) {
        switch ((int)val) {
                /*
                       / HOST( -1, n ) returns internal parameter n
                     */
            case -1:
                icp = XL(struct icblk *);
                if (getint(icp, &val)) {
                    pticblk->icget = TYPE_ICL;
                    SET_XR(pticblk);
                    switch ((int)val) {
                        case 0:
                            pticblk->icval = memincb;
                            return EXIT_8;
                        case 1:
                            pticblk->icval = databts;
                            return EXIT_8;
                        case 2:
                            pticblk->icval = (long)basemem;
                            return EXIT_8;
                        case 3:
                            pticblk->icval = (long)topmem;
                            return EXIT_8;
                        case 4:
                            pticblk->icval = stacksiz - 400; /* safety margin */
                            return EXIT_8;
                        case 5:                              /* stack in use */
                            pticblk->icval = stacksiz - (XS(long) - (long)lowsp);
                            return EXIT_8;
                        case 6:
                            pticblk->icval = sizeof(long);
                            return EXIT_8;
                        case 7:
                            pticblk->icval = stacksiz - ((char *)lowspminx - (char *)lowsp);
                            return EXIT_8;
                        default:
                            return EXIT_1;
                    }
                } else {
                    return EXIT_1;
                }

                /*
                       /  HOST( 0 ) returns the -u command line option argument
                     */
            case 0:
                if (uarg) {
                    cpys2sc(uarg, ptscblk, tscblk_length);
                    SET_XL(ptscblk);
                    return EXIT_3;
                } else if ((val = cmdcnt) != 0) {
                    ptscblk->sclen = 0;
                    while (ptscblk->sclen < tscblk_length - 2 &&
                           arg2scb((int)val++, gblargc, gblargv, ptscblk,
                                   tscblk_length - ptscblk->sclen) > 0) {
                        ptscblk->schar[ptscblk->sclen++] = ' ';
                    }
                    if (ptscblk->sclen) {
                        --ptscblk->sclen;
                    }
                    SET_XL(ptscblk);
                    return EXIT_3;
                } else {
                    return EXIT_4;
                }
                /*
                       / HOST( 1, "command", "path" ) executes "command" using "path"
                     */
            case 1: {
                char *cmd, *path;

                if (!check2str()) {
                    return EXIT_1;
                }
                save2str(&cmd, &path);
                save0(); /* made sure fd 0 OK */
                pticblk->icval = dosys(cmd, path);

                pticblk->icget = TYPE_ICL;
                restore2str();
                restore0();
                if (pticblk->icval < 0) {
                    return EXIT_6;
                }
                SET_XR(pticblk);
                return EXIT_8;
            }

                /*
                       / HOST( 2, n ) returns command line argument n
                     */
            case 2:
                icp = XL(struct icblk *);
                if (getint(icp, &val)) {
                    ptscblk->sclen = 0;
                    retval = arg2scb((int)val, gblargc, gblargv, ptscblk,
                                     tscblk_length);
                    if (retval < 0) {
                        return EXIT_6;
                    }
                    if (retval == 0) {
                        return EXIT_1;
                    }
                    SET_XL(ptscblk);
                    return EXIT_3;
                } else {
                    return EXIT_1;
                }

                /*
                       /  HOST( 3 ) returns the command count
                     */
            case 3:
                if (cmdcnt) {
                    pticblk->icget = TYPE_ICL;
                    pticblk->icval = cmdcnt;
                    SET_XR(pticblk);
                    return EXIT_8;
                } else {
                    return EXIT_6;
                }

                /*
                       / HOST( 4, "env-var" ) returns the value of "env-var" from
                       /        the environment.
                     */
            case 4:
                scp = XL(struct scblk *);
                if (scp->scget == TYPE_SCL) {
                    if (scp->sclen == 0) {
                        return EXIT_1;
                    }
                    if (rdenv(scp, ptscblk) < 0) {
                        return EXIT_6;
                    }
                    SET_XL(ptscblk);
                    return EXIT_3;
                } else {
                    return EXIT_1;
                }
        } /* end switch */

        /*
           / Any other integer value is processed by the system-specific functions
         */

        /*
           /   Here if first argument wasn't an integer or was an illegal value.
         */
    }
    return EXIT_1;
}
