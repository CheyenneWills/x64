
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
/   This header file defines structures used by the Macro SPITBOL compiler
/   that are passed into the OS interface.

/   First, define the C type word to be the same size as a word used
/   by the Macro SPITBOL compiler.  The type of a word is a signed
/   integer for now.
*/

#ifndef BLOCKS_H
#define BLOCKS_H
enum blockcodes {
    bl_ar = 0,  /*           arblk     array */
    bl_cd = 1,  /* bl_ar+1   cdblk     code */
    bl_ex = 2,  /* bl_cd+1   exblk     expression */
    bl_ic = 3,  /* bl_ex+1   icblk     integer */
    bl_nm = 4,  /* bl_ic+1   nmblk     name */
    bl_p0 = 5,  /* bl_nm+1   p0blk     pattern */
    bl_p1 = 6,  /* bl_p0+1   p1blk     pattern */
    bl_p2 = 7,  /* bl_p1+1   p2blk     pattern */
    bl_rc = 8,  /* bl_p2+1   rcblk     real */
    bl_sc = 9,  /* bl_rc+1   scblk     string */
    bl_se = 10, /* bl_sc+1   seblk     expression */
    bl_tb = 11, /* bl_se+1   tbblk     table */
    bl_vc = 12, /* bl_tb+1   vcblk     array */
    bl_xn = 13, /* bl_vc+1   xnblk     external */
    bl_xr = 14, /* bl_xn+1   xrblk     external */
    bl_bc = 15, /* bl_xr+1   bcblk     buffer */
    bl_pd = 16, /* bl_bc+1   pdblk     program defined datatype */
    bl__d = 17, /* bl_pd+1   number of block codes for data */
    bl_tr = 17, /* bl_pd+1   trblk */
    bl_bf = 18, /* bl_tr+1   bfblk */
    bl_cc = 19, /* bl_bf+1   ccblk */
    bl_cm = 20, /* bl_cc+1   cmblk */
    bl_ct = 21, /* bl_cm+1   ctblk */
    bl_df = 22, /* bl_ct+1   dfblk */
    bl_ef = 23, /* bl_df+1   efblk */
    bl_ev = 24, /* bl_ef+1   evblk */
    bl_ff = 25, /* bl_ev+1   ffblk */
    bl_kv = 26, /* bl_ff+1   kvblk */
    bl_pf = 27, /* bl_kv+1   pfblk */
    bl_te = 28, /* bl_pf+1   teblk */
    bl__i = 0,  /* default   identification code */
    bl__t = 18, /* bl_tr+1   code for data or trace block */
    bl___ = 29, /* bl_te+1   number of block codes */
};

struct ar_bounds {
    word arlbd; /* Low bounds */
    word ardim; /* dimension */
};

struct arblk {
    word artyp;
    word idval;
    word arlen;     /* Size in bytes */
    word arofs;     /* Offset to arpro field */
    word arndm;     /* Number of dimensions */
    word ar_data[]; /* variable data */
    /*
     * ar_data is in the following format
     *  struct ar_bounds[arndm];
     *  struct scblk *ar_pro;
     *  word ar_vls[];
     */
};
static inline void *
calc_array_offset(struct arblk *array, long indexes[])
{
    long k;
    long offset = 0;
    struct ar_bounds **bounds;
    word *vls;
    struct scblk *pro;
    bounds = (struct ar_bounds **)&array->ar_data[0];
    pro = (struct scblk *)((uintptr_t)array + array->arofs);
    vls = (word *)((uintptr_t)(pro) + sizeof(uintptr_t));
    for (k = 0; k < array->arndm; k++) {
        offset += (indexes[k] - bounds[k]->arlbd);
        if (k < array->arndm - 1) {
            offset *= bounds[k + 1]->ardim;
        }
    }
    return &vls[offset];
}

/*
 *     BUFFER CONTROL BLOCK
 *
 *    A buffer control block (BCBLK) is created by the BUFFER
 *    function, and serves as an indirect control header for the
 *    buffer. It contains the number of characters currently
 *    stored in the buffer.
 */
struct bcblk {
    word bctyp;          /* type word */
    word idval;          /* identifier value */
    word bclen;          /* number of chars in use in bfblk */
    struct bfblk *bcbuf; /* pointer to bfblk */
};

