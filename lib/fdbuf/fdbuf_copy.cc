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

///////////////////////////////////////////////////////////////////////////////
// Other routines
///////////////////////////////////////////////////////////////////////////////
bool fdbuf_copy(fdibuf& in, fdobuf& out, bool noflush)
{
  if(in.eof())
    return true;
  if(!in || !out)
    return false;
  do {
    char buf[FDBUF_SIZE];
    if(!in.read(buf, FDBUF_SIZE) && in.last_count() == 0)
      break;
    if(!out.write(buf, in.last_count()) && out.last_count() < in.last_count())
      return false;
  } while(!in.eof());
  if(!noflush && !out.flush())
    return false;
  return in.eof();
}
