#include "mystring.h"

// return the sub-string ending at 'offset'
mystring mystring::left(size_t offset) const
{
  if(offset > rep->length)
    return *this;
  else
    return mystring(rep->buf, offset);
}

// return the sub-string starting at 'offset'
mystring mystring::right(size_t offset) const
{
  if(offset >= rep->length)
    return mystring();
  else if(offset == 0)
    return *this;
  else
    return mystring(rep->buf+offset, rep->length-offset);
}

// return the 'len' characters of the string starting at 'offset'
mystring mystring::sub(size_t offset, size_t len) const
{
  // return right(offset).left(len);
  if(len == 0)
    return mystring();
  else if(offset == 0 && len >= rep->length)
    return *this;
  else {
    if(len+offset >= rep->length)
      len = rep->length - offset;
    return mystring(rep->buf+offset, len);
  }
}

