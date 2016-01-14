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

#include "mystring.h"

mystring_iter::mystring_iter(const mystring& s, char e)
  : str(s), sep(e), pos(0)
{
  advance();
}

mystring_iter::~mystring_iter()
{
}

void mystring_iter::advance()
{
  if(pos == -1)
    return;
  int i = str.find_first(sep, pos);
  if(i == -1) {
    if(pos >= 0 && pos < (int)str.length()) {
      part = str.right(pos);
      pos = str.length();
    }
    else {
      part = "";
      pos = -1;
    }
  }
  else {
    part = str.sub(pos, i-pos);
    pos = i + 1;
  }
}
