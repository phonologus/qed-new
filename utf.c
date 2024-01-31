/*
 * UTF-8 
 */

#include "utf.h"

/*
 * lookup table for byte status
 *
 */

static char u8typ_[]={
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
   4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0
};

#define u8typ(c) u8typ_[(unsigned char)(c)]

int u8len(char);
int u8needs(int);
int u8decode(char *);
int u8encode(int, char *);
char *u8next(char *);
char *u8prev(char *);

int u8update(int *a, int c);
int u8bupdate(int *a, int c);
int u8accumulate(int *a, int *u, int c);
int u8baccumulate(int *a, int *u, int c);

/*
 * u8len(char c) returns the number of bytes of UTF-8 byte c promises,
 * or 0 if c is not a valid UTF-8 start byte.
 *
 * Note that no checking is done as to whether the ensuing bytes are
 * actually a valid UTF-8 sequence, or not.
 *
 */

int
u8len(char c)
{
   switch(u8typ(c)){
      case U8ASCII:
         return 1;
      case U82HDR:
         return 2;
      case U83HDR:
         return 3;
      case U84HDR:
         return 4;
   }
   return 0;
}

/*
 * u8needs(int u) returns the number of bytes needed
 * to encode Unicode codepoint u into UTF-8.
 *
 * returns 0 if u is not a valid Unicode codepoint.
 *
 */

int
u8needs(int u)
{
   if(!(u>>=7))
      return 1;      /* needs 1 byte */
   if(!(u>>=4))
      return 2;      /* needs 2 bytes */
   if(!(u>>=5))
      return 3;      /* needs 3 bytes */
   if(!(u>>=5))
      return 4;      /* needs 4 bytes */
   return 0;         /* out of range */
}

/*
 * u8encode(u,p) encodes the Unicode codepoint u into byte array p,
 * and returns the number of bytes used in the encoding, or 0 on error.
 *
 */ 
int
u8encode(int u, char *p)
{
   int n;
   switch (n=u8needs(u)) {
      case 1:
         *p++=(char)u; 
         return n;
      case 2:
         *p++=(char)((0300)|((u>>6)&077));
         goto two;
      case 3:
         *p++=(char)((0340)|((u>>12)&077));
         goto three;
      case 4:
         *p++=(char)((0360)|((u>>18)&077));
         goto four;
      default:
         return n;
   }
   four:
      *p++=(char)((0200)|((u>>12)&077));
   three:
      *p++=(char)((0200)|((u>>6)&077));
   two:
      *p++=(char)((0200)|(u&077));
 
   return n;
}

/*
 * u8decode() returns
 * the Unicode code point encoded by the UTF-8 its argument points
 * to, or -1 if the argument is invalid UTF-8.
 *
 */

int
u8decode(char *p)
{
   char c;
   int u;
   switch (u8typ(c=*p++)) {
      case U8ASCII:
         return c;
      case U82HDR:
         u=c&037;   /* initialise return value */
         goto two;
      case U83HDR:
         u=c&017;   /* initialise the return value */
         goto three;
      case U84HDR:
         u=c&07;    /* initialise the return value */
         goto four;
      default:
         return -1;
   }

   four:
      if(u8typ(c=*p++)!=U8SEQ)
         return -1;
      u=(u<<6) | (c&077);
   three:
      if(u8typ(c=*p++)!=U8SEQ)
         return -1;
      u=(u<<6) | (c&077);
   two:
      if(u8typ(c=*p++)!=U8SEQ)
         return -1;
      u=(u<<6) | (c&077);
 
   return u;
 
}

/*
 * u8next() points to the start of the next character, or
 * NULL if not valid UTF-8
 *
 */

char *
u8next(char *p)
{
   switch (u8typ(*p++)) {
      case U8ASCII:
         return p;             /* ascii byte */
      case U84HDR:
         if(u8typ(*p++)!=U8SEQ)
            return (char *)0;  /* didn't find expected sequence byte */
      case U83HDR:
         if(u8typ(*p++)!=U8SEQ)
            return (char *)0;  /* didn't find expected sequence byte */
      case U82HDR:
         if(u8typ(*p++)!=U8SEQ)
            return (char *)0;  /* didn't find expected sequence byte */
         break;
      default:
         return (char *)0;     /* no valid UTF-8 header byte at p */
   }
   return p;
}

/*
 * u8prev() points the start of the previous character
 *
 */

char *
u8prev(char *p)
{
   switch (u8typ(*--p)) {
      case U8ASCII:
         return p;           /* ascii byte */
      case U8SEQ:
         goto two;
      default:
         return (char *)0;   /* can't find a reverse UTF-8 sequence. */
   }

   two:
   switch (u8typ(*--p)) {
      case U82HDR:
         return p;
      case U8SEQ:
         goto three;
      default:
         return (char *)0;   /* can't find a revers UTF-8 sequence. */
   }

   three:
   switch (u8typ(*--p)) {
      case U83HDR:
         return p;
      case U8SEQ:
         goto four;
      default:
         return (char *)0;   /* can't find a reverse UTF-8 sequence. */
   }

   four:
   switch (u8typ(*--p)) {
      case U84HDR:
         return p;
      default:
         return (char *)0;   /* can't find a reverse UTF-8 sequence. */
   }

   /* not reached */
   return (char *)0;
}

