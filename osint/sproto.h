
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

/*
/    Function prototypes for argument type checking.
/    This files are unique to SPITBOL.
*/

#ifndef _sproto_
#define _sproto_

typedef int File_handle;
typedef int File_mode;
typedef int Open_method;

extern int appendext(char *path, char *ext, char *result, int force);
#if EXTFUN
extern union block *callef(struct efblk *efb, union block **sp, word nargs);
#endif
extern int checkstr(struct scblk *scp);
extern void close_all(struct chfcb *chb);
extern word closeaout(char *filename, char *tmpfnbuf, word errflag);
extern void clrbuf(void);
extern int compress(unsigned char *startadr, uword size);
extern void conv(char *dest, int value);
extern void cpys2sc(char *cp, struct scblk *scptr, word maxlen);
extern int docompress(int bits, char *freeptr, uword size);
extern void doexec(struct scblk *scbptr);
extern int doexpand(int bits, char *freeptr, uword size);
extern FILEPOS doset(struct ioblk *ioptr, FILEPOS offset, int whence);
extern int dosys(char *cmd, char *path);
extern void endbrk(void);
extern int expand(File_handle fd, unsigned char *startadr, uword size);
extern word fillbuf(struct ioblk *ioptr);
extern char *findenv(char *vq, int vn);
extern int flush(struct ioblk *ioptr);
extern int fsyncio(struct ioblk *ioptr);
extern long gencmp(char *, char *, long, long, long);
extern char **getargs(int argc, char *argv[]);
extern FILEPOS geteof(struct ioblk *ioptr);
extern void gethost(struct scblk *scptr, word maxlen);
extern int getint(struct icblk *icp, long *pword);
extern char *getnum(char *cp, uword *ip);
extern File_handle getprfd(void);
extern File_handle getrdfd(void);
extern struct ioblk *getrdiob(void);
extern int getsave(struct minimal_regs *minregs, File_handle fd);
extern char *getshell(void);
extern void heapmove(void);
extern void initpath(char *name);
extern word lenfnm(struct scblk *scptr);
extern void *loadef(void *handle, char *filename);
extern int makeexec(struct scblk *scptr, int type);
extern char make_c_str(char *p);
extern void malloc_empty(void);
extern void *nextef(unsigned char **bufp, int io);
extern int openaout(char *fn, char *tmpfnbuf, int exe);
extern int optfile(struct scblk *varname, struct scblk *result);
extern char *optnum(char *cp, uword *ip);
extern int osclose(struct ioblk *ioptr);
extern int osopen(struct ioblk *ioptr);
extern int ospipe(struct ioblk *ioptr);
extern word osread(word mode, word recsiz, struct ioblk *ioptr,
                   struct scblk *scptr);
extern void oswait(int pid);
extern word oswrite(word mode, word linesiz, word recsiz, struct ioblk *ioptr,
                    struct scblk *scptr);
extern void oupeof(void);
extern char *pathlast(char *path);
extern void prompt(void);
extern word putsave(struct minimal_regs *minregs, word *stkbase, word stklen);
extern int rdaout(File_handle fd, unsigned char *startadr, uword size);
extern int rdenv(struct scblk *varname, struct scblk *result);
extern void rereloc(struct minimal_regs *minregs);
extern void restart(char *code, char *stack);
extern void restore0(void);
extern void restorestring(struct scblk *scp, word c);
extern void restore3str(void);
extern void save0(void);
extern char *savestr(struct scblk *scp, char *cp);
extern void scanef(void);
extern word scnint(char *str, word len, word *intptr);
extern void setout(void);
extern File_handle spit_open(char *Name, Open_method Method, File_mode Mode,
                             int Access);
extern void startbrk(void);
extern void startup(struct minimal_regs *min_regs, long id);
extern int stcu_d(char *out, unsigned int in, int outlen);
extern void stdioinit(void);
extern int swcinp(int filecnt, char **fileptr);
extern int swcoup(char *oupptr);
extern void sysmm_init(void);
extern int testty(File_handle fd);
extern int tryopen(char *cp);
extern int trypath(char *name, char *file);
extern void ttyoutfdn(File_handle h);
extern void ttyinit(void);
extern void ttyraw(File_handle fd, int flag);
extern void unldef(struct efblk *efb);
extern void unmake_c_str(char *p, char saved_c);
extern void unreloc(struct minimal_regs *minregs);
extern word uppercase(word c);
extern int wrtaout(unsigned char *startadr, uword size);
extern void wrterr(char *fmt, ...);
extern int zysax(struct minimal_regs *min_regs);
extern int zysbs(struct minimal_regs *min_regs);
extern int zysbx(struct minimal_regs *min_regs);
extern int zyscm(struct minimal_regs *min_regs);
extern int zysdc(struct minimal_regs *min_regs);
extern int zysdm(struct minimal_regs *min_regs);
extern int zysdt(struct minimal_regs *min_regs);
extern int zysea(struct minimal_regs *min_regs);
extern int zysef(struct minimal_regs *min_regs);
extern int zysej(struct minimal_regs *min_regs);
extern int zysem(struct minimal_regs *min_regs);
extern int zysen(struct minimal_regs *min_regs);
extern int zysep(struct minimal_regs *min_regs);
extern int zysex(struct minimal_regs *min_regs);
extern int zysfc(struct minimal_regs *min_regs);
extern int zysgc(struct minimal_regs *min_regs);
extern int zyshs(struct minimal_regs *min_regs);
extern int zysid(struct minimal_regs *min_regs);
extern int zysif(struct minimal_regs *min_regs);
extern int zysil(struct minimal_regs *min_regs);
extern int zysin(struct minimal_regs *min_regs);
extern int zysio(struct minimal_regs *min_regs);
extern int zysld(struct minimal_regs *min_regs);
extern int zysmm(struct minimal_regs *min_regs);
extern int zysmx(struct minimal_regs *min_regs);
extern int zysou(struct minimal_regs *min_regs);
extern int zyspi(struct minimal_regs *min_regs);
extern int zyspl(struct minimal_regs *min_regs);
extern int zyspp(struct minimal_regs *min_regs);
extern int zyspr(struct minimal_regs *min_regs);
extern int zysrd(struct minimal_regs *min_regs);
extern int zysri(struct minimal_regs *min_regs);
extern int zysrw(struct minimal_regs *min_regs);
extern int zysst(struct minimal_regs *min_regs);
extern int zystm(struct minimal_regs *min_regs);
extern int zystt(struct minimal_regs *min_regs);
extern int zysul(struct minimal_regs *min_regs);
extern int zysxi(struct minimal_regs *min_regs);

/* prototypes for standard system-level functions used by OSINT */

#define LSEEK lseek

#endif
