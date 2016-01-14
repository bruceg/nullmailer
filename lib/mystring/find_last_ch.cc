#include "mystring.h"

int mystring::find_last(char ch, size_t offset) const
{
  if(offset == (size_t)-1)
    offset = rep->length-1;
  const char* ptr = rep->buf + offset;
  while(ptr >= rep->buf) {
    if(*ptr == ch)
      return ptr - rep->buf;
    --ptr;
  }
  return -1;
}
