#include "qed.h"

#define ESIZE 128 /* ESIZE-1 must fit in a signed int byte */
char expbuf[ESIZE+4];
int expgood /*0*/; /* flag indicating if compiled exp is good */
#define CCHR 2
#define CDOT 4
#define CCL 6
#define NCCL 8
#define CFUNNY 10
#define CALT 12
#define CBACK 14

#define STAR 01
#define STARABLE CBACK

#define CKET 16
#define CDOL 17
#define CENF 18
#define CBRA 19
#define CBOI 20
#define CEOI 21
#define CSPACE 22

#define DITTO '\370'  /* must be an invalid UTF-8 byte */

int circfl;
char pmagic[] = "/.$^*+\\()<|>{}[!_123456789";

void compile(char eof);
int getsvc(void);
int execute(ldesc *addr);
int advance(char *lp, char  *ep);
int backref(int i, char *lp);
int alfmatch(char c, int tail);
int cclass(char *set, int c, int f);

void
compile(char eof)
{
   int c;
   int a,b;
   char *ep, *penultep;
   char *lastep, *bracketp, bracket[NBRA];
   struct{
      char   *althd;      /* start of code for < ... > */
      char   *altlast;   /* start of code for last < or | */
      char   *bpstart;   /* bracketp at start of < and | */
      char   *bpend;      /* bracketp at end of > or | */
      int   nbstart;   /* nbra at start of < and | */
      int   nbend;      /* nbra at end of > or | */
      int   firstalt;   /* is this the first alternative? */
   } *asp, altstk[NBRA];

   if(eof == '\n')
      error('x');
   pmagic[0] = eof;
   if ((c=nextchar()) == eof || c=='\n') {
      if (!expgood)
         goto cerror;
      if(c!='\n')
         getchar();   /* eat the eof character */
      return;
   }
   expgood = FALSE;
   ep = expbuf;
   lastep = 0;
   bracketp = bracket;
   nbra = 0;
   asp = &altstk[0]-1;
   startstring();   /* for the saved pattern */
   circfl = 0;
   if (c=='^') {
      getsvc();   /* save the caret */
      circfl++;
   }
   for (;;) {
      c = getquote(pmagic, getsvc);
      if (c==eof || c=='\n') {
         if (bracketp!=bracket || asp>=altstk)
            goto cerror;
         *ep++ = CENF;
         expgood = TRUE;
         dropstring();   /* lose the eof character */
         setstring(SAVPAT);
         if(c=='\n')
            ungetchar(c);
         return;
      }
      if (ep >= &expbuf[ESIZE-11]) /* starting a CCL can add upto 10 bytes */
         goto cerror;
      penultep = lastep;
      lastep = ep;

      if(c != QUOT(eof)) switch (c) {
      case QUOT('('):
         if (nbra >= NBRA)
            goto cerror;
         *bracketp++ = nbra;
         *ep++ = CBRA;
         *ep++ = nbra++;
         continue;
      case QUOT(')'):
         if (bracketp <= bracket)
            goto cerror;
         *ep++ = CKET;
         *ep++ = *--bracketp;
         continue;
      case QUOT('{'):
         *ep++ = CBOI;
         continue;
      case QUOT('}'):
         *ep++ = CEOI;
         continue;
      case QUOT('_'):
         *ep++ = CSPACE;
         continue;
      case QUOT('!'):
         *ep++ = CFUNNY;
         continue;
      case '<':
         if (++asp >= &altstk[NBRA])
            goto cerror;
         *ep++ = CALT;
         asp->althd = ep;
         ep++;
         asp->bpstart = bracketp;
         asp->nbstart = nbra;
         asp->firstalt = TRUE;
         asp->altlast = ep++;
         lastep = 0;
         continue;
      case '|':
         if (asp<altstk)
            break;
         if (asp->firstalt) {
            asp->bpend = bracketp;
            asp->nbend = nbra;
         }
         if (bracketp!=asp->bpend || nbra!=asp->nbend)
            goto cerror;
         *ep++ = CENF;
         asp->altlast[0] = ep-asp->altlast;
         asp->firstalt = FALSE;
         bracketp = asp->bpstart;
         nbra = asp->nbstart;
         asp->altlast = ep++;
         lastep = 0;
         continue;
      case '>':
         if (asp<altstk)
            break;
         if (!asp->firstalt &&
          (bracketp!=asp->bpend || nbra!=asp->nbend))
            goto cerror;
         *ep++ = CENF;
         asp->altlast[0] = ep-asp->altlast;
         lastep = asp->althd;
         *lastep = ep-lastep;
         lastep--;
         if (bracketp!=asp->bpstart || nbra!=asp->nbstart)
            lastep = 0;
         asp--;
         continue;
      case '*':
      case '+':
         if (penultep==0){
            *ep++ = CCHR;
            *ep++ = c;
         } else {
            if(*penultep>STARABLE)
               goto cerror;
            if(c == '+'){
               if((ep-penultep)+ep >= &expbuf[ESIZE-1])
                  goto cerror;
               do
                  *ep++ = *penultep++;
               while (penultep!=lastep);
            }
            *penultep |= STAR;
            lastep = 0;
         }
         continue;
      case '.':
         *ep++ = CDOT;
         continue;

      case '[':
         penultep = ep;
         *ep++ = CCL;
         *ep++ = 0;
         if ((c=getsvc()) == '^') {
            c = getsvc();
            ep[-2] = NCCL;
         }
         do {
            if (c == EOF || c == '\n')
               goto cerror;
            a=b=u_loadcf(ep,c,getsvc);
            *ep++ = DITTO;
            if ((lastc=getsvc()) == '-') {
               c=getsvc();
               if (c == EOF || c == '\n')
                  goto cerror;
               --ep; /* eat the prior DITTO */
               b=u_loadcf(ep,c,getsvc);
               lastc = getsvc();   /* prime lastc */
            } else if (dflag&&'a'<=(c|' ')&&(c|' ')<='z'){
               *ep++ = c^' ';
               *ep++ = DITTO;
            }
            if(a>b)
               goto cerror;
            if (ep >= &expbuf[ESIZE-1])
               goto cerror;
         } while ((c=lastc) != ']');
         penultep[1] = ep-penultep-1;
         continue;

      case '$':
         if (nextchar() == eof || peekc=='\n') {
            *ep++ = CDOL;
            continue;
         }
         /* fall through */
      default:
         break;
      }
      /* if fell through switch, match literal character */
      /* Goddamned sign extension! */
      if (QUOTED(c) && UNQUOT(c)>='1' && UNQUOT(c)<='9') {
         *ep++ = CBACK;
         *ep++ = UNQUOT(c)-'1';
         continue;
      }
      c = UNQUOT(c);
      if(dflag && (c|' ')>='a' && (c|' ')<='z'){
         *ep++ = CCL;
         *ep++ = 5;
         *ep++ = c;
         *ep++ = DITTO;
         *ep++ = c^' ';
         *ep++ = DITTO;
      }else{
         *ep++ = CCHR;
         u_loadf(ep,c,getsvc);
      }
   }
 cerror:
   error('p');
}
int
getsvc(void)
{
   int c;
   addstring(c=getchar());
   return(c);
}
int
execute(ldesc *addr)
{
   char *p1, *p2;

   if (addr==0) {
      if((p1=loc2) == 0)   /* G command */
         p1 = linebuf;
      else if (circfl)   /* not first search in substitute */
         return(FALSE);
   } else {
      if (addr==zero)
         return(FALSE);
      p1 = getline(*addr, linebuf);
   }
   p2 = expbuf;
   if (circfl) {
      loc1 = p1;
      return(advance(p1, p2));
   }
   do {
      if (*p2 != CCHR || p2[1] == *p1) {
         if (advance(p1, p2)) {
            loc1 = p1;
            return(TRUE);
         }
      }
   } while (*u_postincr(p1));
   return(FALSE);
}

