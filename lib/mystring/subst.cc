#include "mystring.h"

mystring mystring::subst(char from, char to) const
{
  const unsigned length = rep->length;
  char buf[length+1];
  const char* in = rep->buf + length;
  bool changed = true;
  for(char* out = buf+length; out >= buf; in--, out--)
    if(*in == from)
      *out = to, changed = true;
    else
      *out = *in;
  if(!changed)
    return *this;
  else
    return mystring(buf, length);
}
