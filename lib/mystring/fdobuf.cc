#include "mystring.h"
#include "fdbuf/fdbuf.h"

fdobuf& operator<<(fdobuf& out, const mystring& str)
{
  out.write(str.c_str(), str.length());
  return out;
}