int
advance(char *lp, char  *ep)
{
   int u;
   char *curlp;
   char *althd, *altend;

   for (;;) {
      curlp = lp;
      switch (*ep++) {

      case CCHR:
         if(u_code(u_postincr(ep))==u_code(u_postincr(lp)))
            continue;
         return(FALSE);

      case CCHR|STAR:
         u=u_code(ep);
         do ; while(u_code(u_postincr(lp)) == u);
         u_incr(ep);
         break;

      case CDOT:
         if(*u_postincr(lp))
            continue;
         return(FALSE);

      case CDOT|STAR:
         do ; while(*u_postincr(lp));
         break;

      case CCL:
      case NCCL:
         if (cclass(ep, u_code(u_postincr(lp)), ep[-1]==CCL)) {
            ep += *ep;
            continue;
         }
         return(FALSE);

      case CCL|STAR:
      case NCCL|STAR:
         do ; while (cclass(ep, u_code(u_postincr(lp)), ep[-1]==(CCL|STAR)));
         ep += *ep;
         break;

      case CFUNNY:
         if ((*lp>=' ' && *lp!='\177') || *lp=='\t' || *lp=='\0')
            return(FALSE);
         lp++;
         continue;

      case CFUNNY|STAR:
         while ((*lp<' ' && *lp>0 && *lp!='\t') || *lp=='\177')
            lp++;
         lp++;
         break;

      case CBACK:
         if (braelist[(int)*ep]==0)
            error('p');
         if (backref(*ep++, lp)) {
            lp += braelist[(int)ep[-1]] - braslist[(int)ep[-1]];
            continue;
         }
         return(FALSE);
   
      case CBACK|STAR:
         if (braelist[(int)*ep] == 0)
            error('p');
         curlp = lp;
         while (backref(*ep, lp))
            lp += braelist[(int)*ep] - braslist[(int)*ep];
         while (lp >= curlp) {
            if (advance(lp, ep+1))
               return(TRUE);
            lp -= braelist[(int)*ep] - braslist[(int)*ep];
         }
         ep++;
         continue;

      case CBRA:
         braslist[(int)*ep++] = lp;
         continue;

      case CKET:
         braelist[(int)*ep++] = lp;
         continue;

      case CDOL:
         if (*lp==0)
            continue;
         return(FALSE);

      case CENF:
         loc2 = lp;
         return(TRUE);

      case CBOI:
         if (alfmatch(*lp,0)
          && (lp==linebuf || !alfmatch(lp[-1],1)))
            continue;
         return(FALSE);

      case CEOI:
         if (!alfmatch(*lp,1)
          && lp!=linebuf && alfmatch(lp[-1],1))
            continue;
         return(FALSE);

      case CSPACE:
         if (*lp==' ' || *lp=='\t') {
            while (*lp == ' ' || *lp=='\t')
               lp++;
            continue;
            }
         return(FALSE);

      case CALT:
         althd = ep-1;
         altend = ep + *ep;
         for(ep++; ; ep+= *ep) {
            if(ep == altend)
               return(FALSE);
            if(advance(lp,ep+1) && advance(loc2,altend))
               return(TRUE);
         }

      case CALT|STAR:
         althd = ep-1;
         altend = ep + *ep;
         for(ep++; ep!=altend; ep+= *ep){
            if(advance(lp, ep+1)){
               if(loc2 == lp)
                  break;
               if(advance(loc2, althd))
                  return(TRUE);
            }
         }
         /* return (advance(lp,altend)) */
         continue;

      default:
         error('!');
      }
      /* star logic: executed by falling out of switch */
      do {
         u_decr(lp);
         if (advance(lp, ep))
            return(TRUE);
      } while (lp > curlp);
      return(FALSE);
   }
}

int
backref(int i, char *lp)
{
   char *bp;

   bp = braslist[i];
   while (*bp++ == *lp++)
      if (bp >= braelist[i])
         return(TRUE);
   return(FALSE);
}
int
alfmatch(char c, int tail)
{
   return (('a' <= c && c <= 'z') ||
      ('A' <= c && c <= 'Z') ||
      (c == '_') ||
      (tail && '0' <= c && c<= '9'));
}


int
cclass(char *set, int c, int f)
{
   int a,b;
   char *end;
   if (c == 0)
      return(0);
   end = set + *set;
   ++set;
   while (set<end) {
      a=u_code(set);
      u_incr(set);
      if(*set==DITTO){
         b=a;
         ++set;
      }else{
         b=u_code(set);
         u_incr(set);
      }
      if(a <= c && c <= b)
         return(f);
   }
   return(!f);
}