struct bfblk {
    word bftyp;   /* 0;ptr to dummy routine b_bft */
    word bfalc;   /* bftyp+1;allocated size of buffer */
    char bfchr[]; /* bfalc+1;characters of string */
};

struct ccblk {
    word cctyp;   /* 0;pointer to dummy routine b_cct */
    word cclen;   /* cctyp+1;length of ccblk in bytes */
    word ccsln;   /* cclen+1;source line number */
    word ccuse;   /* ccsln+1;offset past last used word (bytes) */
    word cccod[]; /* ccuse+1;start of generated code in block */
};

/*
 * CODE BLOCK
 *
 * A code block (CDBLK) is present for every source statement.
 */

struct cdblk {
    word cdjmp;              /* ptr to routine to execute statement */
    word cdstm;              /* statement number */
    word cdsln;              /* source file line number */
    word cdlen;              /* length of CDBLK in bytes */
    union {
        struct cdblk *cdnxt; /* if failure exit is next statement */
        struct vrblk *cdlab; /* if failure exit is a simple label */
        char *cdnof;         /* no failure exit (-NOFAIL mode) */
        word cddir;          /* failure exit is complex or direct */
    } cdfal;                 /* Failure exit */
    word cdcod[];            /* executable pseudo-code */
};

struct cmblk {
    word cmidn;    /* 0;pointer to dummy routine b_cmt */
    word cmlen;    /* cmidn+1;length of cmblk in bytes */
    word cmtyp;    /* cmlen+1;type (c_xxx, see list below) */
    void *cmopn;   /* cmtyp+1;operand pointer (see below) */
    void *cmvls[]; /* cmopn+1;operand value pointers (see below) */
#define cmrop (cmlvs[0]) /* cmvls;right (only) operator operand */
#define cmlop (cmlvs[1]) /* cmvls+1;left operator operand */
#define cmar1 (cmlvs[1]) /* cmvls+1;array subscript pointers */
};
enum cmtyp {
    c_arr = 0,       /* 0;array reference */
    c_fnc = 1,       /* c_arr+1;function call */
    c_def = 2,       /* c_fnc+1;deferred expression (unary *) */
    c_ind = 3,       /* c_def+1;indirection (unary _) */
    c_key = 4,       /* c_ind+1;keyword reference (unary ampersand) */
    c_ubo = 5,       /* c_key+1;undefined binary operator */
    c_uuo = 6,       /* c_ubo+1;undefined unary operator */
    c_uo_ = 7,       /* c_uuo+1;test value (=c_uuo+1=c_ubo+2) */
    c__nm = 7,       /* c_uuo+1;number of codes for name operands */
    c_bvl = 7,       /* c_uuo+1;binary op with value operands */
    c_uvl = 8,       /* c_bvl+1;unary operator with value operand */
    c_alt = 9,       /* c_uvl+1;alternation (binary bar) */
    c_cnc = 10,      /* c_alt+1;concatenation */
    c_cnp = 11,      /* c_cnc+1;concatenation, not pattern match */
    c_unm = 12,      /* c_cnp+1;unary op with name operand */
    c_bvn = 13,      /* c_unm+1;binary op (operands by value, name) */
    c_ass = 14,      /* c_bvn+1;assignment */
    c_int = 15,      /* c_ass+1;interrogation */
    c_neg = 16,      /* c_int+1;negation (unary not) */
    c_sel = 17,      /* c_neg+1;selection */
    c_pmt = 18,      /* c_sel+1;pattern match */
    c_pr_ = 19,      /* c_bvn;last preevaluable code */
    c__nv = 19,      /* c_pmt+1;number of different cmblk types */
};
struct cttyp {
    word cttyp;      /* 0;pointer to dummy routine b_ctt */
    word ctchs[256]; /* cttyp+1;start of character table words */
};

