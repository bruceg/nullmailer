#include "mystring.h"
#include <string.h>

int mystring::find_first(char ch, size_t offset) const
{
  if(offset >= rep->length)
    return -1;
  char* ptr = strchr(rep->buf+offset, ch);
  return ptr ? ptr-rep->buf : -1;
}

