// nullmailer -- a simple relay-only MTA
// Copyright (C) 2018  Bruce Guenter <bruce@untroubled.org>
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
#include "errcodes.h"
#include "fdbuf/fdbuf.h"
#include "hostname.h"
#include "itoa.h"
#include "mystring/mystring.h"
#include "netstring.h"
#include "protocol.h"

const int default_port = 628;
const int default_tls_port = -1; // No standard for QMQP over SSL exists
const char* cli_program = "qmqp";
const char* cli_help_prefix = "Send an email message via QMQP\n";

class qmqp 
{
  fdibuf& in;
  fdobuf& out;
public:
  qmqp(fdibuf& netin, fdobuf& netout);
  ~qmqp();
  void send(fdibuf& msg, unsigned long size, const mystring& env);
};

qmqp::qmqp(fdibuf& netin, fdobuf& netout)
  : in(netin), out(netout)
{
}

qmqp::~qmqp()
{
}

bool skip_envelope(fdibuf& msg)
{
  if(!msg.rewind())
    return false;
  mystring tmp;
  while(msg.getline(tmp))
    if(!tmp)
      break;
  return msg;
}
  
void qmqp::send(fdibuf& msg, unsigned long size, const mystring& env)
{
  if(!skip_envelope(msg))
    protocol_fail(ERR_MSG_READ, "Error re-reading message");
  unsigned long fullsize = strlen(itoa(size)) + 1 + size + 1 + env.length();
  out << itoa(fullsize) << ":";	// Start the "outer" netstring
  out << itoa(size) << ":";	// Start the message netstring
  fdbuf_copy(msg, out, true);	// Send out the message
  out << ","			// End the message netstring
      << env			// The envelope is already encoded
      << ",";			// End the "outer" netstring
  if(!out.flush())
    protocol_fail(ERR_MSG_WRITE, "Error sending message to remote");
  mystring response;
  if(!in.getnetstring(response))
    protocol_fail(ERR_PROTO, "Response from remote was not a netstring");
  switch(response[0]) {
  case 'K': protocol_succ(response.c_str()+1); break;
  case 'Z': protocol_fail(ERR_MSG_TEMPFAIL, response.c_str()+1); break;
  case 'D': protocol_fail(ERR_MSG_PERMFAIL, response.c_str()+1); break;
  default: protocol_fail(ERR_PROTO, "Invalid status byte in response");
  }
}

bool compute_size(fdibuf& msg, unsigned long& size)
{
  char buf[4096];
  size = 0;
  while(msg.read(buf, 4096))
    size += msg.last_count();
  if(msg.eof())
    size += msg.last_count();
  return size > 0;
}

bool make_envelope(fdibuf& msg, mystring& env)
{
  mystring tmp;
  while(msg.getline(tmp)) {
    if(!tmp)
      return true;
    env += str2net(tmp);
  }
  return false;
}
    
bool preload_data(fdibuf& msg, unsigned long& size, mystring& env)
{
  return make_envelope(msg, env) &&
    compute_size(msg, size);
}

static unsigned long msg_size;
static mystring msg_envelope;

void protocol_prep(fdibuf& in)
{
  if(!preload_data(in, msg_size, msg_envelope))
    protocol_fail(ERR_MSG_READ, "Error reading message");
}

void protocol_starttls(fdibuf& netin, fdobuf& netout)
{
  protocol_fail(ERR_USAGE, "QMQP does not support STARTTLS");
  (void)netin;
  (void)netout;
}

void protocol_send(fdibuf& in, fdibuf& netin, fdobuf& netout)
{
  alarm(60*60);			// Connection must close after an hour
  qmqp conn(netin, netout);
  conn.send(in, msg_size, msg_envelope);
}
