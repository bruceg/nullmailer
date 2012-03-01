#include "mystring.h"
#include <string.h>

int mystring::find_first_of(const char* setstr, size_t setlen,
			    size_t offset) const
{
  for(; offset < rep->length; offset++) {
    if(memchr(setstr, rep->buf[offset], setlen))
      return offset;
  }
  return -1;
}

int mystring::find_first_of(const char* setstr, size_t offset) const
{
  return find_first_of(setstr, strlen(setstr), offset);
}

int mystring::find_first_of(const mystring& setstr, size_t offset) const
{
  return find_first_of(setstr.rep->buf, setstr.rep->length, offset);
}
