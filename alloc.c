#include "qed.h"

void *qlloc(size_t);
void *reqlloc(void *, size_t);

void *
qlloc(size_t n)
{
   void *p;
   if((p=malloc(n))==NULL)
      error('c');
   memset(p,0,n);
   return p;
}

void *
reqlloc(void *p, size_t n)
{
   void *r;
   if((r=realloc(p,n))==NULL)
      error('c');
   return r;
}
