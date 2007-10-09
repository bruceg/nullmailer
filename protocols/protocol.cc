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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "connect.h"
#include "errcodes.h"
#include "protocol.h"
#include "cli++.h"

const char* user = 0;
const char* pass = 0;
int auth_method = AUTH_PLAIN;
const char* cli_help_suffix = "";
const char* cli_args_usage = "remote-address < mail-file";
const int cli_args_min = 1;
const int cli_args_max = 1;
cli_option cli_options[] = {
  { 'p', "port", cli_option::integer, 0, &port,
    "Set the port number on the remote host to connect to", 0 },
  { 0, "user", cli_option::string, 0, &user,
    "Set the user name for authentication", 0 },
  { 0, "pass", cli_option::string, 0, &pass,
    "Set the password for authentication", 0 },
  { 0, "auth-login", cli_option::flag, AUTH_LOGIN, &auth_method,
    "Use AUTH LOGIN instead of AUTH PLAIN in SMTP", 0 },
  {0, 0, cli_option::flag, 0, 0, 0, 0}
};

void protocol_fail(int e, const char* msg)
{
  ferr << cli_program << ": Failed: " << msg << endl;
  exit(e);
}

void protocol_succ(const char* msg)
{
  ferr << cli_program << ": Succeeded: " << msg << endl;
  exit(0);
}

int cli_main(int, char* argv[])
{
  const char* remote = argv[0];
  fdibuf in(0, true);
  protocol_prep(&in);
  int fd = tcpconnect(remote, port);
  if(fd < 0)
    protocol_fail(-fd, "Connect failed");
  protocol_send(&in, fd);
  return 0;
}

