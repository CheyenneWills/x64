
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
 * Memory allocations
 *
 * There are 2 memory allocations:
 *  o minimal stack   - Size is static
 *  o minimal heap    - Size is dynamic
 *
 * The size of the minimal stack is defined via a build time size that can be
 * overridden via the -s# command line option.
 *
 * The minimal heap size, increment and maximal size are defined by build time
 * settings and can be overridden via the -d# (for absolute max), -i#[,#] for
 * initial size and increment size.
 *
 * Due to the method used for garbage collection, the minimal heap has a
 * restriction that it's starting address must be greater than then largest size
 * of a spitbol object.  In order to achieve this, the initial memory allocation
 * may be adjusted to force the C memory allocation to allocate from high memory
 * instead of lower memory. (This is sensitive to C's runtime malloc/realloc
 * routines)
 *
 *  Minimal stack (static size, not relocated)
 *  +--------------------+ <- highsp (stack goes down) <- initial compsp
 *  |           ^        |
 *  |           |        |
 *  / stacksiz bytes     /
 *  |           |        | }-- lowspminx (lowest used stack)
 *  |           |        |
 *  |---------- | -------| <-- resultant lowspmin
 *  | 400 guard v bytes  |
 *  +--------------------+ <- lowsp
 *
 *  Minimal heap (dynamic size, relocatable)
 *  +----------------------+ <-- maxmem (will expand until heap_alloced_size
 *  |                    ^ |  ^          reaches heap_max)
 *  |                    | |  |
 *  / heap_alloced_size -| /  } - available for heap expansion without realloc
 *  |                    | |  v
 *  |--------------------|-| <- topmem (will grow to maxmem in heap_inc chunks)
 *  / heap_inc           | /  ^
 *  |--------------------|-|  } - Active minimal heap storage
 *  / heap_initial_size  v /  v
 *  +----------------------+ <- basemem  (address must be creater than maxlen)
 *
 */

/*
 * zysmm- get more memory
 *
 * Parameters:
 *    None
 * Returns:
 *    XR - number of addtional words obtained
 * Exits:
 *    None
 */

#include "port.h"
#include <string.h>
#include <stdint.h>
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

static long
moremem(struct minimal_regs *oregs, long n)
{
    uintptr_t curralloc;
    struct minimal_regs save_regs, *minregs = &save_regs;
    void *newalloc;
    long curr_dnam_offset = GET_MIN_VALUE(dnamb, char *) - basemem;

    curralloc = (uintptr_t)basemem;
    word adjusts[15];
    memset(adjusts, 0, sizeof(adjusts));

    n &= -(long)sizeof(word);
    if (heap_max != MAXUINT && heap_alloced_size + n > heap_max) {
        return -1; /* Exceeds hard limit */
    }
    newalloc = realloc(basemem, heap_alloced_size + n);

    topmem += (uintptr_t)newalloc - curralloc;
    maxmem += (uintptr_t)newalloc - curralloc;
    basemem = newalloc;

    if ((uintptr_t)newalloc != curralloc) {
        if ((size_t)newalloc <= (20 * maxsize)) {
            wrterr("internal system error -- SYSMM");
            exit(1);
        }
        memcpy(minregs, oregs, sizeof(*minregs));
        SET_XR(newalloc);
        SET_CP(newalloc + curr_dnam_offset);
        SET_XL(adjusts);
        SET_WC(&g_aaa);
        SET_WB(&c_aaa);
        SET_WA(&s_aaa);
        minimal(minregs, minimal_relcr);

        memcpy(&save_regs, oregs, sizeof(*minregs));
        SET_XL(adjusts);
        minimal(minregs, minimal_reloc);

        /* Relocate any addresses in stack */
        memcpy(minregs, oregs, sizeof(*minregs));
        SET_XL(adjusts);

        SET_WB(compsp + sizeof(word));
        SET_WA(highsp);
        minimal(minregs, minimal_relaj);

        /* Handle any relocations within the saved registers */
        memcpy(minregs, oregs, sizeof(*minregs));
        SET_XL(adjusts);
        SET_WB(oregs);
        SET_WA((uintptr_t)oregs + sizeof(*oregs));
        minimal(minregs, minimal_relaj);
    }
    heap_alloced_size += n;
    return n;
}

int
zysmm(struct minimal_regs *minregs)
{
    long n;

    SET_XR(0); /* Assume allocation will fail */

    /*
     * If not already at maximum allocation, try to get more memory.
     */
    if (topmem + heap_inc > maxmem) {
        n = moremem(minregs, heap_inc);
        if (n < 0) {
            return NORMAL_RETURN;
        }
        topmem += n;              /*  adjust current top address */

        SET_XR(n / sizeof(word)); /*  set # of words obtained */
    } else {
        topmem += heap_inc;
        SET_XR(heap_inc / sizeof(word));
    }
    return NORMAL_RETURN;
}

/* Initialize memory */
void
sysmm_init(void)
{

    size_t memalloc_size;

    /* Allocate the stack */
    lowsp = calloc(1, stacksiz);
    lowspmin = lowsp + 400;
    compsp = lowsp + stacksiz;
    highsp = compsp;
    lowspminx = highsp;

    if (heap_max != MAXUINT && heap_initial_size > heap_max) {
        heap_initial_size = heap_max;
    }

    memalloc_size = heap_initial_size - heap_inc;
    basemem = NULL;
    /* We need to trick the C runtime so that the address of the basemem
     * is greater than maxsize (see spitbol's garbage collection).
     * Keep asking the runtime for a larger chunk of memory until the allocation
     * is moved above the maxsize.
     */
    while ((uintptr_t)basemem <= (uintptr_t)maxsize) {
        free(basemem);
        memalloc_size += heap_inc;
        basemem = calloc(1, memalloc_size);
        if (basemem == NULL) {
            wrterr("Insufficient memory to initialize SPITBOL environment");
            exit(1);
        }
    }
    heap_alloced_size = memalloc_size;
    if (heap_max != MAXUINT && heap_max < heap_alloced_size) {
        heap_max = heap_alloced_size;
    }
    topmem = basemem + memalloc_size;
    maxmem = basemem + heap_alloced_size;
}
