// nullmailer -- a simple relay-only MTA
// Copyright (C) 2007  Bruce Guenter <bruce@untroubled.org>
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
#include <stdlib.h>
#include <unistd.h>
#include "base64.h"
#include "connect.h"
#include "errcodes.h"
#include "fdbuf/fdbuf.h"
#include "itoa.h"
#include "mystring/mystring.h"
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
  void docmd(mystring cmd, int range, bool show_succ=false);
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
    str += tmp;
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

void smtp::docmd(mystring cmd, int range, bool show_succ)
{
  mystring msg;
  int code;
  if(!cmd)
    code = get(msg);
  else
    code = put(cmd, msg);
  if(code < range || code >= (range+100)) {
    int e;
    if(code >= 500)
      e = ERR_MSG_PERMFAIL;
    else if(code >= 400)
      e = ERR_MSG_TEMPFAIL;
    else
      e = ERR_PROTO;
    out << "QUIT\r\n";
    out.flush();
    protocol_fail(e, msg.c_str());
  }
  else if(show_succ)
    protocol_succ(msg.c_str());
}

void smtp::send_envelope(fdibuf* msg)
{
  mystring tmp;
  msg->getline(tmp);
  docmd("MAIL FROM:<" + tmp + ">", 200);
  while(msg->getline(tmp) && !!tmp)
    docmd("RCPT TO:<" + tmp + ">", 200);
}

void smtp::send_data(fdibuf* msg)
{
  docmd("DATA", 300);
  mystring tmp;
  while(msg->getline(tmp)) {
    if((tmp[0] == '.' && !(out << ".")) ||
       !(out << tmp << "\r\n"))
      protocol_fail(ERR_MSG_WRITE, "Error sending message to remote");
  }
  docmd(".", 200, true);
}

void smtp::send(fdibuf* msg)
{
  send_envelope(msg);
  send_data(msg);
}

void protocol_prep(fdibuf*)
{
}

void protocol_send(fdibuf* in, int fd)
{
  mystring hh = getenv("HELOHOST");
  if (!hh) protocol_fail(1, "$HELOHOST is not set");
  smtp conn(fd);
  conn.docmd("", 200);

  if (user != 0 && pass != 0) {
    conn.docmd("EHLO " + hh, 200);
    if (auth_method == AUTH_LOGIN) {
      mystring encoded;
      base64_encode(user, encoded);
      conn.docmd("AUTH LOGIN " + encoded, 300);
      encoded = "";
      base64_encode(pass, encoded);
      conn.docmd(encoded, 200);
    }
    else {
      mystring plain(user);
      plain += '\0';
      plain += user;
      plain += '\0';
      plain += pass;
      mystring encoded = "AUTH PLAIN ";
      base64_encode(plain, encoded);
      conn.docmd(encoded, 200);
    }
  }
  else
    conn.docmd("HELO " + hh, 200);

  conn.send(in);
}
