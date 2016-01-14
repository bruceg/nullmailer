#include "mystring.h"
#include <ctype.h>

mystring mystring::rstrip() const
{
  const char* ptr = rep->buf + rep->length - 1;
  while(ptr >= rep->buf && isspace(*ptr))
    --ptr;
  return mystring(rep->buf, ptr-rep->buf+1);
}
