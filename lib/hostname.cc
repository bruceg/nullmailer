#include "config.h"
#include "mystring.h"
#include <unistd.h>
#include <sys/utsname.h>
#include "fdbuf.h"

static mystring* hostname_cache = 0;
static mystring* domainname_cache = 0;

static void getnames()
{
  if(hostname_cache)
    return;
  struct utsname buf;
  uname(&buf);
  hostname_cache = new mystring(buf.nodename);
  int i = hostname_cache->find_first('.');
  if(i != -1)
    domainname_cache = new mystring(hostname_cache->right(i+1));
  else
    domainname_cache = new mystring;
}

mystring hostname()
{
  getnames();
  return *hostname_cache;
}

mystring domainname()
{
  getnames();
  return *domainname_cache;
}
