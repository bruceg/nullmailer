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
// Class fdbuf
///////////////////////////////////////////////////////////////////////////////
fdbuf::fdbuf(int fdesc, bool dc, unsigned bufsz)
  : buf(new char[bufsz]),
    buflength(0),
    bufstart(0),
    offset(0),
    errnum(0),
    flags(0),
    bufsize(bufsz),
    fd(fdesc),
    do_close(dc)
{
  if(!buf) {
    flags = flag_error;
    errnum = errno;
  }
  if(fdesc < 0)
    flags |= flag_closed;
#ifdef _REENTRANT
  pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
  mutex = tmp;
  pthread_mutex_init(&mutex, 0);
#else
#ifdef FDBUF_MUTEX_DEBUG
  mutex_count = 0;
#endif
#endif
}

fdbuf::~fdbuf()
{
  close();
#ifdef _REENTRANT
  pthread_mutex_destroy(&mutex);
#endif
  delete buf;
}

bool fdbuf::error() const
{
  return flags & flag_error;
}

bool fdbuf::closed() const
{
  return flags & flag_closed;
}

bool fdbuf::close()
{
  if(do_close && fd >= 0 && !(flags & flag_closed)) {
    if(::close(fd) == -1) {
      errnum = errno;
      flags |= flag_error;
      return false;
    }
    flags |= flag_closed;
  }
  return true;
}

#if defined(FDBUF_MUTEX_DEBUG) && !defined(_REENTRANT)
{
  int* null = 0;
  (*null)++;
  kill(getpid(), 9);
}

// Debugging code
void fdbuf::lock()
{
  if(mutex)
    abort();
  ++mutex;
}

void fdbuf::unlock()
{
  if(mutex != 1)
    abort();
  --mutex;
}
#endif
