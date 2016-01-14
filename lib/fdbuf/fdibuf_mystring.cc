// Copyright (C) 1999,2000 Bruce Guenter <bruce@untroubled.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include <string.h>
#include "fdbuf.h"
#include "mystring/mystring.h"

bool fdibuf::getline(mystring& out, char terminator)
{
  lock();
  count = 0;
  if(bufstart >= buflength)
    refill();
  if(eof() || error()) {
    unlock();
    return false;
  }
  out = "";
  while(!eof() && !error()) {
    char* ptr = buf+bufstart;
    unsigned bufleft = buflength - bufstart;
    const char* end = (const char*)memchr(ptr, terminator, bufleft);
    if(!end) {
      out += mystring(ptr, bufleft);
      bufstart = buflength;
      count += bufleft;
      refill();
    }
    else {
      unsigned copylen = end - ptr;
      out += mystring(ptr, copylen);
      bufstart += copylen+1;
      count += copylen+1;
      break;
    }
  }
  unlock();
  return true;
}
