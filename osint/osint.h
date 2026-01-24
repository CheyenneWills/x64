
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
   This header file defines the interface between the Macro SPITBOL compiler
   written in assembly langauge and the OS interface written in C.
   Communication between the two is handled via a set of global variables
   defined as externals below.
*/

/*
 *  Set up externals for all the compiler's registers.
 */
extern void *lowspmin, *lowspminx;
extern uword minimal_id;

/*
 *  Macros to deal with converting pointers within the Minimal heap
 *  to pointers that the C code can deal with.
 *  These macros were needed for x86, due to NEAR/FAR distinction, which
 *  is no longer relevant, so that the macros should be eliminated when
 *  convenient, to improve readability.
 */
#define MP_OFF(cp, type) ((type)cp)

/*
 *  Macros to fetch a value of appropriate type from a compiler register
 */

struct minimal_regs {
    long reg_ia;
    double reg_ra;
    uword reg_xr;
    uword reg_xl;
    uword reg_wa;
    uword reg_wb;
    uword reg_wc;
    uword reg_cp;
    uword reg_pc;
    uword reg_xs;
};

#define CP(type) ((type)(minregs->reg_cp))
#define IA(type) ((type)(minregs->reg_ia))
#define WA(type) ((type)(minregs->reg_wa))
#define WB(type) ((type)(minregs->reg_wb))
#define WC(type) ((type)(minregs->reg_wc))
#define XR(type) ((type)(minregs->reg_xr))
#define XL(type) ((type)(minregs->reg_xl))
#define PC(type) ((type)(minregs->reg_pc))
#define XS(type) ((type)(minregs->reg_xs))
#define RA(type) ((type)(minregs->reg_ra))

/*
 *  Macros to set a value of appropriate type into a compiler register.
 */
#define SET_CP(val) ((minregs->reg_cp) = (word)(val))
#define SET_IA(val) ((minregs->reg_ia) = (val))
#define SET_WA(val) ((minregs->reg_wa) = (word)(val))
#define SET_WB(val) ((minregs->reg_wb) = (word)(val))
#define SET_WC(val) ((minregs->reg_wc) = (word)(val))
#define SET_XR(val) ((minregs->reg_xr) = (word)(val))
#define SET_XL(val) ((minregs->reg_xl) = (word)(val))
#define SET_PC(val) ((minregs->reg_pc) = (word)(val))
#define SET_XS(val) ((minregs->reg_xs) = (word)(val))
#define SET_RA(val) ((minregs->reg_ra) = (double)(val))

/*
 *  Return values to take exit N from interface
 */
#define EXIT_1 1
#define EXIT_2 2
#define EXIT_3 3
#define EXIT_4 4
#define EXIT_5 5
#define EXIT_6 6
#define EXIT_7 7
#define EXIT_8 8
#define EXIT_9 9

/*
 *   Return value to do a normal return from interface.
 */
#define NORMAL_RETURN 0

/*
 *   Function to call into MINIMAL code.
 *   The argument is an ordinal number defined below.
 */
extern void minimal(struct minimal_regs *minregs, long id);

/*
 *    Ordinals for MINIMAL calls from C.
 *
 *   The order of entries here must correspond to the order of
 *   table entries in the INTER assembly language module.
 */
enum CALLS {
    minimal_relaj,
    minimal_relcr,
    minimal_reloc,
    minimal_alloc,
    minimal_alocs,
    minimal_alost,
    minimal_blkln,
    minimal_insta,
    minimal_rstrt,
    minimal_start,
    minimal_filnm,
    minimal_dtype,
    minimal_enevs,
    minimal_engts
};

/*
 *   Function and macro to get/set value from/to MINIMAL dataspace.
 *   The argument is an ordinal number defined below.
 *      GET_DATA_OFFSET returns the address of a Minimal data value.
 *      GET_CODE_OFFSET returns the address of a Minimal routine.
 *   GET_MIN_VALUE returns the contents of an item of Minimal data.
 *   SET_MIN_VALUE sets the contents of an item of Minimal data.
 */
#define GET_CODE_OFFSET(vn, type) ((type)vn)
#define GET_DATA_OFFSET(vn, type) ((type)&vn)
#define GET_MIN_VALUE(vn, type) ((type)vn)
#define SET_MIN_VALUE(vn, val, type) (*(type *)&vn = (type)(val))

/*
 *  Names for accessing MINIMAL data values via GET_DATA_OFFSET macro.
 */
extern word headv, mxlen, dnamb, dnamp, stbas, state,
  polct, typet, flprt, flptr, gtcef, hshtb, pmhbs, r_fcb,
  c_aaa, c_yyy, g_aaa, w_yyy, kvcom,
  stmcs, stmct;

/*
 *  Names for accessing MINIMAL code values via GET_CODE_OFFSET macro.
 */
extern void b_efc();
extern void b_icl();
extern void b_rcl();
extern void b_scl();
extern void b_vct();
extern void b_xnt();
extern void b_xrt();
extern void dffnc();
extern void s_aaa();
extern void s_yyy();

/* Some shorthand notations */
#define TYPE_EFC GET_CODE_OFFSET(b_efc, word)
#define TYPE_ICL GET_CODE_OFFSET(b_icl, word)
#define TYPE_SCL GET_CODE_OFFSET(b_scl, word)
#define TYPE_VCT GET_CODE_OFFSET(b_vct, word)
#define TYPE_XNT GET_CODE_OFFSET(b_xnt, word)
#define TYPE_XRT GET_CODE_OFFSET(b_xrt, word)
#define TYPE_RCL GET_CODE_OFFSET(b_rcl, word)
