
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
 * This module contains the main function that gets control when
 * the spitbol compiler starts execution.  Responsibilities of
 * this function:
 *
 * Save argc and argv parameters in global storage.
 *
 * Process command line arguments, set up input files, output file,
 * initial memory allocation, and transfer control to compiler
 */

#define GLOBALS /* global variables will be defined in this module */
#include "port.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

int
main(int argc, char **argv)
{
    struct minimal_regs initial_regs, *minregs = &initial_regs;
    memset(&initial_regs, 0, sizeof(initial_regs));

    /*
     * Save command line parameters in global storage, in case they are needed
     * later.
     */
    gblargc = argc;
    gblargv = argv;
    /*
     * Initialize buffers
     */
    stdioinit();
    ttyinit();
    /*
     * Make sure sysdc gets to output stuff at least once.
     */
    dcdone = 0;

    /*
     *  Process command line arguments
     */
    inpptr = getargs(argc, argv);

    if (inpptr) {
        sfn = *inpptr; /* pointer to first file name */
    } else {
        wrterr("");
        prompt();
    }

    /*
     * Switch to proper input file.
     */
    swcinp(inpcnt, inpptr);

    switch (getsave(minregs, getrdfd())) {
        case 1:                     /* save file loaded */
            wrterr("SPITBOL restart is currently broken");
            inpcnt = 0;             /* v1.02 no more cmd line files */
            swcinp(inpcnt, inpptr); /* v1.01 */
            restart(NULL, lowsp);

        case 0:                     /* not a save file */
            break;

        case -1:                    /* error loading save file */
            exit(1);
    }

    /*
     * Setup output and issue brag message
     */
    setout();

    /* Initialize memory */

    sysmm_init();

    /*
     * All compiler registers are initially zero, except for XL and XR
     * which are set to top and bottom of heap.
     */
    SET_CP(0);
    SET_IA(0);

    SET_WA(compsp);
    long mxint = INT64_MAX;

    /*    reg_wb = mxint; */
    SET_WB(mxint);
    SET_WC(0);
    SET_XR(basemem);
    SET_XL(topmem - sizeof(word));

    /*
     * Startup compiler.
     */
    startup(minregs, minimal_start);

    /*
     * Never returns. exit is via exit().
     */
}

/*
 * wrterr( s )
 * Write message to standard error, and append end-of-line.
 */
void
wrterr(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
	fprintf(stderr,"\n");
    va_end(args);
}

/*
 * Setup output file.
 * Issue brag message if approriate
 *
 * This rather clumsy routine was placed here because of sequencing
 * concerns -- it can't be called with a save file until spitflag
 * has been reloaded.
 */
void
setout(void)
{
    /*
     *  Brag prior to calling swcoup
     */

    /*
     * Switch to proper output file.
     */
    swcoup(outptr);

    /*
     * Determine if standard output is a tty or not, and if it is be sure
     * to inform compiler and turn off header.
     */
    spitflag &= ~PRTICH;
    if (testty(getprfd()) == 0) {
        lnsppage = 0;
        spitflag |= (PRTICH | NOHEDR);
    }
}
