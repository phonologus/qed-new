#include "qed.h"
#define BLKSIZE 512
char ibuff[BLKSIZE];
int iblock = -1;
int oblock = 0;
char obuff[BLKSIZE];
int ooff; /* offset of next byte in obuff */

void initio(void);
char * getline(ldesc tl, char *lbuf);
ldesc putline(void);
void blkrd(int b, char *buf);
void blkwr(int b, char *buf);

void
initio(void)
{
   lock++;
   iblock = -1;
   oblock = 0;
   ooff = 0;
   unlock();
}
char *
getline(ldesc tl, char *lbuf)
{
   char *bp, *lp;
   off_t nl,nb;

   bp = (char *)0;
   lp = lbuf;
   nb = tl.ptr/BLKSIZE;
   nl = -tl.ptr%BLKSIZE; /* neg to guarantee nl<=0 first time through loop */
   do {
      if (nl<=0) {
         if (nb==oblock)
            bp = obuff;
         else {
            bp = ibuff;
            if (nb!=iblock) {
               iblock = -1;   /* signal protection */
               blkrd(nb, bp);
               iblock = nb;
            }
         }
         nb++;
         bp -= nl;
         nl += BLKSIZE;
      }
      nl--;
   } while ((*lp++ = *bp++));
   return(lbuf);
}
ldesc
putline(void)
{
   char *op, *lp;
   ldesc r;

   modified();
   lp = linebuf;
   r.ptr = (oblock*BLKSIZE) + ooff;
   r.flags = 0;
   op = obuff + ooff;
   lock++;
   do {
      if (op >= obuff+BLKSIZE) {
         /* delay updating oblock until after blkwr succeeds */
         blkwr(oblock, obuff);
         oblock++;
         op=obuff;
         ooff = 0;
      }
      if((*op = *lp++) == '\n') {
         *op++ = '\0';
         linebp = lp;
         break;
      }
   } while (*op++);
   ooff = (int)(op-obuff);
   unlock();
   return (r);
}
void
blkrd(int b, char *buf)
{
   if ((lseek(tfile, (off_t)b*BLKSIZE, SEEK_SET)<0L)
   || read(tfile, buf, BLKSIZE) != BLKSIZE) {
      error('T');
   }
}
void
blkwr(int b, char *buf)
{
   if ((lseek(tfile, (off_t)b*BLKSIZE, SEEK_SET)<0L)
   || write(tfile, buf, BLKSIZE) != BLKSIZE) {
      error('T');
   }
}
