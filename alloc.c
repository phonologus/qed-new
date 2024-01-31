#include "qed.h"

void *qlloc(int);
void *reqlloc(void *, int);

void *
qlloc(int n)
{
   void *p;
   if((p=malloc(n))==NULL)
      error('c');
   memset(p,0,n);
   return p;
}

void *
reqlloc(void *p, int n)
{
   void *r;
   if((r=realloc(p,n))==NULL)
      error('c');
   return r;
}