struct dfblk {
    word fcode;
    word fargs;
    word dflen;            /* fargs+1;length of dfblk in bytes */
    word dfpdl;            /* dflen+1;length of corresponding pdblk */
    struct scblk *dfnam;   /* dfpdl+1;pointer to scblk for datatype name */
    struct vrblk *dffld[]; /* dfnam+1;start of vrblk ptrs for field names */
};
struct dvblk {
    word dvopn;            /* 0;entry address (ptr to o_xxx) */
    word dvtyp;            /* dvopn+1;type code (c_xxx, see cmblk) */
    word dvlpr;            /* dvtyp+1;left precedence (llxxx, see below) */
    word dvrpr;            /* dvlpr+1;right precedence (rrxxx, see below) */
};
#define rrass 10  /* right     equal */
#define llass 00  /* left      equal */
#define rrpmt 20  /* right     question mark */
#define llpmt 30  /* left      question mark */
#define rramp 40  /* right     ampersand */
#define llamp 50  /* left      ampersand */
#define rralt 70  /* right     vertical bar */
#define llalt 60  /* left      vertical bar */
#define rrcnc 90  /* right     blank */
#define llcnc 80  /* left      blank */
#define rrats 110 /* right     at */
#define llats 100 /* left      at */
#define rrplm 120 /* right     plus, minus */
#define llplm 130 /* left      plus, minus */
#define rrnum 140 /* right     number */
#define llnum 150 /* left      number */
#define rrdvd 160 /* right     slash */
#define lldvd 170 /* left      slash */
#define rrmlt 180 /* right     asterisk */
#define llmlt 190 /* left      asterisk */
#define rrpct 200 /* right     percent */
#define llpct 210 /* left      percent */
#define rrexp 230 /* right     exclamation */
#define llexp 220 /* left      exclamation */
#define rrdld 240 /* right     dollar, dot */
#define lldld 250 /* left      dollar, dot */
#define rrnot 270 /* right     not */
#define llnot 260 /* left      not */
#define lluno 999 /* left      all unary operators */

/*
/   EFBLK - external function block
/
*/

struct efblk {
    word fcode;          /*  type word */
    word fargs;          /*  number of arguments */
    word eflen;          /*  block length */
    word efuse;          /*  usage count */
    void *efcod;         /*  pointer to XNBLK */
    struct vrblk *efvar; /*  pointer to VRBLK */
    word efrsl;          /*  result type */
    word eftar[];        /*  argument types */
    /*
     * 0 - type is unconverted
     * 1 - string
     * 2 - integer
     * 3 - file
     * 4 - real
     */
};

struct evblk {
    word evtyp;          /* 0;pointer to dummy routine b_evt */
    struct exblk *evexp; /* evtyp+1;pointer to exblk for expression */
    struct trblk *evvar; /* evexp+1;pointer to trbev dummy trblk */
};
struct exblk {
    word extyp;          /* 0;ptr to routine b_exl to load expr */
    word exstm;          /* cdstm;stores stmnt no. during evaluation */
    word exsln;          /* exstm+1;stores line no. during evaluation */
    word exlen;          /* exsln+1;length of exblk in bytes */
    word exflc;          /* exlen+1;failure code (=o_fex) */
    word excod[];        /* exflc+1;pseudo-code for expression */
};

struct ffblk {
    word fcode;
    word fargs;
    struct dfblk *ffdfp; /* fargs+1;pointer to associated dfblk */
    struct ffblk *ffnxt; /* ffdfp+1;ptr to next ffblk on chain or zero */
    word ffofs;          /* ffnxt+1;offset (bytes) to field in pdblk */
};

/*
/   ICBLK - integer block
/
/   Integer values are stored in ICBLKs.  Field icval should be defined
/   to be the appropriate type for the implementation.
*/

struct icblk {
    word icget; /*  type word - b$icl */
    long icval;
};
struct kvblk {
    word kvtyp;         /* 0;pointer to dummy routine b_kvt */
    word kvvar;         /* kvtyp+1;pointer to dummy block trbkv */
    word kvnum;         /* kvvar+1;keyword number */
};
struct nmblk {
    word nmtyp;         /* 0;ptr to routine b_nml to load name */
    void *nmbas;        /* nmtyp+1;base pointer for variable */
    word nmofs;         /* nmbas+1;offset for variable */
};
struct patblk {
    word pcode;         /* 0;ptr to match routine (p_xxx) */
    struct pblk *pthen; /* pcode+1;pointer to subsequent node */
    word parms[];       /* pthen+1;size of p0blk */
#define parm1 (parms[0]) /* pthen+1;first parameter value */
#define parm2 (parms[1]) /* parm1+1;second parameter value */
};

