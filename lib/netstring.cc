#include "config.h"
#include "netstring.h"
#include "itoa.h"

mystring str2net(const mystring& s)
{
  return mystringjoin(itoa(s.length())) + ":" + s + ",";
}

mystring strnl2net(const mystring& s)
{
  return mystringjoin(itoa(s.length()+1)) + ":" + s + "\012,";
}
