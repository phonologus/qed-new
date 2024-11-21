#ifndef BYTES
#define BYTES

typedef unsigned char byte ;
typedef byte * bytes ;

unsigned long get_uval(int bc, bytes bs) ;
long val(int bc, bytes bs) ;
unsigned long get_uval_le(int bc, bytes bs) ;
long get_val_le(int bc, bytes bs) ;

void set_uval(unsigned long v, int bc, bytes b) ;
void set_val(long v, int bc, bytes b) ;
void set_uval_le(unsigned long v, int bc, bytes b) ;
void set_val_le(long v, int bc, bytes b) ;

#endif
