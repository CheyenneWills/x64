
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
/    zysmm- get more memory
/
/    Parameters:
/        None
/    Returns:
/        XR - number of addtional words obtained
/    Exits:
/        None
*/

#include "port.h"
#include <string.h>
#include <stdint.h>
int
zysmm(void)
{
    long n;

    SET_XR(0); /* Assume allocation will fail */

    /*
     * If not already at maximum allocation, try to get more memory.
     */
    if (topmem + memincb > maxmem) {
        n = moremem(memincb);
        topmem += n;              /*  adjust current top address */

        SET_XR(n / sizeof(word)); /*  set # of words obtained */
    } else {
        topmem += memincb;
        SET_XR(memincb / sizeof(word));
    }
    return NORMAL_RETURN;
}

/*
 * moremem(n) - Attempt to fetch n bytes more memory.
 *        Returns number of bytes actually obtained.
 *
 *    Returns the maximum amount <= n.
 *
 * Use realloc to expand the memory region relocating
 * any of minimal's pointers if necessary via relcr and
 * reloc.
 */
long
moremem(long n)
{
    uintptr_t curralloc;
    void *newalloc;
    long curr_dnam_offset = GET_MIN_VALUE(dnamb, char *) - basemem;
    curralloc = (uintptr_t)basemem;
    word adjusts[15];
    word saveregs[10];
    memset(adjusts, 0, sizeof(adjusts));

    n &= -(long)sizeof(word);
    newalloc = realloc(basemem, currallocsize + n);

    topmem += (uintptr_t)newalloc - curralloc;
    maxmem += (uintptr_t)newalloc - curralloc;
    basemem = newalloc;

    if ((uintptr_t)newalloc != curralloc) {
        if ((size_t)newalloc <= (20 * maxsize)) {
            wrterr("internal system error -- SYSMM");
            exit(1);
        }
        /* Save regs */
        saveregs[0] = reg_wa;
        saveregs[1] = reg_wb;
        saveregs[2] = reg_wc;
        saveregs[3] = reg_xr;
        saveregs[4] = reg_xl;
        saveregs[5] = reg_ia;
        saveregs[6] = reg_cp;
        saveregs[7] = (word)compsp;
        saveregs[8] = (word)reg_pc;
        SET_WA(&s_aaa);
        SET_WB(&c_aaa);
        SET_WC(&g_aaa);
        SET_XR(newalloc);
        SET_CP(newalloc + curr_dnam_offset);
        SET_XL(adjusts);
        MINIMAL(minimal_relcr);
        MINIMAL(minimal_reloc);

        /* Relocate any addresses in stack */
        SET_WB(compsp + sizeof(word));
        SET_WA(highsp);
        MINIMAL(minimal_relaj);

        /* Handle any relocations within the saved registers */
        SET_WB(&saveregs[0]);
        SET_WA(&saveregs[8]);
        MINIMAL(minimal_relaj);
        reg_wa = saveregs[0];
        reg_wb = saveregs[1];
        reg_wc = saveregs[2];
        reg_xr = saveregs[3];
        reg_xl = saveregs[4];
        reg_ia = saveregs[5];
        reg_cp = saveregs[6];
        compsp = (void *)saveregs[7];
        reg_pc = saveregs[8];
    }
    currallocsize += n;
    return n;
}

/* Initialize memory */
void
sysmm_init(void)
{

    /* Allocate the stack */
    lowsp = calloc(1, stacksiz);

    /* Ensure minimum memory allocation is large enough */
    if (databts < 15 * 1024) {
        databts = 15 * 1024;
    }

    databts -= memincb;
    basemem = NULL;
    /* We need to trick the C runtime so that the address of the basemem
     * is greater than maxsize (see spitbol's garbage collection).
     * Keep asking the runtime for a larger chunk of memory until the allocation
     * is moved above the maxsize.
     */
    while ((uintptr_t)basemem <= (uintptr_t)maxsize) {
        free(basemem);
        databts += memincb;
        basemem = calloc(1, databts);
        if (basemem == NULL) {
            wrterr("Insufficient memory to initialize SPITBOL environment");
            exit(1);
        }
    }
    currallocsize = databts;
    if (databts < memincb) {
        topmem = basemem + databts;
    } else {
        topmem = basemem + memincb;
    }
    maxmem = basemem + databts;
}
