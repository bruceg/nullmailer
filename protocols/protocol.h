#ifndef NULLMAILER__PROTOCOL__H__
#define NULLMAILER__PROTOCOL__H__

#include <fdbuf.h>

// This must be provided by the protocol, but will be set by the lib.
extern int port;

extern int protocol_prep(fdibuf* in);
extern int protocol_send(fdibuf* in, int fd);

#endif
