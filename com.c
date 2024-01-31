#include "qed.h"
#define OUT 10
#define BACKWARD 11
#define FORWARD 12
char jumpcs[] = "0123456789o`'";

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

void
jump(void)
{
   int i;
   int cond;
   int c;

   if(stackp->type==TTY)
      error('y');
   c = getchar();
   cond = TRUE;
   if(c=='t' || c=='f'){
      cond = ((c=='t') == truth());
      c = getchar();
   }
   i = posn(c, jumpcs);
   if(i==-1){   /* to handle y[tf] without labels for globals */
      ungetchar(c);
      if(cond){
         while((c = getc())!='\n')
            if(c==EOF)
               return;
      }
   }
   else if(i<=9){
      stacktype(BUF);
      ungetchar(c);
      i = getnum();
      stacktype(BUF);      /* we may have popped stack! */
      if(cond){
         /* getchar will handle problems if off end of buffer */
         bbempty = TRUE;
         stackp->lineno = i;
         stackp->charno = 0;
      }
   }
   else if(i==OUT){
      if(cond){
         if(stackp==stack)
            error('y');
         if(stackp->type==GLOB){
            --nestlevel;
            stackp->globp = "";
         }else
            popinp();
      }
   }
   else if(i==FORWARD || i==BACKWARD){
      stacktype(BUF);
      getlabel();   /* must eat label even if not yumping */
      stacktype(BUF);
      if(cond)
         search(i==FORWARD);
   }
}
void
stacktype(int t)
{
   if(stackp->type != t)
      error('y');
}
void
getlabel(void)
{
   char *p, c;
   p = genbuf;
   for(c=getchar(); posn(c," \"\t\n")<0; c=getchar()){
      *p++ = c;
      *p = '\0';
   }
   if(p==genbuf)
      error('y');
}
int *
looper(int *a1, int  *a2, char *str, int dir)
{
   char *p1;
   char *p2;
   while(dir ? a1<=a2 : a1>=a2){
      p2 = getline(*a1, linebuf);
      while(*p2==' ' || *p2=='\t')
         p2++;
      if(*p2++=='"' && *p2!=' ' && *p2!='\t' && *p2!='\0') {
         for(p1=str; *p2 && (*p1 == *p2); p2++)
            p1++;
         if(*p1==0 && (*p2==0 || *p2==' ' || *p2=='\t'))
            return(a1);
      }
      if(dir)
         a1++;
      else
         --a1;
   }
   return((int *)0);
}
void
search(int forward)
{
   int *a1;
   struct buffer *bufp;
   bufp = stackp->bufptr;
   if(forward){
      if((a1 = looper(bufp->zero + (stackp->lineno + 1),
         bufp->dol,genbuf,1))==0){
         a1 = looper(bufp->zero + 1,bufp->zero + (stackp->lineno - 1),genbuf,1);
      }
   }else{
      if((a1 = looper(bufp->zero + (stackp->lineno - 1),
         bufp->zero + 1,genbuf,0))==0){
         a1 = looper(bufp->dol,bufp->zero + stackp->lineno + 1,genbuf,0);
      }
   }
   if(a1){
      stackp->lineno = a1 - bufp->zero;
      stackp->charno = 0;
   }
}
int pchar;
void
setapp(void)
{
   int c;
   c=getchar();
   if(posn(c, lchars)>=0) {
      pchar = c;
      c = getchar();
   }
   if(c==' ' || c=='\t'){
      oneline++;
      if(c=='\t')
         ungetchar(c);
   }
   else if(c!='\n')
      error('x');
}

