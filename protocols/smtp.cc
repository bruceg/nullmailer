#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include "connect.h"
#include "errcodes.h"
#include "fdbuf.h"
#include "hostname.h"
#include "itoa.h"
#include "mystring.h"
#include "protocol.h"

int port = 25;
const char* cli_program = "smtp";
const char* cli_help_prefix = "Send an email message via SMTP\n";

class smtp 
{
  fdibuf in;
  fdobuf out;
public:
  smtp(int fd);
  ~smtp();
  int get(mystring& str);
  int put(mystring cmd, mystring& result);
  void docmd(mystring cmd, int range, bool nofail=false);
  void send_data(fdibuf* msg);
  void send_envelope(fdibuf* msg);
  void send(fdibuf* msg);
};

smtp::smtp(int fd)
  : in(fd), out(fd)
{
}

smtp::~smtp()
{
}

int smtp::get(mystring& str)
{
  mystring tmp;
  str = "";
  int code = -1;
  while(in.getline(tmp)) {
    if(tmp[tmp.length()-1] == '\r')
      tmp = tmp.left(tmp.length()-1);
    code = atoi(tmp.c_str());
    if(!!str)
      str += "/";
    str += tmp.right(4);
    if(tmp[3] != '-')
      break;
  }
  return code;
}

int smtp::put(mystring cmd, mystring& result)
{
  out << cmd << "\r\n";
  if(!out.flush())
    return -1;
  return get(result);
}

void smtp::docmd(mystring cmd, int range, bool nofail)
{
  mystring msg;
  int code;
  if(!cmd)
    code = get(msg);
  else
    code = put(cmd, msg);
  if(!nofail && (code < range || code >= (range+100)))
    exit(ERR_PROTO);
}

void smtp::send_envelope(fdibuf* msg)
{
  mystring tmp;
  msg->getline(tmp);
  docmd("MAIL FROM: <" + tmp + ">", 250);
  while(msg->getline(tmp) && !!tmp)
    docmd("RCPT TO: <" + tmp + ">", 250);
}

void smtp::send_data(fdibuf* msg)
{
  docmd("DATA", 354);
  mystring tmp;
  while(msg->getline(tmp)) {
    if((tmp[0] == '.' && tmp[1] == 0 && !(out << ".")) ||
       !(out << tmp << "\r\n"))
      exit(ERR_MSG_WRITE);
  }
  docmd(".", 250);
}

void smtp::send(fdibuf* msg)
{
  send_envelope(msg);
  send_data(msg);
}

int protocol_prep(fdibuf*)
{
  return 0;
}

int protocol_send(fdibuf* in, int fd)
{
  smtp conn(fd);
  conn.docmd("", 220);
  conn.docmd("HELO " + hostname(), 250);
  conn.send(in);
  conn.docmd("QUIT", 221, true);
  return 0;
}
