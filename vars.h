/*
 * QED
 */

#ifndef VARSINCLUDED
#define VARSINCLUDED

#define ESCBYTE '\370'
#define ESCAPE 0200000
#define UNESCAPE 0177777
#define QUOT(c) ((c) | ESCAPE) 
#define QUOTED(c) ((c) & ESCAPE)
#define UNQUOT(c) ((c) & UNESCAPE)

extern jmp_buf env;
#define setexit() setjmp(env)
#define reset() longjmp(env, 0)
#define TRUE 1
#define FALSE 0
#define LBSIZE 4096
#define RHSIZE (LBSIZE/4) /* ed says /2; but that's unreasonable! */
#define NBRA 9
#define EOF (-1)
#define FILERR 0200
#define SAVENEVER 0
#define SAVEIFFIRST 1
#define SAVEALWAYS 2
#define LDCHUNK 1024  /* realloc amount */
/*
 * Stack types.  Must appear in the order as in cspec[]/getchar.
 * XTTY, GLOB and BRWS are special types with no corresponding special character.
 * Special is never used directly - it is just used in tracing
 * pushinp()/getchar.c can be called with a negative index for cspec[]
 */
extern char special[]/*="xgBbBcfFlprzN\"\\'0"*/;
#define cspec (special + 3)
#define XTTY  0175
#define GLOB  0176
#define BRWS  0177
#define BUF  0
#define CURBN  1
#define QUOTE  2
#define FILEN  3
#define BFILEN  4
#define TTY  5
#define PAT  6
#define RHS  7
#define STRING  8
#define NEWL  9
#define NOTHING  10
#define BACKSLASH 11
#define LITERAL  12
#define OCTAL 13
/*
 * Getchar-linked macros
 */
#define ungetchar(c) (peekc = (c))
#define nextchar() (peekc = getchar())
#define NBUFS 56
/*
 * The buffer structure.  All info associated with each buffer stored here
 */
struct buffer{
  int *zero;
  int *dot;
  int *dol;
  char cflag;
  char gmark;
};
extern struct buffer buffer[NBUFS];
extern struct buffer *curbuf;
/*
 * The string structure
 * The first NBUFS strings are the registers
 * The next NBUFS are the file names
 * The next two are the saved pattern and saved right hand side
 * The next is the special register for browsing (the `ob' command)
 * The next is a file buffer.
 * Strings are stored in the `strarea' area and garbage collected
 * when the area is full.  The NSTRCHARS parameter can be increased
 * if you've got lots of core.
 * The first two characters of `strarea' form the null string.
 * The third and subsequent characters are the storage for non null
 * strings.
 * The null string has to be stored in this area so that
 * the R and S commands write out valid pointers to the null string.
 * The last entry in the string structure is used to store the pointer
 * to the next free position.
 * In string.c strfree is defined as "string[NSTRING].str".
 */
#define NSTRING (NBUFS+NBUFS+4)
#define COUNT (26+'C'-'A')
#define TRUTH (26+'T'-'A')
#define UNIX (26+'U'-'A')
#define FILE(z) (NBUFS+(z))
#define SAVPAT (NBUFS+NBUFS)
#define SAVRHS (SAVPAT+1)
#define BROWSE (SAVRHS+1)
#define FILEBUF (BROWSE+1)
struct string{
  int size;
  int len;
  char *str;
};
extern struct string string[NSTRING+1];
#define NSTRCHARS 4096 
extern char strarea[NSTRCHARS + 2];
#define nullstr strarea
#define strchars (&strarea[2])
#define STACKSIZE 16
/*
 * The getchar stack.
 */
struct stack{
  char type;
  char literal;
  union{
    struct buffer *u1bufptr;
    char *u1globp;
  }u1;
  union{
    int u2lineno;
    int u2strname;
  }u2;
  int charno;
};
#define bufptr u1.u1bufptr
#define globp u1.u1globp
#define lineno u2.u2lineno
#define strname u2.u2strname
extern struct stack stack[STACKSIZE];
extern struct stack *stackp;
extern int peekc;
extern int lastc;
extern char line[70];
extern char *linp;
extern int savedfile;
extern char linebuf[LBSIZE];
extern int *zero;
extern int *dot;
extern int *dol;
extern int  *lastdol;
extern int *begcore;
extern int *endcore;
extern int *fendcore;
extern int *addr1;
extern int *addr2;
extern char genbuf[LBSIZE];
extern char *linebp;
extern struct termios ttybuf;
extern int ninbuf;
extern int io;
extern sig_t onhup;
extern sig_t onquit;
extern sig_t onintr;
extern char lasterr;
#define PAGESIZE 22
extern int pagesize;
extern char bformat; /* = 'p' */
extern int appflag;
extern int cflag;
extern int cprflag;
extern int dflag;
extern int eflag;
extern int gflag;
extern int biggflag;
extern int iflag;
extern int prflag;
extern int tflag;
extern int uflag;
extern int vflag;
extern int qok;
extern int eok;
extern int initflag;
extern int nestlevel;
extern int lastttyc;
extern int listf;
extern int tfile;
extern int tfile2;
extern char *loc1;
extern char *loc2;
extern int names[NBUFS];
extern char *braslist[NBRA];
extern char *braelist[NBRA];
extern int nbra;
extern int oneline;
extern int lock;
extern char bname[]/*="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ{|}~"*/;
extern char lchars[]/*="pPlL"*/;
extern int bbempty; /* whether getc's internal buffer buffer needs reloading */
extern char tfname[]/*="/tmp/qed.XXXXXXXXXX"*/;

extern int aflag;  /* process regexes as 8-bit ascii, or UTF-8 */

#endif

