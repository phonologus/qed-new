#include "qed.h"

void newcore(size_t x);
void morecore(size_t x);
void bufinit(ldesc *n);
void chngbuf(int bb);
void newbuf(int bb);
void fixbufs(int n);
void relocatebuf(ldesc *from, ldesc *to);
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

void
newcore(size_t x)
{
   if(x<3)
      error('c');
   free(begcore);
   begcore = (ldesc *)qlloc(x*sizeof(ldesc));
   fendcore = begcore + x;
   endcore = fendcore - 2;
}
void
morecore(size_t x)
{
   ldesc *o1, *o2;
   size_t n;
   o1=begcore;
   o2=fendcore;
   n=(o2-o1)+x+(LDCHUNK-1) & ~(LDCHUNK-1);
   begcore=reqlloc(begcore,n * sizeof(ldesc));
   fendcore=begcore+n;
   endcore=fendcore-2;
   memset(begcore+(o2-o1),0,x * sizeof(ldesc));
   if(o1!=begcore)
      relocatebuf(o1,begcore);
}

void
bufinit(ldesc *n)
{
   struct buffer *bufp;
   for(bufp=buffer;bufp!=buffer+NBUFS;bufp++){
      bufp->zero=n;
      bufp->dot=n;
      bufp->dol=n;
      bufp->cflag = FALSE;
   }
}
void
chngbuf(int bb)
{
   syncbuf();
   newbuf(bb);
   savedfile = FILE(curbuf-buffer);
}
void
newbuf(int bb)
{
   curbuf = buffer+bb;
   zero=curbuf->zero;
   dot=curbuf->dot;
   dol=curbuf->dol;
   cflag = curbuf->cflag;
}
void
fixbufs(int n)
{
   struct buffer *bufp;
   lock++;
   for(bufp=curbuf+1;bufp!=buffer+NBUFS;bufp++){
      bufp->zero+=n;
      bufp->dot+=n;
      bufp->dol+=n;
   }
   unlock();
}
void
relocatebuf(ldesc *from, ldesc *to)
{
   struct buffer *bufp;
   ptrdiff_t d;
   d=to-from;
   lock++;
   for(bufp=buffer;bufp!=buffer+NBUFS;bufp++){
      bufp->zero+=d;
      bufp->dot+=d;
      bufp->dol+=d;
   }
   /* fix the ldesc* globals */
   zero+=d;
   dot+=d;
   dol+=d;
   lastdol+=d;
   addr1+=d;
   addr2+=d;
   unlock();
}
void
shiftbuf(int up)
{
   struct buffer *bufp;
   if(up)
      for(bufp=buffer;bufp!=buffer+NBUFS;bufp++){
         bufp->zero=begcore+(ptrdiff_t)bufp->zero;
         bufp->dot=begcore+(ptrdiff_t)bufp->dot;
         bufp->dol=begcore+(ptrdiff_t)bufp->dol;
      }
   else
      for(bufp=buffer;bufp!=buffer+NBUFS;bufp++){
         bufp->zero=(ldesc*)(bufp->zero-begcore);
         bufp->dot=(ldesc*)(bufp->dot-begcore);
         bufp->dol=(ldesc*)(bufp->dol-begcore);
      }
}
void
syncbuf(void)
{
   curbuf->zero = zero;  /* we never assign to it, so needn't save */
   curbuf->dot=dot;
   curbuf->dol=dol;
   curbuf->cflag = cflag!=FALSE;   /* Normalize to fit in a char */
}
void
error(int code)
{
   extern sig_t savint;   /* error during a ! > < | ?? */
   unlock();
   if(code){
      for(;stackp != stack ;--stackp)
         if(stackp->type == BUF || stackp->type == STRING){
            putchar('?');
            if(stackp->type == BUF){
               putchar('b');
               putchar(bname[stackp->bufptr - buffer]);
               putlong((long)stackp->lineno);
               putchar('.');
            }else{
               putchar('z');
               putchar(bname[stackp->lineno]);
            }
            putlong((long)stackp->charno);
            putchar(' ');
         }
      putchar('?');
      putchar(lasterr=(code&~FILERR));
      if(code&FILERR){
         putchar(' ');
         putl(string[FILEBUF].str);
      } else
         putchar('\n');
   }
   if(eflag && code){     /* temp file is *not* unlinked. Intentional? */
      free(begcore);
      exit(code);
   }
   nestlevel = 0;
   listf = FALSE;
   gflag = FALSE;
   biggflag = FALSE;
   stackp = stack;
   peekc = 0;
   if(code && code!='?'){      /* if '?', system cleared tty input buf */
      while(lastttyc!='\n' && lastttyc!=EOF){
         getchar();
      }
   }
   lseek(0, 0L, SEEK_END);
   if (io > 0) {
      close(io);
      io = -1;
      cflag = TRUE;   /* well, maybe not, but better be safe */
   }
   appflag=0;
   if(savint!=SIG_ERR){
      signal(SIGINT, savint);
      savint= SIG_ERR;
   }
   reset();
}

char tfname[]="/tmp/qed.XXXXXXXXXX";
int tfnX=10;   /* Number of X-s in the template */

void
init(void)
{
   char *p;
   int n=tfnX;
   lock++;

   if(tfile>0){
      close(tfile);
      unlink(tfname);
   }
   p=tfname+strlen(tfname);
   while(n-->0)
      *--p='X';
   tfile = mkstemp(tfname);
   fcntl(tfile, F_SETFD, FD_CLOEXEC);
   bufinit(begcore);
   newbuf(0);
   lastdol=dol;
   stackp=stack;
   stackp->type=TTY;
   unlock();
}
void
comment(void)
{
   int c, mesg;

   c = getchar();
   mesg = 0;
   if(c == '\"') {
      mesg++;
      c = getchar();
   }
   while(c != '\n' && c != '\"') {
      if(mesg)
         putchar(c);
      c = getchar();
   }
   if(mesg) {
      if(c == '\n')
         putchar(c);
      flush();
   }
}
int
abs(int n)
{
   return(n<0?-n:n);
}
/*
 * Slow, but avoids garbage collection
 */
void
settruth(int t)
{
   if(stoi(string[TRUTH].str) != t)
      numset(TRUTH, t);
}
void
setcount(int c)
{
   if(stoi(string[COUNT].str) != c)
      numset(COUNT, c);
}
int
truth(void)
{
   return(stoi(string[TRUTH].str) != FALSE);
}
void
modified(void)
{
   cflag=TRUE;
   eok=FALSE;
   qok=FALSE;
}
