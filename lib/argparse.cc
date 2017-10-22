#include <ctype.h>
#include "argparse.h"

static const char* parse_arg(mystring& arg, const char* start, const char* end)
{
  const char* ptr;
  for (ptr = start; ptr < end && ! isspace(*ptr); ++ptr) {
    switch (*ptr) {
    case '\'':
      arg.append(start, ptr - start);
      for (start = ++ptr; ptr < end && *ptr != '\''; ++ptr) ;
      arg.append(start, ptr - start);
      start = ptr + 1;
      continue;
    case '"':
      arg.append(start, ptr - start);
      for (start = ++ptr; ptr < end && *ptr != '\"'; ptr++) {
	if (*ptr == '\\') {
	  arg.append(start, ptr - start);
	  if (++ptr < end)
	    arg.append(ptr, 1);
	  start = ++ptr;
	}
      }
      arg.append(start, ptr - start);
      start = ptr + 1;
      continue;
    case '\\':
      arg.append(start, ptr - start);
      if (++ptr < end)
	arg.append(ptr, 1);
      start = ++ptr;
      continue;
    }
  }
  if ((ptr - start) > 0)
    arg.append(start, ptr - start);
  return ptr;
}

unsigned parse_args(arglist& lst, const mystring& str)
{
  lst.empty();
  const char* ptr = str.c_str();
  const char* end = ptr + str.length();
  unsigned count = 0;
  while (ptr < end) {
    // Skip any leading spaces
    if (isspace(*ptr))
      ++ptr;
    else {
      mystring s;
      ptr = parse_arg(s, ptr, end);
      if (ptr == 0)
	break;
      lst.append(s);
      ++count;
    }
  }
  return count;
}
