#include "qed.h"
char *nextip;
long count;
long bcount;
/*
 * newfile: read a new file name (if given) from command line.
 *
 *	nullerr: if true, error if file is null.
 *	savspec: decides if string[savedfile] is set.
 *		If savspec==SAVEIFFIRST, only save if deffile is null.
 *	deffile: default file name.
 *
 *	return value: string[FILEBUF]!=deffile
 *		      (unless SAVEIFFIRST && deffile null ==> return FALSE)
 *	side effects:
 *		can set string[savedfile]
 *		always sets string[FILEBUF]
 *		zeroes count
 */

int newfile(int nullerr, int savspec, char *deffile);
void exfile(void);
int getfile(void);
void putfile(void);
void Unix(char type);

int
newfile(int nullerr, int savspec, char *deffile)
{
   char c;

   bcount = count = 0L;
   cpstr(deffile, genbuf);   /* in case we strcompact() */
   startstring();
   c = getchar();
   if(c == '\n')
      copystring(genbuf);
   else {
      if(c != ' ')
         error('f');
      do c = getchar(); while(c == ' ');
      while(posn(c, " \t\n") < 0){
         if((c>0 && c<' ') || c=='\177')
            error('f');
         addstring(c);
         c = getchar();
      }
   }
   setstring(FILEBUF);
   if(nullerr && string[FILEBUF].str[0]=='\0')
      error('f');
   if(savspec==SAVEALWAYS || (savspec==SAVEIFFIRST && genbuf[0]=='\0')){
      startstring();
      dupstring(FILEBUF);
      setstring(savedfile);
      if(savspec==SAVEIFFIRST && genbuf[0]=='\0')
         return(FALSE);
   }
   return(!eqstr(genbuf, string[FILEBUF].str));
}
void
exfile(void)
{
   close(io);
   io = -1;
   if (vflag && initflag)
      ncom('f');
   else if (vflag) {
      putlong(count);
      putchar('\n');
   }
   setcount((int)count);
}
int
getfile(void)
{
   int c;
   char *lp, *fp;
   lp = linebuf;
   fp = nextip;
   int utf=0;
   int af=aflag;
   do {
      if (--ninbuf < 0) {
         if ((ninbuf = read(io, genbuf, LBSIZE)-1) < 0) {
            if(ninbuf < -1)
               error('r');
            if(lp != linebuf) {
               puts("!N");
               genbuf[++ninbuf]='\n';
            } else
               return(EOF);
         }
         fp = genbuf;
      }
      if (lp >= &linebuf[LBSIZE])
         error('l');

      if ((*lp++ = c = *fp++ ) == 0) {
         lp--;
         continue;
      }

      bcount++;
      if(af)
         count++;
      else {
         if(u8invalid(u8update(&utf,c))){
            puts("!U");
            af++;
            count=bcount;
         } else if(u8ready(utf))
            count++;
      }

   } while (c != '\n');
   *--lp = 0;
   nextip = fp;
   return(0);
}
void
putfile(void)
{
   ldesc *a1;
   char *fp, *lp;
   int nib;
   nib = 512;
   fp = genbuf;
   int utf=0;
   int af = aflag;
   a1 = addr1;
   if(a1 == zero)
      a1++;
   while(a1 <= addr2){
      lp = getline(*a1++, linebuf);
      for(;;){
         if (--nib < 0) {
            if(write(io, genbuf, fp-genbuf) < 0)
               error('w');
            nib = (sizeof genbuf)-1;
            fp = genbuf;
         }

         bcount++;
         if(af)
            count++;
         else {
            if(u8invalid(u8update(&utf,*lp))){
               puts("!U");
               af++;
               count=bcount;
            } else if(u8ready(utf))
               count++;
         }

         if ((*fp++ = *lp++) == 0) {
            fp[-1] = '\n';
            break;
         }
      }
   }
   write(io, genbuf, fp-genbuf);
}

sig_t savint= SIG_ERR; /* awful; this is known in error() */
void
Unix(char type)
{
   int pid, rpid;
   char *s;
   int c;
   sig_t onbpipe;
   int retcode;
   char unixbuf[512];
   int   pipe1[2];
   int   pipe2[2];
   ldesc  *a, *a1, *a2, *ndot;
   ndot=(ldesc*)0;
   startstring();   /* for the \zU */
   if(type == '!')
      setnoaddr();
   else {
      if(type == '>' || type == '|')
         nonzero();
      bcount = count = 0L;
      if(pipe(pipe1) == -1){
         lastc = '\n';
         error('|');
      }
   }
   /* Quick hack: if char is doubled, push \'zU */
   if(nextchar()==type){
      getchar();   /* throw it away */
      pushinp(STRING, UNIX, TRUE);
   }
	/*
	 * Use c not *s as EOF and getchar() are int's
    */
   for(s=unixbuf;(c=getquote("\n", getsvc))!='\n' && c!=EOF;*s++=UNQUOT(c)){
      if(s>=unixbuf+512)
         error('l');
   }
   dropstring();   /* drop the newline */
   setstring(UNIX);
   *s='\0';
   a1 = addr1;
   a2 = addr2;
   if ((pid = fork()) == 0) {
      signal(SIGHUP, onhup);
      signal(SIGINT, onintr);
      signal(SIGQUIT, onquit);
      if(type=='<' || type=='|'){
         close(1);
         dup(pipe1[1]);
      }else if(type == '>'){
         close(0);
         dup(pipe1[0]);
      }
      if (type != '!') {
         close(pipe1[0]);
         close(pipe1[1]);
      }
      if(type == '|'){
         if(pipe(pipe2) == -1){
            puts("?|");
            exit(1);
         }
         if((pid=fork()) == 0){
            close(1);
            dup(pipe2[1]);
            close(pipe2[0]);
            close(pipe2[1]);
	/*
	 * It's ok if we get SIGPIPE here
         */
            display('p');
            exit(0);
         }
         if(pid == -1){
            puts("Can't fork\n?!");
            exit(1);
         }
         close(0);
         dup(pipe2[0]);
         close(pipe2[0]);
         close(pipe2[1]);
      }
      if(*unixbuf)
         execl("/bin/sh", "sh", "-c", unixbuf, 0);
      else
         execl("/bin/sh", "sh", 0);
      exit(-1);
   }
   if(pid == -1){
      puts("Can't fork");
      error('!');
   }
   savint = signal(SIGINT, SIG_IGN);
   if(type=='<' || type=='|') {
      close(pipe1[1]);
      io = pipe1[0];
      ninbuf = 0;
      append(getfile,addr2);
      close(io);
      io = -1;
      ndot = dot;
   } else if(type == '>') {
      onbpipe = signal(SIGPIPE, SIG_IGN);
      close(pipe1[0]);
      a=addr1;
      do{
         s=getline(*a++, linebuf);
         do; while(*s++);
         *--s='\n';
         if (write(pipe1[1],linebuf,s-linebuf+1)<0){
            puts("?o");
            break;
         }
      }while (a<=addr2);
      close(pipe1[1]);
      signal(SIGPIPE, onbpipe);
   }
   while ((rpid = wait(&retcode)) != pid && rpid != -1);
   retcode = (retcode>>8)&0377;
   settruth(retcode);
   signal(SIGINT, savint);
   if(type == '|'){
      if(retcode == 0){
         addr1 = a1;
         addr2 = a2;
         delete();
         dot = ndot - (a2-a1+1);
      } else
         error('0');
   }
   if(vflag)
      puts("!");
}
