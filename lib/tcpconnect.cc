#include "config.h"
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "errcodes.h"
#include "connect.h"

static int sethostbyname(const mystring& hostname, struct sockaddr_in& sa)
{
  struct hostent *he = gethostbyname(hostname.c_str());
  if(!he) {
    switch(h_errno) {
    case HOST_NOT_FOUND: return -ERR_HOST_NOT_FOUND;
    case NO_ADDRESS: return -ERR_NO_ADDRESS;
    case NO_RECOVERY: return -ERR_GHBN_FATAL;
    case TRY_AGAIN: return -ERR_GHBN_TEMP;
    default: return -ERR_GHBN_TEMP;
    }
  }
  memcpy(&sa.sin_addr, he->h_addr, he->h_length);
  return 0;
}

int tcpconnect(const mystring& hostname, int port)
{
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  int e = sethostbyname(hostname, sa);
  if(e) return e;
  sa.sin_port = htons(port);
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s == -1)
    return -ERR_SOCKET;
  if(connect(s, (const sockaddr*)&sa, sizeof(sa)) != 0) {
    switch(errno) {
    case ECONNREFUSED: return -ERR_CONN_REFUSED;
    case ETIMEDOUT: return -ERR_CONN_TIMEDOUT;
    case ENETUNREACH: return -ERR_CONN_UNREACHABLE;
    default: return -ERR_CONN_FAILED;
    }
  }
  return s;
}
