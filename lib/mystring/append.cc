#include <string.h>
#include "mystring.h"
#include "trace.h"

void mystring::append(const char* str, size_t len)
{
  if(!str || !len)
    return;
  if(!*this)
    assign(str, len);
  else
    rep = rep->append(str, len);
}

void mystring::append(const char* in)
{
  if(in)
    append(in, strlen(in));
}
