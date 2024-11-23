#include "qed.h"

struct buffer buffer[NBUFS];
struct buffer *curbuf=buffer;
struct string string[NSTRING+1];
char *strarea;
size_t nstrarea;
struct stack stack[STACKSIZE];
struct stack *stackp;

int peekc;
int lastc;
char line[LINECHARS];
char *linp = line;
int savedfile;
char linebuf[LBSIZE];
ldesc *zero;
ldesc *dot;
ldesc *dol;
ldesc  *lastdol;
ldesc *begcore;
ldesc *endcore;
ldesc *fendcore;
ldesc *addr1;
ldesc *addr2;
char genbuf[LBSIZE];
char *linebp;
struct termios ttybuf;
int ninbuf;
int io;
sig_t onhup;
sig_t onquit;
sig_t onintr;
char lasterr;
int aflag = 0; /* UTF-8 */
int appflag = 0;
int cflag;
int cprflag;
int dflag = 0;
int eflag;
int gflag = 0;
int biggflag;
int iflag;
int prflag = 0; /* prflag==1 ==> much code to get it right. use the startup buffer */
int tflag = 0;
int uflag = 's';
int vflag = 0;
int qok;
int eok;
int initflag = 1;
int nestlevel;
int lastttyc='\n';
int listf;
int tfile=-1;
char *loc1;
char *loc2;
ldesc names[NBUFS];
char *braslist[NBRA];
char *braelist[NBRA];
int nbra;
int oneline;
int lock;
char lchars[]="pPlL";
int bbempty;

int pagesize = PAGESIZE;
int   *option[] = {
   &aflag, &cflag, &dflag, &eflag, &iflag, &prflag, &tflag, &vflag
};
char opcs[] = "acdeipTv";
char QEDFILE[]="QEDFILE";
sig_t pending;
jmp_buf env;

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

void
rescue(int s)
{
   /* Save in qed.hup:[ab]q on hangup */
   signal(SIGHUP,SIG_IGN);
   if (lock) {
      pending = rescue;
      return;
   }
   startstring();
   copystring("qed.hup");
   setstring(FILEBUF);
   savall();
   free(begcore);
   exit(SIGHUP);
}
char *
filea(void)
{
   struct string *sp;
   int i, d;
   char c;

   sp = &string[FILEBUF];
   startstring();
   d = 0;
   i = 0;
   while((c=sp->str[i]) != '\0'){
      addstring(c);
      i++;
      if(c == '/')
         d = i;
   }
   if((i-d) > 12)      /* file name is >= 13 characters long */
      string[NSTRING].str -= (i-(d+12));   /* truncate string */
   copystring(":aq");
   setstring(FILEBUF);
   return(sp->str);
}

