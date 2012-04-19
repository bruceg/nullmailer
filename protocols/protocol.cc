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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "connect.h"
#include "errcodes.h"
#include "protocol.h"
#include "cli++.h"

const char* user = 0;
const char* pass = 0;
int port = 0;
int auth_method = AUTH_DETECT;
int use_ssl = 0;
int use_starttls = 0;
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
    "Use AUTH LOGIN instead of auto-detecting in SMTP", 0 },
#ifdef HAVE_TLS
  { 0, "ssl", cli_option::flag, 1, &use_ssl,
    "Connect using SSL (on an alternate port by default)", 0 },
  { 0, "starttls", cli_option::flag, 1, &use_starttls,
    "Use STARTTLS command", 0 },
  { 0, "x509cafile", cli_option::string, 0, &tls_x509cafile,
    "Certificate authority trust file", DEFAULT_CA_FILE },
  { 0, "x509crlfile", cli_option::string, 0, &tls_x509crlfile,
    "Certificate revocation list file", 0 },
  { 0, "x509fmtder", cli_option::flag, true, &tls_x509derfmt,
    "X.509 files are in DER format", "PEM format" },
  { 0, "insecure", cli_option::flag, true, &tls_insecure,
    "Don't abort if server certificate fails validation", 0 },
#endif
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

static void plain_send(fdibuf& in, int fd)
{
  fdibuf netin(fd);
  fdobuf netout(fd);
  if (!netin || !netout)
    protocol_fail(ERR_MSG_TEMPFAIL, "Error allocating I/O buffers");
  if (use_starttls) {
    protocol_starttls(netin, netout);
    tls_send(in, fd);
  }
  else
    protocol_send(in, netin, netout);
}

int cli_main(int, char* argv[])
{
  const char* remote = argv[0];
  if (port == 0)
    port = use_ssl ? default_ssl_port : default_port;
  if (port < 0)
    protocol_fail(ERR_USAGE, "Invalid value for --port");
  if (use_ssl || use_starttls)
    tls_init(remote);
  fdibuf in(0, true);
  protocol_prep(in);
  int fd = tcpconnect(remote, port);
  if(fd < 0)
    protocol_fail(-fd, "Connect failed");
  if (use_ssl)
    tls_send(in, fd);
  else
    plain_send(in, fd);
  return 0;
}

