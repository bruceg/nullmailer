#include "mystring.h"
#include <ctype.h>

mystring mystring::strip() const
{
  const char* start = rep->buf;
  while(*start && isspace(*start))
    ++start;
  const char* end = rep->buf + rep->length - 1;
  while(end >= start && isspace(*end))
    --end;
  return mystring(start, end-start+1);
}
