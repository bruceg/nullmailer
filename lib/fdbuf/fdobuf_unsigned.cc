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

#include "fdbuf.h"
#include <limits.h>

#define MAXSTRLEN ((sizeof(signed long)*CHAR_BIT)/3)

fdobuf& fdobuf::operator<<(unsigned long i)
{
  if(i == 0)
    return operator<<('0');
  char buf[MAXSTRLEN+1];
  char* ptr = buf+MAXSTRLEN;
  *ptr-- = 0;
  while(i) {
    *ptr-- = i % 10 + '0';
    i /= 10;
  }
  return operator<<(ptr+1);
}
