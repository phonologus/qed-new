#ifndef LINEINCLUDED
#define LINEINCLUDED

/*
 * the line descriptor type
 */

struct ldesc {
   off_t ptr;
   unsigned char flags;
};

typedef struct ldesc ldesc;

#endif
