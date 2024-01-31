#include "qed.h"

int u_length(char c);
char *u_incrp(char **p);   /* ++p */
char *u_decrp(char **p);   /* --P */
char *u_postincrp(char **p); /* p++ */
char *u_postdecrp(char **p); /* p-- */
int u_code(char *p);
int u_utf8(int c,char *p);
int u_count(char *p);
char *u_nth(char *p, int i);
int u_range(char *a, char *b);

void u_loadp(char **p, int c, char **q);
void u_loadfp(char **p, int c, int (*f)(void));
int u_loadcp(char **p, int c, char **q);
int u_loadcfp(char **p, int c, int (*f)(void));

int
u_length(char c)
{
   int n;
   if(aflag)
      return 1;
   if(!(n=u8len(c)))
      error('U');
   return n;
}

char *
u_incrp(char **p)
{
   if(aflag)
      ++*p;
   else if(!(*p=u8next(*p)))
      error('U');
   return *p;
}

char *
u_decrp(char **p)
{
   if(aflag)
      --*p;
   else if(!(*p=u8prev(*p)))
      error('U');
   return *p;
}

char *
u_postincrp(char **p)
{
   char *r = *p;
   u_incrp(p);
   return r;
}

char *
u_postdecrp(char **p)
{
   char *r = *p;
   u_decrp(p);
   return r;
}

int
u_code(char *p)
{
   int c;
   if(aflag)
      return (unsigned char)*p;
   if((c=u8decode(p))<0)
      error('U');
   return c;
}

int
u_utf8(int c, char *p)
{
   int n;
   if(aflag){
      *p=c;
      return 1;
   }
   if(!(n=u8encode(c,p)))
      error('U');

   return n;
}

int
u_count(char *p)
{
   int n=0;
   if(aflag)
      while(*p++)
         ++n;
   else
      while(*p){
         ++n;
         if(!(p=u8next(p)))
            error('U');
      }
   return n;
}

char*
u_nth(char *p, int i)
{
   if(aflag)
      while(*p && i-->0)
         ++p;
   else
      while(*p && i-->0)
         if(!(p=u8next(p)))
            error('U');
   return p;
}

void
u_loadp(char **p, int c, char **q)
{
   int utf=0;
   if(aflag)
      *(*p)++=c;
   else{
      while(u8wait(u8update(&utf,c))){
         *(*p)++=c;
         c=*(*q)++; 
      }
      if(u8ready(utf))
         *(*p)++=c;
      else
         error('U');
   }
}

void
u_loadfp(char **p, int c, int (*f)(void))
{
   int utf=0;
   if(aflag)
      *(*p)++=c;
   else{
      while(u8wait(u8update(&utf,c))){
         *(*p)++=c;
         c=f(); 
      }
      if(u8ready(utf))
         *(*p)++=c;
      else
         error('U');
   }
}

int
u_loadcp(char **p, int c, char **q)
{
   int u=0,utf=0;
   if(aflag)
      *(*p)++=u=c;
   else {
      while(u8wait(u8accumulate(&utf,&u,c))){
         *(*p)++=c;
         c=*(*q)++; 
      }
      if(u8ready(utf))
         *(*p)++=c;
      else
         error('U');
   }
   return u;
}

int
u_loadcfp(char **p, int c, int (*f)(void))
{
   int u=0,utf=0;
   if(aflag)
      *(*p)++=u=c;
   else {
      while(u8wait(u8accumulate(&utf,&u,c))){
         *(*p)++=c;
         c=f(); 
      }
      if(u8ready(utf))
         *(*p)++=c;
      else
         error('U');
   }
   return u;
}

int
u_range(char *a,char *b)
{
   int n=0;
   if(a>b)
      error('U');
   if(aflag)
      return (int)(b-a);
   else
      while(*a && a<b){
         ++n;
         if(!(a=u8next(a)))
            error('U');
      }
   if(a>b)
      error('U');
   return n;
}

