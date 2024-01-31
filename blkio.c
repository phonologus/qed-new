#include "qed.h"
#define BLKSIZE 512
#ifdef BIGTMP
#define MAXBLOCKS 4095
#else
#define MAXBLOCKS 255
#endif
#define BLMASK MAXBLOCKS
char ibuff[512];
int iblock = -1;
int oblock = 0;
char obuff[512];
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
   int nl;

   bp = (char *)0;
   lp = lbuf;
   nl = -((tl<<1) & 0774);
   tl = (tl>>8) & BLMASK;
   do {
      if (nl<=0) {
         if (tl==oblock)
            bp = obuff;
         else {
            bp = ibuff;
            if (tl!=iblock) {
               iblock = -1;   /* signal protection */
               blkrd(tl, bp);
               iblock = tl;
            }
         }
         tl++;
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
   r = (oblock<<8) + (ooff>>1);   /* ooff may be 512! */
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
   ooff = (((op-obuff)+3)&~3);
   unlock();
   return (r);
}
void
blkrd(int b, char *buf)
{
   if (b>=MAXBLOCKS
   || (lseek(tfile, ((long) b) * 512L, 0)<0L)
   || read(tfile, buf, 512) != 512) {
      error('T');
   }
}
void
blkwr(int b, char *buf)
{
   if (b>=MAXBLOCKS
   || (lseek(tfile, ((long) b) * 512L, 0)<0L)
   || write(tfile, buf, 512) != 512) {
      error('T');
   }
}