char *
fileb(void)
{
   struct string *sp;

   sp = &string[FILEBUF];
   sp->str[sp->len-2] = 'b';
   return(sp->str);
}
#define LWIDTH 8
void
write_long(int fd, long n)
{
   unsigned char b[LWIDTH];

   set_val_le(n, LWIDTH, b);
   
   if(write(fd,(char *)b,LWIDTH) != LWIDTH)
      error('S');
    
   return;
}
long
read_long(int fd)
{
   unsigned char b[LWIDTH];
   
   if(read(fd,(char *)b,LWIDTH) != LWIDTH)
      error('R');
    
   return get_val_le(LWIDTH,b);
}
void
savall(void)
{
   int fi;
   unsigned long n;

   syncbuf();
   addr1 = buffer[0].zero + 1;
   addr2 = buffer[NBUFS-1].dol;
   if(addr1 > addr2){
      error('$');
      return;
   }
   if((io = open(filea(), O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
      error('o'|FILERR);
   putfile();
   exfile();
   if((fi = open(fileb(), O_CREAT | O_TRUNC | O_WRONLY, 0644)) < 0)
      error('o'|FILERR);
   write_long(fi,n = sizeof buffer);
   lock++;
   shiftbuf(DOWN);
   if(write(fi, (char *)buffer, n) != n)
      error('S');;
   shiftbuf(UP);
   unlock();
   write_long(fi,n = nstrarea);
   if(write(fi, strarea, n) != n)
      error('S');
   write_long(fi,n = sizeof string);
   lock++;
   shiftstring(DOWN);
   if(write(fi, (char *)string, n) != n)
      error('S');;
   shiftstring(UP);
   unlock();
   close(fi);
}
void
restor(void)
{
   int t, fi;
   unsigned long n;
   curbuf = buffer;
   if((t = open(filea(), 0)) < 0){
      lastc = '\n';
      error('o'|FILERR);
   }
   initio();
   init();
   io = t;
   ninbuf = 0;
   append(getfile, dol);
   exfile();
   if((fi = open(fileb(),0)) < 0)
      error('o'|FILERR);
   lock++;
   /* read size of buffer */
   n=read_long(fi);
   if(read(fi,(char *)buffer,n) != n)
      error('R');
   /* DO: read size of strarea */
   n=read_long(fi);
   strarea=reqlloc(strarea,n);
   if(read(fi, strarea,n) != n)
      error('R');
   /* read size of string */
   n=read_long(fi);
   if(read(fi, (char *)string,n) != n)
      error('R');
   close(fi);
   shiftstring(UP);
   shiftbuf(UP);
   newbuf(0);
   error(0);   /* ==> error, but don't print anything. calls unlock() */
}

/*
 *	On INTR, generate error '?'
 */

void
interrupt(int s)
{
   signal(SIGINT, interrupt);
   if (lock) {
      pending = interrupt;
      return;
   }
   if(iflag){
      unlink(tfname);
      free(begcore);
      exit(SIGINT);
   }
   linp=line;
   putchar('\n');
   lastc = '\n';
   error('?');
}

/*
 * Unlock: exit a critical section, invoking any pending signal routines.
 */

void
unlock(void)
{
   sig_t p;

   p = pending;
   pending = SIG_DFL;
   lock = 0;
   if (p==rescue)
      (*p)(SIGHUP);
   else if (p==interrupt)
      (*p)(SIGINT);
}

char cleanup[] = "ba z~:\n";
char setvflag[] = "ov?";
char boot1[] = "G/^[";
char boot2[] = "].+\t./r\n";

int
main(int argc, char **argv)
{
   char *p1;
   int i;
   char buf;
   int rvflag;
   char *startup=(char *)0;
   int optend=0;

   argv++;
   onquit = signal(SIGQUIT, SIG_IGN);
   onhup = signal(SIGHUP, SIG_IGN);
   onintr = signal(SIGINT, SIG_IGN);
   rvflag = 1;
   newstrarea();
   while(!optend && argc > 1 && **argv=='-'){
      switch(argv[0][1]){
      casedefault:
      default:
         rvflag = 0;
         break;
      case 'a':
      /* process strings as ascii (bytes) */
         aflag++;
         break;
      case 'q':
      /* allow debugging quits? */
         signal(SIGQUIT, SIG_DFL);
         break;
      case 'i':
      /* allow interrupt quits? */
         iflag++;
         break;
      case 'e':
      /* Exit on error? */
         eflag++;
         break;
      case 'x':
         if(argc == 2)
            goto casedefault;
         startup = argv[1];
         argv++;
         --argc;
      case '-':
         optend++;
         break;
      }
      argv++;
      --argc;
   }
   tcgetattr(0,&ttybuf);
   if(startup==0)
      startup = getenv(QEDFILE);
   newcore();
   curbuf = buffer;
   init();
   if (onhup != SIG_IGN)
      signal(SIGHUP, rescue);
   if (onintr != SIG_IGN)
      signal(SIGINT, interrupt);
   /*
    * Build the initialization code in register z~
    */
   if(startup){
      startstring();
      copystring(startup);
      setstring(FILE(NBUFS-1));
      p1 = "b~ r\n\\b~\n";
   } else
      p1 = "";
   startstring();
   copystring(p1);
   setvflag[2] = "rs"[rvflag];
   copystring(setvflag);
	/*
	 * z~ now has startup-buffer initialization; prepare the file list
	 * and generate a GLOBUF to read them in
    */
   if(--argc > 0) {
      if(argc >= 53)   /* buffer names a-zA-Z */
         puts("?i");
      else {
         copystring(boot1);
         for(i=0; i<argc; i++)   /* argument files only */
            addstring(bname[i]);
         copystring(boot2);
         copystring(cleanup);
         setstring(NBUFS-1);
         buf = 0;
         while(argc > 0) {
            startstring();
            copystring(*argv);
            setstring(FILE(buf++));
            --argc;
            argv++;
         }
      }
   }
   else{
      copystring(cleanup);
      setstring(NBUFS-1);
   }
   pushinp(STRING, NBUFS-1, FALSE);
   setexit();
   lastttyc = '\n';
   commands();
   unlink(tfname);
   free(begcore);
   exit(lasterr);
}

int noaddr;
void
commands(void)
{
   ldesc *a;
   int c, lastsep;
   int r;
   int changed;
   long locn;
   int startline;

   for (;;) {
   startline = (lastttyc == '\n' && peekc == 0);
   cprflag=prflag;
   c = '\n';
   for (addr1=0;;) {
      lastsep = c;
      a=address();
      c=getchar();
      if (c!=',' && c!=';')
         break;
      if (lastsep==',')
         error('a');
      if (a==0) {
         a = zero+1;
         if (a>dol)
            --a;
      }
      addr1 = a;
      if (c==';')
         dot = a;
   }
   if (lastsep!='\n' && a==0)
      a=dol;
   if((addr2=a) == 0) {
      addr2=dot;
      noaddr = TRUE;
   } else
      noaddr = FALSE;

   if(addr1 == 0)
      addr1 = addr2;

   cprflag=FALSE;
   switch(c){
   case 'a':
      setdot();
      setapp();
      append(gettty, addr2);
      continue;
   case 'b':
      if(posn((c=nextchar()), bname)<0){   /* browse command */
         setdot();
         nonzero();
         bcom();
         continue;
      }
      c = getaz('b');
      setnoaddr();
      chngbuf(c);
      continue;
   case 'c':
      setdot();
      nonzero();
      setapp();
      append(gettty, addr2);
      a = dot-(addr2-addr1+1);
      delete();
      dot = a;
      continue;
   case 'd':
      if(posn(nextchar(),"\377\npPlL \t") < 0)
         error('x');
      delete();
      continue;
   case 'E':
   case 'e':
      setnoaddr();
      if(c=='e' && !eok && cflag){
         eok=TRUE;
         error('e');
      }
      newfile(TRUE, SAVEALWAYS, "");
      delall();
      addr1 = zero;
      addr2 = zero;
      modified();   /* In case file open causes error */
      goto caseread;
   case 'f':
      setnoaddr();
      if((c = getchar()) != '\n'){
         ungetchar(c);
         if(newfile(FALSE, SAVEALWAYS, string[savedfile].str))
            modified();
         if(vflag)
            ncom('f');
      }
      else
         ncom('f');
      continue;
   case 'g':
      global(TRUE);
      continue;
   case 'G':
      globuf(TRUE);
      continue;
   case 'h':
      setnoaddr();
      if(nextchar()=='\n')
         error('x');
      if('0'<=peekc && peekc<='9')
         until(TRUE, getnum());
      else
         until(FALSE, 0);
      continue;
   case 'i':
      setdot();
      nonzero();
      setapp();
      append(gettty, addr2-1);
      continue;
   case 'j':
      setdot();
      if (addr1 == addr2 && lastsep == '\n'){
         addr1--;
         if(addr1 <= zero)
            error('$');
      }
      nonzero();
      join();
      continue;
   case 'k':
      c = getaz(c);
      setdot();
      nonzero();
      names[c] = *addr2;
      names[c].flags=1;
      continue;
   case 'm':
      move(FALSE);
      continue;
   case 'n':
   case 'N':
      ncom(c);
      continue;
   case 'o':
      setnoaddr();
      c = getchar();
      r=posn(c, opcs);
      if(r >= 0)
         setreset(option[r]);
      else switch(c) {
      case 'B':
         if(nextchar() == '\n')
            clearstring(BROWSE);
         else {
            startstring();
            while((c=getchar()) != '\n')
               addstring(c);
            copystring("\\N");
            setstring(BROWSE);
         }
         break;
      case '?':
         if ((r = posn(getchar(), opcs)) < 0)
            error('O');
         settruth(*option[r]);
         break;
      case 'q':
         c = getchar();
         if(c=='s' || c =='r')
            signal(SIGQUIT, c=='r'?SIG_IGN:SIG_DFL);
         else
            error('x');
         break;
      case 'u':
         c = getchar();
         if(c == 'r')
            uflag = 0;
         else if(posn(c, "slu") >= 0)
            uflag = c;
         else
            error('x');
         break;
      case 'b':
         if((r=getnum()) > 0)
            pagesize = r;
         if(posn(nextchar(), lchars) >=0)
            bformat = getchar();
         break;
      default:
         error('x');
      }
      continue;
   case '\n':
      if (a==0) {
         if(stackp != &stack[0] || !startline)
            continue;
         if(*string[BROWSE].str){
            pushinp(BRWS, 0, FALSE);
            continue;
         }
         a = dot+1;
         addr2 = a;
         addr1 = a;
      }
      if (lastsep==';')
         addr1 = a;
      c = 'p';   /* fall through */
   case 'L':
   case 'l':
   case 'p':
   case 'P':
      display(c);
      continue;
   case EOF:
      return;
   case 'Q':
   case 'q':
      setnoaddr();
      if(c!=EOF && (!startline || getchar()!='\n'))
         error('x');
      if(c!='Q' && !qok){
         struct buffer *bp;
         syncbuf();
         qok=TRUE;
         for(bp=buffer; bp<&buffer[NBUFS]; bp++)
            if(bp->cflag && (bp->dol>bp->zero ||
               string[FILE(bp-buffer)].str[0]))
               error('q');
      }
      unlink(tfname);
      free(begcore);
      exit(0);   /* exit(0) not lasterr, otherwise caller gets confused */ 
   case 'r':
      newfile(TRUE, SAVEIFFIRST, string[savedfile].str);
   caseread:
      if((io = open(string[FILEBUF].str, 0)) < 0){
         if(initflag){
            putchar('?');
            putchar('o');
            putchar(' ');
            puts(string[FILEBUF].str);
            continue;
         }
         lastc = '\n';
         error('o'|FILERR);
      }
      setall();
      changed = (zero!=dol);
      ninbuf = 0;
      append(getfile, addr2);
      if(eqstr(string[savedfile].str, string[FILEBUF].str))
         if((cflag = changed))   /* Assignment = */
            modified();
      /* else append got cflag right */
      exfile();
      continue;
   case 'R':
      setnoaddr();
      newfile(TRUE, SAVENEVER, "q");
      restor();
      continue;
   case 's':
      setdot();
      nonzero();
      substitute(stackp != &stack[0], -1);
      continue;
   case 'S':
      setnoaddr();
      newfile(TRUE, SAVENEVER, "q");
      savall();
      continue;
   case 't':
      move(TRUE);
      continue;
   case 'u':
      setnoaddr();
      undo();
      modified();
      continue;
   case 'v':
      global(FALSE);
      continue;
   case 'V':
      globuf(FALSE);
      continue;
   case 'W':
   case 'w':
      if(addr2==0 && dol==zero)
         error('$');
      setall();
      if(newfile(TRUE, SAVEIFFIRST, string[savedfile].str))
         changed = cflag;
      else
         changed = (addr1>(zero+1) || addr2!=dol);
      if(c=='w' || (io=open(string[FILEBUF].str,1))==-1){
       Create:
         if ((io = open(string[FILEBUF].str,
            O_CREAT | O_TRUNC | O_WRONLY, 0666)) < 0)
               error('o'|FILERR);
      }else{
         if((locn=lseek(io, 0L, SEEK_END)) == -1L)
            goto Create;
         if(locn != 0L)   /* W on non-empty file */
            changed = TRUE;   /* PHEW! figured it out */
      }
      putfile();
      if((cflag = changed))   /* Assignment = */
         modified();
      exfile();
      continue;
   case 'x':
      setdot();
      nonzero();
      xform();
      continue;
   case 'y':
      jump();
      continue;
   case 'z':
      strcom(getaz('z'));
      continue;
   case 'Z':
      setnoaddr();
      if((c=getchar())!=' ' && c!='\n')
         error('x');
      delall();
      cflag=FALSE;
      clearstring(savedfile);
      continue;
   case '"':
      setdot();
      dot=addr2;
      comment();
      continue;
   case '=':
      setall();
      putlong(addr2-zero);
      putchar('\n');
      continue;
   case '>':
   case '<':
   case '|':
      setall();
   case '!':
      Unix(c);
      continue;
   case '#':
      setnoaddr();
      allnums();
      continue;
   case '%':
      setnoaddr();
      allstrs();
      continue;
   }
   error('x');
   }
}
void
setreset(int *opt)
{
   int c;

   c = getchar();
   if(c!='s' && c!= 'r')
      error('x');
   *opt = (c=='s');
}
void
delall(void)
{
   if(dol!=zero){
      addr1=zero+1;
      addr2=dol;
      delete();
   }
}
