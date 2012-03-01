#include "mystring.h"
#include <ctype.h>

istream& operator>>(istream& in, mystring& str)
{
  str = "";
  char buf[256];		// buffer this many characters at a time
  unsigned i = 0;
  in >> ws;			// skip leading whitespace
  char ch;
  while((ch = in.get()) != EOF) {
    if(isspace(ch)) {		// end the input on whitespace
      buf[i] = 0;
      str += buf;
      i = 0;
      break;
    }
    else {
      buf[i++] = ch;
      if(i == sizeof(buf)-1) {	// append the filled buffer and empty it
	buf[i] = 0;
	str += buf;
	i = 0;
      }
    }
  }
  if(i > 0) {			// If EOF was reached before whitespace,
    buf[i] = 0;			// append the buffer to the string.
    str += buf;
  } else if(str.length() == 0)	// Mark failure if no string was read.
    in.set(ios::failbit);
  return in;
}