struct pdblk {
    word pdtyp;          /* 0;ptr to dummy routine b_pdt */
    word idval;
    struct dfblk *pddfp; /* idval+1;ptr to associated dfblk */
    void *pdfld[];       /* pddfp+1;start of field value pointers */
};
struct pfblk {
    word fcode;
    word fargs;
    word pflen;            /* :fargs+1;length of pfblk in bytes */
    struct vrblk *pfvbl;   /* :pflen+1;pointer to vrblk for function name */
    word pfnlo;            /* :pfvbl+1;number of locals */
    struct vrblk *pfcod;   /* :pfnlo+1;ptr to vrblk for entry label */
    struct trblk *pfctr;   /* :pfcod+1;trblk ptr if call traced else 0 */
    struct trblk *pfrtr;   /* :pfctr+1;trblk ptr if return traced else 0 */
    struct vrblk *pfarg[]; /* :pfrtr+1;vrblk ptrs for arguments and locals */
};

/*
/    RCBLK - real block
/
/    Real values are stored in RCBLKs.  Field rcval should be defined
/    to be the appropriate type for the implementation.
*/

struct rcblk {
    word rcget;   /*  type word - b$rcl */
    double rcval; /*  real value */
};

/*
/   SCBLK - string block
/
/   String values are stored in SCBLKs.  Notice that the scstr field
/   is defined as an array of characters of length 1.  This is a slight
/   kludge to side-step C's lack of support for varying length strings.
/
/   The actual length of a SCBLK is 2 words + the number of words necessary
/   to hold a string of length sclen.
*/

struct scblk {
    word scget;   /*  type word - b$scl */
    word sclen;   /*  string length */
    char schar[]; /*  string characters */
};

struct seblk {
    word setyp;          /* 0;ptr to routine b_sel to load expr */
    struct vrblk *sevar; /* setyp+1;ptr to vrblk for variable */
};
struct svblk {
    word svbit;          /* 0;bit string indicating attributes */
#define svpre 0x0001  /* 1;set if preevaluation permitted */
#define svffc 0x0002  /* svpre+svpre;set on if fast call permitted */
#define svckw 0x0004  /* svffc+svffc;set on if keyword value constant */
#define svprd 0x0008  /* svckw+svckw;set on if predicate function */
#define svnbt 4       /* number of bits to right of svknm */
#define svknm 0x0010  /* svprd+svprd;set on if keyword association */
#define svfnc 0x0020  /* svknm+svknm;set on if system function */
#define svnar 0x0040  /* svfnc+svfnc;set on if system function */
#define svlbl 0x0080  /* svnar+svnar;set on if system label */
#define svval 0x0100  /* svlbl+svlbl;set on if predefined value */
#define svfnf 0x0060; /* svfnc+svnar;function with no fast call */
#define svfnn 0x0062; /* svfnf+svffc;function with fast call, no preeval */
#define svfnp 0x0063  /* svfnn+svpre;function allowing preevaluation */
#define svfpr 0x006a  /* svfnn+svprd;predicate function */
#define svfnk 0x0072  /* svfnn+svknm;no preeval func + keyword */
#define svkwv 0x0110  /* svknm+svval;keyword + value */
#define svkwc 0x0014  /* svckw+svknm;keyword with constant value */
#define svkvc 0x0114  /* svkwv+svckw;constant keyword + value */
#define svkvl 0x0194  /* svkvc+svlbl;constant keyword + value + label */
#define svfpk 0x0177  /* svfnp+svkvc;preeval fcn + const keywd + val */
    word svlen;    /* 1;(=sclen) length of name in chars */
    word svchs[];  /* 2;(=schar) characters of name */
};
struct tbblk {
    word tbtyp;    /* 0;pointer to dummy routine b_tbt */
    word idval;
    word tblen;    /* offs2;length of tbblk in bytes */
    word tbinv;    /* offs3;default initial lookup value */
    void *tbbuk[]; /* tbinv+1;start of hash bucket pointers */
#define tbnbk 11   /* default no. of buckets */
};
struct teblk {
    word tetyp;              /* 0;pointer to dummy routine b_tet */
    word tesub;              /* tetyp+1;subscript value */
    word teval;              /* tesub+1;(=vrval) table element value */
    word tenxt;              /* teval+1;link to next teblk */
};
struct trblk {
    word tridn;              /* 0;pointer to dummy routine b_trt */
    word trtyp;              /* tridn+1;trap type code */
    union {
        word trval;          /* trtyp+1;value of trapped variable (=vrval) */
        struct trblk *trnxt; /* trval;ptr to next trblk on trblk chain */
        word trlbl;          /* trval;ptr to actual label (traced label) */
        struct vrblk *trkvr; /* trval;vrblk pointer for keyword trace */
    };
    union {
        word trtag;          /* trval+1;trace tag */
        struct vrblk *trter; /* trtag;ptr to terminal vrblk or null */
        struct trblk *trtrf; /* trtag;ptr to trblk holding fcblk ptr */
    };
    union {
        struct vrblk *trfnc; /* trtag+1;trace function vrblk (zero if none) */
        struct fcblk *trfpt; /* trfnc;fcblk ptr for sysio */
    };
};
enum trtyp {
    trtin = 0,    /* 0;trace type for input association */
    trtac = 1,    /* trtin+1;trace type for access trace */
    trtvl = 2,    /* trtac+1;trace type for value trace */
    trtou = 3,    /* trtvl+1;trace type for output association */
    trtfc = 4,    /* trtou+1;trace type for fcblk identification */
};
struct vcblk {
    word vctyp;   /* 0;pointer to dummy routine b_vct */
    word idval;
    word vclen;   /* offs2;length of vcblk in bytes */
    word vcvls[]; /* offs3;start of vector values */
#define vcvlb (vcvls[0]) /* vcvls-1;offset one word behind vcvls */
};

