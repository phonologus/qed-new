#include "bytes.h"

/* read integer value from byte array */

/* Big-endian */

unsigned long get_uval(int bc, bytes bs)
{
  
  unsigned long r ;

  r = 0;

  while (bc-- > 0) 
    r |= ((unsigned long) *bs++) << (bc << 3) ;
  
  return r ; 

}

long get_val(int bc, bytes bs)
{
  
  unsigned long r ;

  unsigned long sign ; 

  r = (unsigned long) *bs++ ;

  sign = (r & 0x80) << ((--bc) << 3);

  r = (r & 0x7F) << (bc << 3) ;

  while (bc-- > 0) 
    r |= ((unsigned long)*bs++) << (bc << 3) ;

  return r - sign ; 

}

/* Little endian */

unsigned long get_uval_le(int bc, bytes bs)
{
  
  unsigned long r ;

  r = 0;

  bs += bc ;

  while (bc-- > 0) 
    r |= ((unsigned long) *--bs) << (bc << 3) ;
  
  return r ; 

}

long get_val_le(int bc, bytes bs)
{
  
  unsigned long r ;

  unsigned long sign ; 

  bs += bc ;

  r = (unsigned long) *--bs ;

  sign = (r & 0x80) << ((--bc) << 3);

  r = (r & 0x7F) << (bc << 3) ;

  while (bc-- > 0) 
    r |= ((unsigned long)*--bs) << (bc << 3) ;

  return r - sign ; 

}

/* write integer value to byte array */

void set_uval(unsigned long v, int bc, bytes b)
{

  b += bc ;

  while (bc-- > 0) {
    *--b = (byte) (v & 0xFF) ; 
    v >>= 8 ;
  }

  return ;

}

void set_val(long v, int bc, bytes b)
{

  unsigned long val ; 

  if (v < 0) {
    val = 1 + ~(-v);
  } else {
    val = v ;
  }

  b += bc ;

  while (bc-- > 0) {
    *--b = (byte) (val & 0xFF) ; 
    val >>= 8 ;
  }

  return ;

}


void set_uval_le(unsigned long v, int bc, bytes b)
{

  while (bc-- > 0) {
    *b++ = (byte) (v & 0xFF) ; 
    v >>= 8 ;
  }

  return ;

}


void set_val_le(long v, int bc, bytes b)
{

  unsigned long val ; 

  if (v < 0) {
    val = 1 + ~(-v);
  } else {
    val = v ;
  }

  while (bc-- > 0) {
    *b++ = (byte) (val & 0xFF) ; 
    val >>= 8 ;
  }

  return ;

}
