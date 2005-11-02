// nullmailer -- a simple relay-only MTA
// Copyright (C) 2005  Bruce Guenter <bruceg@em.ca>
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
// You can contact me at <bruceg@em.ca>.  There is also a mailing list
// available to discuss this package.  To subscribe, send an email to
// <nullmailer-subscribe@lists.em.ca>.

#include "config.h"
#include <stdlib.h>
#include <unistd.h>
#include "connect.h"
#include "errcodes.h"
#include "fdbuf/fdbuf.h"
#include "itoa.h"
#include "mystring/mystring.h"
#include "protocol.h"

int port = 25;
char* auth = "";
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

void to64(char* infile, char* outfile);
void to64(const mystring& infile, mystring& outfile);
void output64chunk(int c1, int c2, int c3, int pads, char** outfile);
void output64chunk(int c1, int c2, int c3, int pads, mystring& outfile);

void protocol_send(fdibuf* in, int fd)
{
  mystring hh = getenv("HELOHOST");
  if (!hh) protocol_fail(1, "$HELOHOST is not set");
  smtp conn(fd);
  conn.docmd("", 200);
  conn.docmd("HELO " + hh, 200);

  if ( strlen(auth) > 0 )
  {
    mystring authstr = auth;
    mystring uname = authstr.left(authstr.find_first(','));
    mystring pass = authstr.sub(authstr.find_first(',')+1,authstr.length());
    mystring plain = uname + "\1" + uname + "\1" + pass;
    mystring encoded = "AUTH PLAIN ";
    to64(plain,encoded);
    conn.docmd(encoded,200);
  }

  conn.send(in);
}

static char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void to64(const mystring& infile, mystring& outfile)
{
    int c1, c2, c3;
    size_t inpos = 0;
    while ((c1 = infile[inpos++])) {
	c2 = infile[inpos++];
	if (!c2) {
	    output64chunk(c1, 0, 0, 2, outfile);
	} else {
	    c3 = infile[inpos++];
	    if (!c3) {
		output64chunk(c1, c2, 0, 1, outfile);
	    } else {
		output64chunk(c1, c2, c3, 0, outfile);
	    }
	}
    }
}

void output64chunk(int c1, int c2, int c3, int pads, mystring& outfile)
{
  if (c1==1) c1 = 0;
  if (c2==1) c2 = 0;
  if (c3==1) c3 = 0;

  char out[5];
  out[0] = basis_64[c1>>2];
  out[1] = basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)];
  switch (pads)
  {
  case 0:
    out[2] = basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
    out[3] = basis_64[c3 & 0x3F];
    break;
  case 1:
    out[2] = basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)];
    out[3] = '=';
    break;
  case 2:
    out[2] = '=';
    out[3] = '=';
    break;
  }; 
  out[4] = 0;
  outfile += out;
}

