#ifndef NULLMAILER__PROTOCOL__H__
#define NULLMAILER__PROTOCOL__H__

#include "fdbuf/fdbuf.h"

extern void protocol_fail(int e, const char* msg);
extern void protocol_succ(const char* msg);

// This must be provided by the protocol, but will be set by the lib.
extern int port;

extern void protocol_prep(fdibuf* in);
extern void protocol_send(fdibuf* in, int fd);

#endif