int
append(int (*f)(void), int *a)
{
   int *a1, *a2, *rdot;
   int nline, tl;
   appflag++;
   nline = 0;
   dot = a;
   while ((*f)()==0) {
      if (lastdol>=endcore)
         morecore(LDCHUNK);
      tl = putline();
      nline++;
      lock++;
      ++dol;
      a1 = ++lastdol;
      fixbufs(1);
      a2 = a1+1;
      rdot = ++dot;
      while (a1 > rdot)
         *--a2 = *--a1;
      *rdot = tl;
      unlock();
      if(oneline)
         break;
   }
   appflag=0;
   oneline=0;
   if(pchar) {
      a1 = addr1; addr1 = dot;
      a2 = addr2; addr2 = dot;
      display(pchar);
      pchar = 0;
      addr1 = a1; addr2 = a2;
   }
   return(nline);
}
char bformat = 'p';
void
bcom(void)
{
   int dir, n;
   int psize;
   int *olddot=addr2;   /* for b. */
   dir=1;
   if((peekc=getchar())!='\n'){   /* really nextchar() */
      if (peekc=='-' || peekc=='+' || peekc=='.') {
         if(peekc=='-')
            dir= -1;
         else if(peekc=='.')
            dir=0;
         getchar();   /* eat peekc */
      }
      if((n=getnum())>0)
         pagesize=n;
   }
   psize=pagesize;
   if(dir==0)
      psize/=2;
   if(posn(nextchar(), lchars)>=0)
      bformat=getchar();
   if(dir<=0) {
      if((addr1=addr2-(psize-1))<=zero)
         addr1=zero+1;
      if(dir==0){
         display(bformat);
         puts("^^^^^");
         addr2++;
      }
   }
   if (dir>=0) {
      addr1=addr2;
      if((addr2+=(psize-1))>dol)
         addr2=dol;
   }
   display(bformat);
   if(dir==0)
      dot=olddot;
}
void
delete(void)
{
   int *a1, *a2, *a3;
   setdot();
   a1 = addr1;
   a2 = addr2;
   if(a1==zero) {
      if(a2!=zero)
         a1++;
      else
         return;      /* 0,$d on an empty buffer */
   }
   a2++;
   a3 = lastdol;
   lock++;
   dol -= a2 - a1;
   lastdol -= a2 - a1;
   fixbufs(a1-a2);
   do
      *a1++ = *a2++;
   while (a2<=a3);
   a1 = addr1;
   if (a1 > dol)
      a1 = dol;
   dot = a1;
   unlock();
   modified();
}
void
allnums(void)
{
   int i;
   char *p;
   setdot();
   for(i=0; i<NBUFS; i++){
      p = string[i].str;
      if(*p!='\0' && alldigs(p)){
         putct(bname[i]);
         puts(p);
      }
   }
}
void
numcom(int z)
{
   int n;
   struct string *sp;
   char *l;
   char c;
   int numeric;
/*
 * Must generate error if attempt is made to do arithmetic on
 * a register which is not numeric; this is done by remembering
 * (via `numeric') whether the initial string was numeric or not.
*/
   sp = &string[z];
   numeric = alldigs(sp->str);
   n = stoi(sp->str);
   for(;;){
      switch(c=getchar()){
      default:
         goto Done;
      case ':':
         n=getsigned();
         goto Not_numeric;
      case 'a':
         n=addr2-zero;
         goto Not_numeric;
      case 'r':
         if(z+1>=NBUFS)
            error('z');
         n = addr1-zero;
         numset(z+1, addr2-zero);
         goto Not_numeric;
      case 'n':
         nonzero();
         l = getline(*addr2,linebuf);
         n = u_count(l);
         goto Not_numeric;
      case 'P':
         n = getpid();
      Not_numeric:
         numeric = TRUE;      /* It's numeric now! */
         break;

      case 'p':
         if(n<0)
            putchar('-');
         putdn(abs(n));
         goto Numeric;
      case '+':
         n += getsigned();
         goto Numeric;
      case '-':
         n -= getsigned();
         goto Numeric;
      case '*':
         n *= getsigned();
         goto Numeric;
      case '/':
         n /= getsigned();
         goto Numeric;
      case '%':
         n %= getsigned();
         goto Numeric;
      case '!':
         if(posn(c=getchar(), "=><")<0)
            error('#');
         settruth(condition(n, getsigned(), c, 1));
         goto Numeric;
      case '<':
      case '>':
      case '=':
         settruth(condition(n, getsigned(), c, 0));
      Numeric:
         if(numeric == FALSE)
            error('#');
         break;
      }
   }
 Done:
   ungetchar(c);
   numset(z, n);
}
int
condition(int n, int  m, int  cond, int  negate)
{
   int retval=0;
   if(cond=='=')
      retval = (n==m);
   else if(cond=='<')
      retval = (n<m);
   else if(cond=='>')
      retval = (n>m);
   else
      error('!');
   return(negate^retval);
}
void
numset(int z, int n)
{
   startstring();
   numbuild(n);
   setstring(z);
}
void
numbuild(int n)
{
   int i;
   if(n<0){
      addstring('-');
      n = -n;
   }
   i = n/10;
   if(i)
      numbuild(i);
   addstring(n%10+'0');
}
void
strcom(int z)
{
   char *q;
   int n,m;
   struct string *sp;
   int cond, c, negate;
   setdot();
   sp = &string[z];
   if(nextchar()=='#'){
      getchar();   /* eat the '#' */
      numcom(z);
      return;
   }
   negate=FALSE;
   switch(cond=getchar()){
   default:
      error('x');
   case 'p':
      puts(sp->str);
      break;
   case 'l':
      putl(sp->str);
      break;
   case ':':
      startstring();
      for(;;){
         c = getquote("\n", getchar);
         if(c=='\n'){
            setstring(z);
            return;
         }
         addstring(UNQUOT(c));
      }
   case '\'':
      startstring();
      dupstring(getname('z'));
      setstring(z);
      break;
   case '.':
      nonzero();
      startstring();
      copystring(getline(*addr2, linebuf));
      setstring(z);
      break;
   case '/':
      nonzero();
      compile('/');
      if(execute(addr2)==0){
         clearstring(z);
         settruth(FALSE);
         return;
      }
      q = loc1;
      startstring();
      while(q < loc2)
         addstring(*q++);
      setstring(z);
      settruth(TRUE);
      return;
   case 's':
      substitute(stackp!=&stack[0], z);
      return;
   case '+':
      strinc(z, getsigned());
      break;
   case '-':
      strinc(z, -getsigned());
      break;
   case 'n':
      setcount(u_count(sp->str));
      break;
   case 'e':
      startstring();
      addstring(lasterr);
      setstring(z);
      break;
   case '(':
      n=getsigned();
      m=u_count(sp->str);
      if(abs(n) > m)
         error('[');
      if(n>=0){
         sp->str = u_nth(sp->str,n);
         sp->len = length(sp->str);
      }
      else{
         sp->str = u_nth(sp->str,m+n);
         sp->len = length(sp->str);
      }
      break;
   case ')':
      n=getsigned();
      m=u_count(sp->str);
      if(abs(n) > m)
         error('[');
      if(n<0){
         *u_nth(sp->str,m+n)='\0';
         sp->len = length(sp->str);
      } else if(n==0)
         clearstring(z);
      else {
         *u_nth(sp->str,n)='\0';
         sp->len = length(sp->str);
      }
      break;
   case '[':
   case ']':
      cpstr(sp->str, linebuf);
      loc2=0;   /* Tell execute that it's already in linebuf */
      compile(getchar());
      if(execute(0)==0){
         settruth(FALSE);
         setcount(-1);
      } else {
         settruth(TRUE);
         if(cond=='[')
            setcount(u_range(linebuf,loc1));
         else
            setcount(u_range(linebuf,loc2)-1);
      }
      return;
   case 'C':
      startstring();
      clean(z);
      setstring(z);
      break;
   case '!':
      if(posn(cond=getchar(), "=><")<0)
         error('x');
      negate=TRUE;
   case '=':
   case '<':
   case '>':
      settruth(FALSE);
      q=sp->str;
      for(;;){
         c=getchar();
         if(c=='\n' && *q==0){
            if(cond == '=')
               settruth(!negate);
            return;
         }
         if(*q++!=c){
            if(cond=='<')
               settruth(negate ^ (*--q < c));
            else if(cond=='>')
               settruth(negate ^ (*--q > c));
            while(c!='\n')
               c=getchar();
            return;
         }
      }
   case '{':
      q = genbuf;
      while(posn(c=getchar(), "} \t\n")<0 && c!=EOF)
         *q++ = c;
      *q = '\0';
      if((q=getenv(genbuf)) == 0)
         clearstring(z);
      else{
         startstring();
         copystring(q);
         setstring(z);
      }
      break;
   /* end of switch */
   }
}
void
strinc(int z, int n)
{
   char uc[4];
   char *q, *p;
   int c,i;
   q=string[z].str;
   startstring();
   while(*q){
      p=uc;
      c=u_code(q);
      i=u_utf8(c+n,p);
      while(i--)
         addstring(*p++);
      u_incr(q);
   }
   setstring(z);
}
int
locn(char *ap, char  *aq)
{
   char *p, *q, *lastq;
   p=ap;
   q=aq;
   for(;;){
      lastq=q;
      while(*p==*q){
         if(*p++==0 || *p==0)
            return(lastq - aq);
         q++;
      }
      if(*q==0)
         return(-1);
      p=ap;
      q=lastq+1;
   }
}
#define ISEMPTY (TRUE+1) /* ==> ignore this buf in G/V */
void
ncom(int c)
{
   struct buffer *bufp;
   struct buffer *stop;
   char *f, *lp;
   int globflag;

   setnoaddr();
   syncbuf();
   globflag = FALSE;
   if(c == 'f')
      stop = bufp = curbuf;
   else{
      bufp = buffer;
      stop = buffer+NBUFS-1;
      if(c=='G')
         globflag = TRUE;
   }
   do{
      lp = linebuf;
      f = string[FILE(bufp-buffer)].str;
      if(c=='N' && (!bufp->cflag || *f==0))
         continue;
      if(bufp->dol==bufp->zero && *f=='\0'){
         if(globflag)
            bufp->gmark = ISEMPTY;
         if(globflag || bufp!=curbuf)
            continue;
      }
      *lp++ = bname[bufp-buffer];
      *lp++ = (bufp->cflag) ? '\'' : ' ';
      *lp++ = (bufp==curbuf) ? '.' : ' ';
      numset(FILEBUF, bufp->dol-bufp->zero);
      cpstr(string[FILEBUF].str, lp);
      lp += string[FILEBUF].len;
      f = string[FILE(bufp-buffer)].str;   /* in case strings got g.c.'d */
      if(*f)
         *lp++ = '\t';
      if(globflag){
         cpstr(f, lp);
         loc2 = 0;   /* ==> we are about to search for 1st time */
         bufp->gmark = execute((int *)0);
      } else {
         *lp = '\0';
         lp = linebuf;
         while(*lp)
            putchar(*lp++);
         putl(f);
      }
   }while(bufp++!=stop);
}
void
allstrs(void)
{
   int i;
   char *p;
   setdot();
   for(i=0; i<NBUFS; i++){
      p=string[i].str;
      if(*p!=0){
         putct(bname[i]);
         putl(p);
      }
   }
   putct('P');
   putl(string[SAVPAT].str);
   putct('R');
   putl(string[SAVRHS].str);
   if(*string[BROWSE].str) {
      putct('B');
      putl(string[BROWSE].str);
   }
}
/*
 *	clean (string) to support zaC
 *	strips leading and trailing white space from a string
 *	and replaces interior white space by single blanks
 */
void
clean(int z)
{
   char *s;
   char *d;
   char c;
   d = genbuf;
   for (s = string[z].str; (c = *s) == ' ' || c == '\t'; s++);
   while ((c = *s++)) {
      if (c == ' ' || c == '\t') {
         *d++ = ' ';
         while(*s == ' ' || *s == '\t')
            s++;
      } else {
         *d++ = c;
      }
   }
   while (d > genbuf && d[-1] == ' ')
      --d;
   *d = 0;
   copystring(genbuf);
}
