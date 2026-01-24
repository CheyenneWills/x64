
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
extern int check2str(void);
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
extern word *get_fp(void);
extern char **getargs(int argc, char *argv[]);
extern FILEPOS geteof(struct ioblk *ioptr);
extern void gethost(struct scblk *scptr, word maxlen);
extern int getint(struct icblk *icp, long *pword);
extern char *getnum(char *cp, uword *ip);
extern File_handle getprfd(void);
extern File_handle getrdfd(void);
extern struct ioblk *getrdiob(void);
extern int getsave(File_handle fd);
extern char *getshell(void);
extern void heapmove(void);
extern void initpath(char *name);
extern word lenfnm(struct scblk *scptr);
extern void *loadef(void *handle, char *filename);
extern int makeexec(struct scblk *scptr, int type);
extern char make_c_str(char *p);
extern void malloc_empty(void);
extern long moremem(long n);
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
extern word putsave(word *stkbase, word stklen);
extern int rdaout(File_handle fd, unsigned char *startadr, uword size);
extern int rdenv(struct scblk *varname, struct scblk *result);
extern void rereloc(void);
extern void restart(char *code, char *stack);
extern void restore0(void);
extern void restorestring(struct scblk *scp, word c);
extern void restore2str(void);
extern void restore3str(void);
extern void save0(void);
extern char *savestr(struct scblk *scp, char *cp);
extern void save2str(char **s1p, char **s2p);
extern void scanef(void);
extern word scnint(char *str, word len, word *intptr);
extern void setout(void);
extern File_handle spit_open(char *Name, Open_method Method, File_mode Mode,
                             int Access);
extern void startbrk(void);
extern void startup(void);
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
extern void unreloc(void);
extern word uppercase(word c);
extern int wrtaout(unsigned char *startadr, uword size);
extern void wrterr(char *s);
extern void wrtmsg(char *s);
extern int zysax(void);
extern int zysbs(void);
extern int zysbx(void);
extern int zyscm(void);
extern int zysdc(void);
extern int zysdm(void);
extern int zysdt(void);
extern int zysea(void);
extern int zysef(void);
extern int zysej(void);
extern int zysem(void);
extern int zysen(void);
extern int zysep(void);
extern int zysex(void);
extern int zysfc(void);
extern int zysgc(void);
extern int zyshs(void);
extern int zysid(void);
extern int zysif(void);
extern int zysil(void);
extern int zysin(void);
extern int zysio(void);
extern int zysld(void);
extern int zysmm(void);
extern int zysmx(void);
extern int zysou(void);
extern int zyspi(void);
extern int zyspl(void);
extern int zyspp(void);
extern int zyspr(void);
extern int zysrd(void);
extern int zysri(void);
extern int zysrw(void);
extern int zysst(void);
extern int zystm(void);
extern int zystt(void);
extern int zysul(void);
extern int zysxi(void);

/* prototypes for standard system-level functions used by OSINT */

#define LSEEK lseek

#endif
