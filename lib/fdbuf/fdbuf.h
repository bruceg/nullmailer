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

#ifndef FDBUF__H__
#define FDBUF__H__

#include "config.h"
#include <string.h>
#include <fcntl.h>

#ifdef _REENTRANT
#include <pthread.h>
#endif

#ifndef FDBUF_SIZE
#define FDBUF_SIZE 4096
#endif

class mystring;

class fdbuf 
{
public:
  enum flagbits { flag_eof=1, flag_error=2, flag_closed=4 };

  fdbuf(int fdesc, bool dc, unsigned bufsz = FDBUF_SIZE);
  ~fdbuf();
  bool error() const;
  bool closed() const;
  bool close();
#ifdef _REENTRANT
  void lock() { pthread_mutex_lock(&mutex); }
  void unlock() { pthread_mutex_unlock(&mutex); }
#else
#ifdef FDBUF_MUTEX_DEBUG
  void lock();
  void unlock();
#else
  void lock() { }
  void unlock() { }
#endif
#endif
protected:
  char* const buf;
  unsigned buflength;		// Length of the data in the buffer
  unsigned bufstart;		// Start of the data in the buffer
  unsigned offset;		// Current file read/write offset
  int errnum;			// Saved error flag
  unsigned flags;		// Status flags

  const unsigned bufsize;	// Total buffer size
  const int fd;
  const bool do_close;		// True to close on destructor

#ifdef _REENTRANT
  pthread_mutex_t mutex;
#else
#ifdef FDBUF_MUTEX_DEBUG
  unsigned mutex;
#endif
#endif
};

#include "fdbuf/fdibuf.h"
#include "fdbuf/fdobuf.h"

bool fdbuf_copy(fdibuf&, fdobuf&, bool noflush = false);

#endif // FDBUF__H__
