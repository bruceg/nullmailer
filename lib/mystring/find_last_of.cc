#include "mystring.h"
#include <string.h>

int mystring::find_last_of(const char* setstr, size_t setlen,
			   size_t offset) const
{
  if(offset == (size_t)-1)
    offset = rep->length-1;
  for(int i = offset; i >= 0; --i) {
    if(memchr(setstr, rep->buf[i], setlen))
      return i;
  }
  return -1;
}

int mystring::find_last_of(const char* setstr, size_t offset) const
{
  return find_last_of(setstr, strlen(setstr), offset);
}

int mystring::find_last_of(const mystring& setstr, size_t offset) const
{
  return find_last_of(setstr.rep->buf, setstr.rep->length, offset);
}
