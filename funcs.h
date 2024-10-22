#ifndef FUNCSDOTH
#define FUNCSDOTH

/* alloc.c */
void *qlloc(size_t);
void *reqlloc(void *, size_t);

/* address.c */

int * address(void);

/* blkio.c */

void initio(void);
char * getline(int tl, char *lbuf);
int putline(void);
void blkrd(int b, char *buf);
void blkwr(int b, char *buf);

/* com.c */

void jump(void);
void stacktype(int t);
void getlabel(void);
int * looper(int *a1, int  *a2, char *str, int dir);
void search(int forward);
void setapp(void);
int append(int (*f)(void), int *a);
void bcom(void);
void delete(void);
void allnums(void);
void numcom(int z);
int condition(int n, int  m, int  cond, int  negate);
void numset(int z, int n);
void numbuild(int n);
void strcom(int z);
void strinc(int z, int n);
int locn(char *ap, char  *aq);
void ncom(int c);
void allstrs(void);
void clean(int z);

/* getchar.c */

int geto(int d);
int getnum(void);
int getsigned(void);
int stoi(char *s);
int alldigs(char *s);
int getname(int e);
int getaz(int e);
int getnm(int e, int (*f)(void));
int getchar(void);
int getc(void);
int ttyc(void);
int posn(char c, char *s);
void pushinp(int type, uintptr_t arg, int literal);
void popinp(void);
int gettty(void);
int getquote(char *p, int (*f)(void));

/* getfile.c */

int newfile(int nullerr, int savspec, char *deffile);
void exfile(void);
int getfile(void);
void putfile(void);
void Unix(char type);

/* glob.c */

void until(int nfl, int n);
void global(int k);
void globuf(int k);
void getglob(char globuf[]);
int exglob(char *cmd, char  *dflt);

/* main.c */

void rescue(int);
char * filea(void);
char * fileb(void);
void savall(void);
void restor(void);
void interrupt(int);
void unlock(void);
void commands(void);
void setreset(int *opt);
void delall(void);

/* misc.c */

void morecore(size_t x);
void bufinit(int *n);
void chngbuf(int bb);
void newbuf(int bb);
void fixbufs(int n);
void relocatebuf(int *from, int *to);
void shiftbuf(int up);
void syncbuf(void);
void error(int code);
void init(void);
void comment(void);
int abs(int n);
void settruth(int t);
void setcount(int c);
int truth(void);
void modified(void);

/* move.c */

void move(int copyflag);
void fixup(int from, int to, int tot);
void reverse(int *a1, int  *a2);
int getcopy(void);

/* pattern.c */

void compile(char eof);
int getsvc(void);
int execute(int *addr);
int advance(char *lp, char  *ep);
int backref(int i, char *lp);
int alfmatch(char c, int tail);
int cclass(char *set, int c, int f);

/* putchar.c */

void putdn(int i);
void putlong(unsigned long i);
void putl(char *sp);
void puts(char *sp);
void display(int lf);
void putct(int c);
void putchar(char c);
void flush(void);

/* setaddr.c */

void setdot(void);
void setall(void);
void setnoaddr(void);
void nonzero(void);

/* string.c */

int length(char *s);
void startstring(void);
void addstring(int c);
void dropstring(void);
void cpstr(char *a, char  *b);
void shiftstring(int up);
void clearstring(int z);
void copystring(char *s);
int eqstr(char *a, char  *b);
void dupstring(int z);
void setstring(int n);
void strcompact(void);

/* subs.c */

void substitute(int inglob, int reg);
int compsub(int subbing, int *autop);
int getsub(void);
void dosub(void);
void place(char *l1, char *l2, int ucase);
void undo(void);
void replace(int *line, int ptr);
void join(void);
int next_col(int col, char *cp, int input);
void xform(void);

/* u.c */

int u_length(char c);
char *u_incrp(char **p);
char *u_decrp(char **p);
char *u_postincrp(char **p);
char *u_postdecrp(char **p);
int u_code(char *p);
int u_utf8(int c,char *p);
int u_count(char *p);
char *u_nth(char *p, int i);
int u_range(char *a, char *b);
void u_loadp(char **p, int c, char **q);
void u_loadfp(char **p, int c, int (*f)(void));
int u_loadcp(char **p, int c, char **q);
int u_loadcfp(char **p, int c, int (*f)(void));

#endif

