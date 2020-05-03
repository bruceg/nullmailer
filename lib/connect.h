#ifndef NULLMAILER_CONNECT__H__
#define NULLMAILER_CONNECT__H__

extern int tcpconnect(const char* hostname, int port, const char* source);
extern int unixconnect(const char* unix_socket);

#endif // NULLMAILER_CONNECT__H__
