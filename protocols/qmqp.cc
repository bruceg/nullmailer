#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include "errcodes.h"
#include "fdbuf/fdbuf.h"
#include "hostname.h"
#include "itoa.h"
#include "mystring/mystring.h"
#include "netstring.h"
#include "protocol.h"

int port = 628;
const char* cli_program = "qmqp";
const char* cli_help_prefix = "Send an emal message via QMQP\n";

class qmqp 
{
  fdibuf in;
  fdobuf out;
public:
  qmqp(int fd);
  ~qmqp();
  int send(fdibuf* msg, unsigned long size, const mystring& env);
};

qmqp::qmqp(int fd)
  : in(fd), out(fd)
{
}

qmqp::~qmqp()
{
}

bool skip_envelope(fdibuf* msg)
{
  if(!msg->rewind())
    return false;
  mystring tmp;
  while(msg->getline(tmp))
    if(!tmp)
      break;
  return msg;
}
  
int qmqp::send(fdibuf* msg, unsigned long size, const mystring& env)
{
  if(!skip_envelope(msg))
    return -ERR_MSG_READ;
  unsigned long fullsize = strlen(itoa(size)) + 1 + size + 1 + env.length();
  out << itoa(fullsize) << ":"	// Start the "outer" netstring
      << itoa(size) << ":";	// Start the message netstring
  fdbuf_copy(*msg, out, true);	// Send out the message
  out << ","			// End the message netstring
      << env			// The envelope is already encoded
      << ",";			// End the "outer" netstring
  if(!out.flush())
    return -ERR_MSG_WRITE;
  mystring response;
  if(!in.getnetstring(response))
    return -ERR_PROTO;
  switch(response[0]) {
  case 'K': return 0;
  case 'Z': return -ERR_MSG_TEMPFAIL;
  case 'D': return -ERR_MSG_PERMFAIL;
  default: return -ERR_PROTO;
  }
}

bool compute_size(fdibuf* msg, unsigned long& size)
{
  char buf[4096];
  size = 0;
  while(msg->read(buf, 4096))
    size += msg->last_count();
  if(msg->eof())
    size += msg->last_count();
  return size > 0;
}

bool make_envelope(fdibuf* msg, mystring& env)
{
  mystring tmp;
  while(msg->getline(tmp)) {
    if(!tmp)
      return true;
    env += str2net(tmp);
  }
  return false;
}
    
bool preload_data(fdibuf* msg, unsigned long& size, mystring& env)
{
  return make_envelope(msg, env) &&
    compute_size(msg, size);
}

static unsigned long msg_size;
static mystring msg_envelope;

int protocol_prep(fdibuf* in)
{
  if(!preload_data(in, msg_size, msg_envelope))
    return ERR_MSG_READ;
  return 0;
}

int protocol_send(fdibuf* in, int fd)
{
  alarm(60*60);			// Connection must close after an hour
  qmqp conn(fd);
  return -conn.send(in, msg_size, msg_envelope);
}
