#include "mystring.h"
#include "trace.h"
#include <ctype.h>
#include <string.h>

void mystring::dupnil()
{
  trace("");
  rep = &nil;
  rep->attach();
}

void mystring::assign(const char* in)
{
  if(in)
    assign(in, strlen(in));
  else {
    mystringrep* tmp = rep;
    dupnil();
    tmp->detach();
  }
}

void mystring::assign(const char* in, size_t len)
{
  trace("in='" << in << "'");
  if(in != rep->buf) {
    mystringrep* tmp = rep;
    dup(in, len);
    tmp->detach();
  }
}

void mystring::dup(const char* in, size_t len)
{
  trace("in='" << in << "'");
  rep = mystringrep::dup(in, len);
  rep->attach();
}

void mystring::dup(const char* in)
{
  if(in)
    dup(in, strlen(in));
  else
    dupnil();
}

void mystring::operator=(const mystringjoin& in)
{
  mystringrep* tmp = rep;
  rep = in.traverse();
  rep->attach();
  tmp->detach();
}
