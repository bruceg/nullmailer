// nullmailer -- a simple relay-only MTA
// Copyright (C) 2017  Bruce Guenter <bruce@untroubled.org>
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

#include "config.h"
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "errcodes.h"
#include "itoa.h"
#include "connect.h"

static int err_return(int errn, int dflt)
{
  if (errn == HOST_NOT_FOUND)
    return -ERR_HOST_NOT_FOUND;
  if (errn == NO_ADDRESS)
    return -ERR_NO_ADDRESS;
  if (errn == NO_RECOVERY || errn == EAI_FAIL)
    return -ERR_GHBN_FATAL;
  if (errn == TRY_AGAIN || errn == EAI_AGAIN)
    return -ERR_GHBN_TEMP;
  if (errn == EAI_NONAME)
    return -ERR_HOST_NOT_FOUND;
  if (errn == ECONNREFUSED)
    return -ERR_CONN_REFUSED;
  if (errn == ETIMEDOUT)
    return -ERR_CONN_TIMEDOUT;
  if (errn == ENETUNREACH)
    return -ERR_CONN_UNREACHABLE;
  return -dflt;
}

#ifdef HAVE_GETADDRINFO

static int getaddr(const char* hostname, int port, struct addrinfo** result)
{
  const char *service = itoa(port, 6);
  struct addrinfo req;
  memset(&req, 0, sizeof(req));
  req.ai_flags = AI_NUMERICSERV;
  req.ai_socktype = SOCK_STREAM;
  int e = getaddrinfo(hostname, service, &req, result);
  return e ? err_return(e, ERR_GHBN_TEMP) : 0;
}

static bool canbind(int family, const struct addrinfo* ai)
{
  for (; ai; ai = ai->ai_next)
    if (ai->ai_family == family)
      return true;
  return false;
}

static bool bindit(int fd, int family, const struct addrinfo* ai)
{
  for (; ai; ai = ai->ai_next)
    if (ai->ai_family == family)
      if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0)
        return true;
  return false;
}

int tcpconnect(const char* hostname, int port, const char* source)
{
  struct addrinfo* res;
  int err = getaddr(hostname, port, &res);
  if (err)
    return err;
  struct addrinfo* source_addr = NULL;
  if (source) {
    err = getaddr(source, 0, &source_addr);
    if (err)
      return err;
  }
  int s = -1;
  err = ERR_CONN_FAILED;
  struct addrinfo* orig_res = res;

  if (source_addr)
    // Check if some address is the same family as the source
    for (; res != NULL; res = res->ai_next)
      if (canbind(res->ai_family, source_addr))
        break;
  if (res == NULL)
    return -ERR_BIND_FAILED;

  for (; res != NULL; res = res->ai_next) {
    if (!source_addr || canbind(res->ai_family, source_addr)) {
      s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
      if(s > 0) {
        if(source_addr && !bindit(s, res->ai_family, source_addr)) {
          close(s);
          err = ERR_BIND_FAILED;
          s = -1;
          break;
        }
        if(connect(s, res->ai_addr, res->ai_addrlen) == 0)
          break;
        close(s);
        s = -1;
      }
    }
  }

  freeaddrinfo(orig_res);
  if (source_addr)
    freeaddrinfo(source_addr);

  if(s < 0)
    return err_return(errno, err);
  return s;
}

#else

static int sethostbyname(const char* hostname, struct sockaddr_in& sa)
{
  struct hostent *he = gethostbyname(hostname);
  if(!he)
    return err_return(h_errno, ERR_GHBN_TEMP);
  memcpy(&sa.sin_addr, he->h_addr, he->h_length);
  return 0;
}

int tcpconnect(const char* hostname, int port, const char* source)
{
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  int e = sethostbyname(hostname, sa);
  if(e) return e;
  struct sockaddr_in source_sa;
  memset(&source_sa, 0, sizeof source_sa);
  if(source) {
    e = sethostbyname(source, source_sa);
    if(e) return e;
  }
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  int s = socket(PF_INET, SOCK_STREAM, 0);
  if(s == -1)
    return -ERR_SOCKET;
  if(source && bind(s, (sockaddr*)&source_sa, sizeof source_sa) != 0) {
    close(s);
    return err_return(errno, ERR_BIND_FAILED);
  }
  if(connect(s, (sockaddr*)&sa, sizeof(sa)) != 0) {
    close(s);
    return err_return(errno, ERR_CONN_FAILED);
  }
  return s;
}

#endif
