// nullmailer -- a simple relay-only MTA
// Copyright (C) 2012  Bruce Guenter <bruce@untroubled.org>
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

const int default_port = 25;
const int default_ssl_port = 465;
const char* cli_program = "smtp";
const char* cli_help_prefix = "Send an email message via SMTP\n";

class smtp 
{
  fdibuf& in;
  fdobuf& out;
  mystring caps;
public:
  smtp(fdibuf& netin, fdobuf& netout);
  ~smtp();
  int get(mystring& str);
  int put(mystring cmd, mystring& result);
  void docmd(mystring cmd, int range, mystring& result);
  void docmd(mystring cmd, int range);
  void dohelo(bool ehlo);
  bool hascap(const char* name, const char* word = NULL);
  void auth_login(void);
  void auth_plain(void);
  void send_data(fdibuf& msg);
  void send_envelope(fdibuf& msg);
  void send(fdibuf& msg);
};

smtp::smtp(fdibuf& netin, fdobuf& netout)
  : in(netin), out(netout)
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
      str += "\n";
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

void smtp::docmd(mystring cmd, int range, mystring& result)
{
  int code;
  if(!cmd)
    code = get(result);
  else
    code = put(cmd, result);
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
    protocol_fail(e, result.c_str());
  }
}

void smtp::docmd(mystring cmd, int range)
{
  mystring msg;
  docmd(cmd, range, msg);
}

void smtp::dohelo(bool ehlo)
{
  mystring hh = getenv("HELOHOST");
  if (!hh) protocol_fail(1, "$HELOHOST is not set");
  docmd((ehlo ? "EHLO " : "HELO ") + hh, 200, caps);
}

static int issep(char ch)
{
  return ch == ' ' || ch == '\n' || ch == '\0';
}

bool smtp::hascap(const char* name, const char* word)
{
  const size_t namelen = strlen(name);
  int i = -1;
  do {
    const char* s = caps.c_str() + i + 5;
    if (strncasecmp(s, name, namelen) == 0) {
      if (s[namelen] == '\n')
	return word == NULL;
      else if (s[namelen] == ' ') {
	if (word == NULL)
	  return true;
	s += namelen + 1;
	const size_t wordlen = strlen(word);
	do {
	  if (strncasecmp(s, word, wordlen) == 0 && issep(s[wordlen]))
	    return true;
	  while (!issep(*s))
	    ++s;
	} while (*s++ == ' ');
	return false;
      }
    }
    i = caps.find_first('\n', i+1);
  } while (i > 0);
  return false;
}

void smtp::auth_login(void)
{
  mystring encoded;
  base64_encode(user, encoded);
  docmd("AUTH LOGIN " + encoded, 300);
  encoded = "";
  base64_encode(pass, encoded);
  docmd(encoded, 200);
}

void smtp::auth_plain(void)
{
  mystring plain(user);
  plain += '\0';
  plain += user;
  plain += '\0';
  plain += pass;
  mystring encoded = "AUTH PLAIN ";
  base64_encode(plain, encoded);
  docmd(encoded, 200);
}

void smtp::send_envelope(fdibuf& msg)
{
  mystring tmp;
  msg.getline(tmp);
  docmd("MAIL FROM:<" + tmp + ">", 200);
  while(msg.getline(tmp) && !!tmp)
    docmd("RCPT TO:<" + tmp + ">", 200);
}

void smtp::send_data(fdibuf& msg)
{
  docmd("DATA", 300);
  mystring tmp;
  while(msg.getline(tmp)) {
    if((tmp[0] == '.' && !(out << ".")) ||
       !(out << tmp << "\r\n"))
      protocol_fail(ERR_MSG_WRITE, "Error sending message to remote");
  }
  docmd(".", 200, tmp);
  out << "QUIT\r\n";
  out.flush();
  protocol_succ(tmp.c_str());
}

void smtp::send(fdibuf& msg)
{
  send_envelope(msg);
  send_data(msg);
}

void protocol_prep(fdibuf&)
{
}

static int did_starttls = 0;

void protocol_starttls(fdibuf& netin, fdobuf& netout)
{
  smtp conn(netin, netout);
  conn.docmd("", 200);
  conn.dohelo(true);
  conn.docmd("STARTTLS", 200);
  did_starttls = 1;
}

void protocol_send(fdibuf& in, fdibuf& netin, fdobuf& netout)
{
  smtp conn(netin, netout);
  if (!did_starttls)
    conn.docmd("", 200);

  if (user != 0 && pass != 0) {
    conn.dohelo(true);
    if (auth_method == AUTH_LOGIN)
      conn.auth_login();
    else if (auth_method == AUTH_PLAIN)
      conn.auth_plain();
    else {
      // Detect method
      if (conn.hascap("AUTH", "PLAIN"))
	conn.auth_plain();
      else if (conn.hascap("AUTH", "LOGIN"))
	conn.auth_login();
      else
	protocol_fail(ERR_MSG_TEMPFAIL, "Server does not advertise any supported authentication methods");
    }
  }
  else
    conn.dohelo(false);

  conn.send(in);
}
