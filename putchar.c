#include "qed.h"

int col;

void putdn(int i);
void putlong(unsigned long i);
void putl(char *sp);
void puts(char *sp);
void display(int lf);
void putct(int c);
void putchar(char c);
void flush(void);

void
putdn(int i)
{
   putlong((unsigned long)i);
   putchar('\n');
}

void
putlong(unsigned long i)
{
   int r;
   r = i%10;
   i /= 10;
   if(i)
      putlong(i);
   putchar('0'+r);
}

void
putl(char *sp)
{
   listf++;
   puts(sp);
   listf = FALSE;
}
void
puts(char *sp)
{
   col = 0;
   while (*sp)
      putchar(*sp++);
   putchar('\n');
}
void
display(int lf)
{
   ldesc *a1;
   ptrdiff_t r;
   char *p;
   int i;
   int nf;
   listf = (lf == 'l' || lf == 'L');
   nf = (lf == 'P' || lf == 'L');
   lf = listf;
   setdot();
   nonzero();
   a1 = addr1;
   r = a1 - zero;
   do{
      col = 0;
      if(nf){
         putlong(r++);
         for(i=0; i<NBUFS; i++)
            if(a1->ptr == names[i].ptr){
               putchar('\'');
               putchar(bname[i]);
            }
         listf = 0;
         putchar('\t');
         col = TABCHARS;
         listf = lf;
      }
      for(p = getline(*a1++,linebuf);*p;putchar(*p++));
      putchar('\n');
   }while (a1 <= addr2);
   dot = addr2;
   listf = FALSE;
}
void
putct(int c)
{
   putchar(c);
   putchar('\t');
}
void
putchar(char c)
{
   char *lp;

   lp = linp;
   if (listf) {
      if (c=='\n') {
         if(linp!=line && linp[-1]==' ') {
            *lp++ = '\\';
            *lp++ = 'n';
         }
      } else {
         if (col >= (LINECHARS-4-2)) {
            *lp++ = '\\';
            *lp++ = '\n';
            *lp++ = '\t';
            col = TABCHARS;
         }
         col++;
         if (c=='\b' || c=='\\' || c=='\t') {
            *lp++ = '\\';
            c = c=='\b'? 'b' : c=='\t'? 't' : '\\';
            col++;
         } else if ((c&0200) || c<' ' || c=='\177') {
            *lp++ = '\\';
            *lp++ = ((c>>6)&03)+'0';
            *lp++ = ((c>>3)&07)+'0';
            c = (c&07)+'0';
            col += 3;
         }
      }
   }
   *lp++ = c;
   if(c == '\n' || lp >= &line[LINECHARS-2-4]) {
      linp = lp;
      flush();
      lp = linp;
   }
   linp = lp;
}
void
flush(void)
{
   if(linp != line){
      write(1, line, linp-line);
      linp = line;
   }
}
