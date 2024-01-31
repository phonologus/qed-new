#ifndef UTFINCLUDED
#define UTFINCLUDED

#define U8BYTES 4

#define u8wait(A)    ((A)>0)
#define u8invalid(A) ((A)<0)
#define u8ready(A)   ((A)==0)

enum {
   U8INVALID=0,
   U8ASCII,
   U82HDR,
   U83HDR,
   U84HDR,
   U8SEQ
};

int u8len(char);
int u8decode(char *);
int u8needs(int);
int u8encode(int, char*);
char *u8next(char *);
char *u8prev(char *);

int u8update(int *a, int c);
int u8bupdate(int *a, int c);
int u8accumulate(int *a, int *u, int c);
int u8baccumulate(int *a, int *u, int c);

#endif
