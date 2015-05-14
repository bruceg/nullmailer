#ifndef NULLMAILER__PROTOCOL__H__
#define NULLMAILER__PROTOCOL__H__

#include "fdbuf/fdbuf.h"

#define DEFAULT_CA_FILE "/etc/ssl/certs/ca-certificates.crt"

extern const int default_port;
extern const int default_ssl_port;
extern void protocol_fail(int e, const char* msg);
extern void protocol_succ(const char* msg);

#define AUTH_DETECT 0
#define AUTH_LOGIN 1
#define AUTH_PLAIN 2
extern const char* user;
extern const char* pass;
extern const char* passfile;
extern int auth_method;
extern int port;
extern int use_ssl;
extern int use_starttls;
extern int tls_insecure;

extern void protocol_prep(fdibuf& in);
extern void protocol_send(fdibuf& in, fdibuf& netin, fdobuf& netout);
extern void protocol_starttls(fdibuf& netin, fdobuf& netout);

extern int tls_insecure;
extern const char* tls_x509certfile;
extern const char* tls_x509cafile;
extern const char* tls_x509crlfile;
extern int tls_x509derfmt;
extern void tls_init(const char* remote);
extern void tls_send(fdibuf& in, int fd);

#endif
