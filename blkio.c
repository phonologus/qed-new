#include "qed.h"
#define BLKSIZE 512
char ibuff[BLKSIZE];
int iblock = -1;
int oblock = 0;
char obuff[BLKSIZE];
int ooff; /* offset of next byte in obuff */

void initio(void);
char * getline(int tl, char *lbuf);
int putline(void);
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
getline(int tl, char *lbuf)
{
   char *bp, *lp;
   int nl,nb;

   bp = (char *)0;
   lp = lbuf;
   nl = -((tl<<1) & (BLKSIZE-1) & ~03);
   nb = tl/(BLKSIZE/2);
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
int
putline(void)
{
   char *op, *lp;
   int r;

   modified();
   lp = linebuf;
   r = (oblock<<8) + (ooff>>1);   /* ooff may be BLKSIZE! */
   op = obuff + ooff;
   lock++;
   do {
      if (op >= obuff+BLKSIZE) {
         /* delay updating oblock until after blkwr succeeds */
         blkwr(oblock, op=obuff);
         oblock++;
         ooff = 0;
      }
      if((*op = *lp++) == '\n') {
         *op++ = '\0';
         linebp = lp;
         break;
      }
   } while (*op++);
   ooff = (((op-obuff)+3)&~03);
   unlock();
   return (r);
}
void
blkrd(int b, char *buf)
{
   if ((lseek(tfile, ((long) b) * BLKSIZE, SEEK_SET)<0L)
   || read(tfile, buf, BLKSIZE) != BLKSIZE) {
      error('T');
   }
}
void
blkwr(int b, char *buf)
{
   if ((lseek(tfile, ((long) b) * BLKSIZE, SEEK_SET)<0L)
   || write(tfile, buf, BLKSIZE) != BLKSIZE) {
      error('T');
   }
}
