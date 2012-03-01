#include "mystring.h"
#include <ctype.h>

mystring mystring::lstrip() const
{
  const char* ptr = rep->buf;
  while(*ptr && isspace(*ptr))
    ++ptr;
  return ptr;
}