/*
 * VARIABLE BLOCK
 *
 * A variable block (VRBLK) is used to hold a program variable.
 */

struct vrblk {
    word vrget;          /* routine to load variable onto stack */
    word vrsto;          /* routine to store stack top into var. */
    union block *vrval;  /* variable value */
    word vrtra;          /* routine to transfer to label */
    union block *vrlbl;  /* pointer to code for label */
    union block *vrfnc;  /* function block if name is function */
    struct vrblk *vrnxt; /* next vrblk on hash chain */
    word vrlen;          /* length of name */
    char vrchs[];        /* vrlen+1;characters of name (vrlen gt 0) */
};

struct xnblk {
    word xntyp;    /* 0;pointer to dummy routine b_xnt */
    word xnlen;    /* xntyp+1;length of xnblk in bytes */
    word xndta[];  /* xnlen+1;data words */
};
struct xrblk {
    word xrtyp;    /* 0;pointer to dummy routine b_xrt */
    word xrlen;    /* xrtyp+1;length of xrblk in bytes */
    void *xrptr[]; /* xrlen+1;start of address pointers */
};
struct reloc_cr {
    word rlend;    /* 0;end */
    word rladj;    /* rlend+1;adjustment */
    word rlstr;    /* rladj+1;start */
};
#define rnsi_ 5 /* number of structures */

/*
/    BLOCK - an arbitrary block
*/

union block {
    struct arblk arb;
    struct bcblk bcb;
    struct bfblk bfb;
    struct ccblk ccb;
    struct cdblk cdb;
    struct cmblk cmb;
    struct cttyp cctypb;
    struct dfblk dfb;
    struct dvblk dvb;
    struct efblk efb;
    struct evblk evb;
    struct exblk exb;
    struct ffblk ffb;
    struct icblk icb;
    struct rcblk rcb;
    struct nmblk nmb;
    struct patblk patb;
    struct pdblk pdb;
    struct pfblk pfb;
    struct scblk scb;
    struct seblk seb;
    struct svblk svb;
    struct tbblk tbb;
    struct teblk teb;
    struct trblk trb;
    struct vcblk vcb;
    struct vrblk vrb;
    struct xnblk xnb;
    struct xrblk xrb;
};

#endif
