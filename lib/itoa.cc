#include "itoa.h"

const char *itoa(long v, int digits)
{
  static char buf[INTLENGTH];
  bool neg = false;
  if(v < 0) {
    v = -v;
    neg = true;
  }
  char* ptr = buf + INTLENGTH;
  *--ptr = '\0';
  do {
    *--ptr = (v % 10) + '0';
    v /= 10;
    --digits;
  } while(v != 0);
  while(digits > 0 && ptr >= buf)
    *--ptr = '0', --digits;
  if(neg)
    *--ptr = '-';
  return ptr;
}
