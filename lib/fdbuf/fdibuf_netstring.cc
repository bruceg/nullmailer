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

bool fdibuf::getnetstring(mystring& out)
{
  // Read in the size
  char ch;
  unsigned long size = 0;
  for(;;) {
    if(!get(ch))
      return false;
    if(ch == ':')
      break;
    else if(ch >= '0' && ch <= '9')
      size = size*10 + (ch-'0');
    else
      return false;
  }
  char tmp[size];
  if(!read(tmp, size) || !get(ch) || ch != ',')
    return false;
  out = mystring(tmp, size);
  return true;
}
