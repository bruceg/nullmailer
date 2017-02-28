#include "mystring.h"
#include <string.h>

bool mystring::starts_with(const char* that, size_t len) const
{
  return len <= rep->length && memcmp(that, rep->buf, len) == 0;
}

bool mystring::starts_with(const char* that) const
{
  return starts_with(that, strlen(that));
}

bool mystring::starts_with(const mystring& that) const
{
  return starts_with(that.rep->buf, that.rep->length);
}
