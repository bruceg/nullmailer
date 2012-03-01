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
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

///////////////////////////////////////////////////////////////////////////////
// Class fdibuf
///////////////////////////////////////////////////////////////////////////////
fdibuf::fdibuf(int fdesc, bool dc, unsigned bufsz)
  : fdbuf(fdesc, dc, bufsz)
{
}

fdibuf::fdibuf(const char* filename, unsigned bufsz)
  : fdbuf(open(filename, O_RDONLY), true, bufsz)
{
  if(fd == -1) {
    flags = flag_error;
    errnum = errno;
  }
}

fdibuf::~fdibuf()
{
}

bool fdibuf::eof() const
{
  return (flags & flag_eof) && (bufstart >= buflength);
}

bool fdibuf::operator!() const
{
  return eof() || error() || closed();
}

// refill is protected -- no locking
bool fdibuf::refill()
{
  if(flags)
    return false;
  if(bufstart != 0) {
    if(bufstart < buflength) {
      buflength -= bufstart;
      memcpy(buf, buf+bufstart, buflength);
    } else
      buflength = 0;
    bufstart = 0;
  }
  unsigned oldbuflength = buflength;
  if(buflength < bufsize) {
    ssize_t red = ::read(fd, buf+buflength, bufsize-buflength);
    if(red == -1) {
      errnum = errno;
      flags |= flag_error;
    }
    else if(red == 0)
      flags |= flag_eof;
    else {
      buflength += red;
      offset += red;
    }
  }
  return buflength > oldbuflength;
}

bool fdibuf::get(char& ch)
{
  lock();
  count = 0;
  if(bufstart >= buflength)
    refill();
  bool r = true;
  if(eof() || error())
    r = false;
  else {
    ch = buf[bufstart++];
    count = 1;
  }
  unlock();
  return r;
}

bool fdibuf::read_large(char* data, unsigned datalen)
{
  lock();
  count = 0;

  // If there's any content in the buffer, memcpy it out first.
  unsigned len = buflength - bufstart;
  if(len > datalen)
    len = datalen;
  memcpy(data, buf+bufstart, len);
  data += len;
  datalen -= len;
  bufstart += len;
  count += len;

  // After the buffer is empty and there's still data to read,
  // read it straight from the fd instead of copying it through the buffer.
  while(datalen > 0) {
    ssize_t red = ::read(fd, data, datalen);
    if(red == -1) {
      errnum = errno;
      flags |= flag_error;
      break;
    }
    else if(red == 0) {
      flags |= flag_eof;
      break;
    }
    data += red;
    datalen -= red;
    offset += red;
    count += red;
  }
  unlock();
  return datalen == 0;
}

bool fdibuf::read(char* data, unsigned datalen)
{
  if(datalen >= bufsize)
    return read_large(data, datalen);
  lock();
  count = 0;
  char* ptr = data;
  while(datalen && !eof()) {
    if(bufstart >= buflength)
      refill();
    unsigned len = buflength-bufstart;
    if(len > datalen)
      len = datalen;
    memcpy(ptr, buf+bufstart, len);
    bufstart += len;
    datalen -= len;
    ptr += len;
    count += len;
  }
  unlock();
  return !datalen;
}

bool fdibuf::seek(unsigned o)
{
  lock();
  unsigned buf_start = offset - buflength;
  if(o >= buf_start && o < offset) {
    bufstart = o - buf_start;
  }
  else {
    if(lseek(fd, o, SEEK_SET) != (off_t)o) {
      errnum = errno;
      flags |= flag_error;
      unlock();
      return false;
    }
    offset = o;
    buflength = bufstart = 0;
  }
  count = 0;
  flags &= ~flag_eof;
  unlock();
  return true;
}

bool fdibuf::seekfwd(unsigned o)
{
  return seek(tell() + o);
}

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////
fdibuf fin(0);
