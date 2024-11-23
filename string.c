#include "qed.h"
#define strfree string[NSTRING].str
char *strstart;


size_t length(char *s);
void startstring(void);
void addstring(int c);
void dropstring(void);
void cpstr(char *a, char  *b);
void shiftstring(int up);
void clearstring(int z);
void copystring(char *s);
int eqstr(char *a, char  *b);
void dupstring(int z);
void setstring(int z);
void strcompact(void);

size_t
length(char *s)
{
   char *t;
   if((t=s)==0)
      return(0);
   do;while(*t++);
   return(t-s-1);
}
void
startstring(void)
{
   strstart=strfree;
}
void
addstring(int c)
{
   if(strfree==strarea+nstrarea){
      strcompact();
   }
   *strfree++ = c;
}
void
dropstring(void)
{
   --strfree;
}
void
cpstr(char *a, char  *b)
{
   do;while ((*b++ = *a++));
}
void
shiftstring(int up)
{
   struct string *sp;
   for(sp=string; sp<=string+NSTRING; sp++)
      if(up)
         sp->str += (ptrdiff_t)strarea;
      else
         sp->str -= (ptrdiff_t)strarea;
   if(up)
      strstart += (ptrdiff_t)strarea;
   else
      strstart -= (ptrdiff_t)strarea;
}
void
clearstring(int z)
{
   string[z].len = 0;
   string[z].str = nullstr;
}
void
copystring(char *s)
{
   while(*s)
      addstring(*s++);
}
int
eqstr(char *a, char  *b)
{
   while(*a)
      if(*a++ != *b++)
         return(FALSE);
   return(*b=='\0');
}
/*
 * dupstring duplicates a string.
 * Because we may strcompact(), we do it first if necessary.
 */
void
dupstring(int z)
{
   if(strfree+string[z].len > strarea+nstrarea)
      strcompact();   /* if insufficient, will get error when we copystring() */
   copystring(string[z].str);
}
void
setstring(int z)
{
   addstring('\0');
   if((string[z].len = length(strstart)) == 0)
      string[z].str = nullstr;
   else
      string[z].str = strstart;
   if(strfree >= strarea + nstrarea){
      strcompact();
   }
}
void
strcompact(void)
{
   struct string *cursor;
   struct string *thisstr=0;
   char *s, *t;
   lock++;
   s=strchars;
   for(;;){
      t=strarea+nstrarea;
      for(cursor=string;cursor!=string+NSTRING;cursor++)
         if(s<=cursor->str && cursor->str<t){
            t = cursor->str;
            thisstr = cursor;
         }
      if(t==strarea+nstrarea)
         break;
      thisstr->str=s;
      do;while((*s++ = *t++));
   }
   t=strstart;
   strstart=s;
   while(t!=strfree)
      *s++ = *t++;
   if(s==strarea+nstrarea)
      morestrarea(); /* was error('Z'); */
   else
      strfree=s;
   unlock();
}