/*
 * u8update(&a,c) takes a pointer to an int (the state) 
 * and a char (in an int), updates the state accordingly:
 * 0 if a valid UTF-8 sequence has been
 * recognised, -1 if a UTF-8 error has occured, or
 * the number of bytes still expected.
 *
 * (we could make the invalid state codes (<0) more informative)
 *
 */

int
u8update(int *a, int c)
{
   switch (u8typ(c)) {
      case U8ASCII:
         return *a = *a>0 ? -1 : 0;    /* ascii byte unexpected : valid */
      case U8SEQ:
         return *a = *a>0 ? *a-1 : -1; /* sequence byte valid : unexpected */
      case U82HDR:
         return *a = *a>0 ? -1 : 1;    /* start of 2-byte unexpected : valid */
      case U83HDR:
         return *a = *a>0 ? -1 : 2;    /* start of 3-byte unexpected : valid */
      case U84HDR:
         return *a = *a>0 ? -1 : 3;    /* start of 4-byte unexpected : valid */
      default:
         return *a=-1;
   }
}

/*
 * u8bupdate(&a,c) takes a pointer to an int (the state) 
 * and a char (in an int), updates the state accordingly:
 * 0 if a valid UTF-8 sequence has been backed over,
 * -1 if a UTF-8 error has occured, or
 * the number of sequence bytes seen. 
 *
 * (we could make the invalid state codes (<0) more informative)
 *
 */

int
u8bupdate(int *a, int c)
{
   switch (u8typ(c)) {
      case U8ASCII:
         return *a = (*a>0) ? -1 : 0;     /* ascii byte unexpected : valid */
      case U8SEQ:
         *a = (*a<0) ? 0 : *a;            /* start (end!) of sequence */
         return *a = (*a<3) ? *a+1 : -1;  /* sequence byte valid : unexpected */
      case U82HDR:
         return *a = (*a==1) ? 0 : -1;    /* 2-byte header valid : unexpected */
      case U83HDR:
         return *a = (*a==2) ? 0 : -1;    /* 3-byte header valid : unexpected */
      case U84HDR:
         return *a = (*a==3) ? 0 : -1;    /* 4-byte header valid : unexpected */
      default:
         return *a=-1;                    /* not a valid UTF-8 byte */
   }
}

/*
 * u8accumulate(&a,&u,c) takes a pointer to an int (the state) 
 * and a char (in an int), updates the state as for u8stat().
 *
 * a running accumulator `u` is updated with the value
 * implied by c. When the state is 0, the accumulator contains
 * a valid unicode codepoint.
 *
 * (we could make the invalid state codes (<0) more informative)
 *
 */

int
u8accumulate(int *a, int *u, int c)
{
   switch (u8typ(c)) {
      case U8ASCII:
         return *a = *a>0 ? -1 : (*u=(unsigned char)c,0); 
      case U8SEQ:
         return *a = *a>0 ? (*u=(*u<<6)|(c&077),*a-1) : (*u=-1);
      case U82HDR:
         return *a = *a>0 ? (*u=-1) : (*u=c&037,1);
      case U83HDR:
         return *a = *a>0 ? (*u=-1) : (*u=c&017,2);
      case U84HDR:
         return *a = *a>0 ? (*u=-1) : (*u=c&07,3);
      default:
         return *a=*u=-1;
   }
}

/*
 * u8baccumulate(&a,&u,c) takes a pointer to an int (the state) 
 * and a char (in an int), updates the state as for u8bstat().
 *
 * a running accumulator `u` is updated with the value
 * implied by c. When the state is 0, the accumulator contains
 * a valid unicode codepoint.
 *
 * (we could make the invalid state codes (<0) more informative)
 *
 */

int
u8baccumulate(int *a, int *u, int c)
{
   switch (u8typ(c)) {
      case U8ASCII:
         return *a = (*a>0) ? (*u=-1) : (*u=(unsigned char)c,0);
      case U8SEQ:
         *a = (*a<=0) ? (*u=0) : *a;
         return *a = (*a<3) ? (*u|=((c&077)<<(*a*6)),*a+1) : (*u=-1);
      case U82HDR:
         return *a = (*a==1) ? (*u|=((c&037)<<6),0) : (*u=-1);
      case U83HDR:
         return *a = (*a==2) ? (*u|=((c&017)<<12),0) : (*u=-1);
      case U84HDR:
         return *a = (*a==3) ? (*u|=((c&07)<<18),0) : (*u=-1);
      default:
         return *a=*u=-1;
   }
}

