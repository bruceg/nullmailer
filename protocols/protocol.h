#ifndef NULLMAILER__PROTOCOL__H__
#define NULLMAILER__PROTOCOL__H__

#include "fdbuf/fdbuf.h"

extern const int default_port;
extern const int default_ssl_port;
extern void protocol_fail(int e, const char* msg);
extern void protocol_succ(const char* msg);

#define AUTH_DETECT 0
#define AUTH_LOGIN 1
#define AUTH_PLAIN 2
extern const char* user;
extern const char* pass;
extern int auth_method;
extern int port;
extern int use_ssl;

extern void protocol_prep(fdibuf& in);
extern void protocol_send(fdibuf& in, fdibuf& netin, fdobuf& netout);

#endif
