// nullmailer -- a simple relay-only MTA
// Copyright (C) 2018  Bruce Guenter <bruce@untroubled.org>
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
//
// You can contact me at <bruce@untroubled.org>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.untroubled.org>.

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "selfpipe.h"

static int fds[2] = { -1, -1 };

static int fcntl_fl_on(int fd, int flag)
{
  int flags;
  int newflags;
  if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
    return 0;
  if ((newflags = flags | flag) != flags)
    if (fcntl(fd, F_SETFL, newflags))
      return 0;
  return 1;
}

selfpipe::selfpipe()
{
  if (fds[0] < 0) {
    if (pipe(fds) != -1) {
      if (fcntl_fl_on(fds[0], O_NONBLOCK)
	  && fcntl_fl_on(fds[1], O_NONBLOCK)
	  && fcntl_fl_on(fds[1], FD_CLOEXEC)
	  && fcntl_fl_on(fds[1], FD_CLOEXEC))
	return;

      close(fds[0]);
      close(fds[1]);
    }
    fds[0] = fds[1] = -1;
  }
}

selfpipe::operator bool() const
{
  return fds[0] >= 0;
}

static void catcher(int sig)
{
  signal(sig, catcher);
  write(fds[1], &sig, sizeof sig);
}

void selfpipe::catchsig(int sig)
{
  signal(sig, catcher);
}

int selfpipe::caught()
{
  int buf;
  if (read(fds[0], &buf, sizeof buf) == sizeof buf)
    return buf;
  return 0;
}

int selfpipe::waitsig(int timeout)
{
  if (timeout > 0) {
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fds[0], &fdset);
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    int s;
    while ((s = select(fds[0] + 1, &fdset, 0, 0,
		     (timeout <= 0) ? 0 : &tv)) == -1) {
      if (errno != EINTR)
        return -1;
    }
    if (s != 1)
      return 0;
  }
  return caught();
}
