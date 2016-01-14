#include "mystring.h"
#include "trace.h"
#include <ctype.h>
#include <string.h>

#ifdef MYSTRING_TRACE
mystring::~mystring()
{
  trace("rep=" << (void*)rep);
  rep->detach();
}
#endif

int mystring::operator!=(const char* in) const
{
  if(rep->buf == in)
    return 0;
  return strcmp(rep->buf, in);
}

int mystring::operator!=(const mystring& in) const
{
  if(rep->buf == in.rep->buf)
    return 0;
  return strcmp(rep->buf, in.rep->buf);
}

const mystring mystring::NUL("", 1);
