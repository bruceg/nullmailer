// nullmailer -- a simple relay-only MTA
// Copyright (C) 1999-2003  Bruce Guenter <bruceg@em.ca>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "connect.h"
#include "errcodes.h"
#include "protocol.h"
#include "cli++.h"

const char* cli_help_suffix = "";
const char* cli_args_usage = "remote-address < mail-file";
const int cli_args_min = 1;
const int cli_args_max = 1;
cli_option cli_options[] = {
  { 'p', "port", cli_option::integer, 0, &port,
    "Set the port number on the remote host to connect to", 0 },
  {0}
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

