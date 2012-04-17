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
#include <gnutls/gnutls.h>
#include <gnutls/abstract.h>
#include "fdbuf/tlsibuf.h"
#include "fdbuf/tlsobuf.h"

const char* user = 0;
const char* pass = 0;
int port = 0;
int auth_method = AUTH_DETECT;
int use_ssl = 0;
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
  { 0, "ssl", cli_option::flag, 1, &use_ssl,
    "Connect using SSL (on an alternate port by default)", 0 },
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

static void do_plain(fdibuf& in, int fd)
{
  fdibuf netin(fd);
  fdobuf netout(fd);
  if (!netin || !netout)
    protocol_fail(ERR_MSG_TEMPFAIL, "Error allocating I/O buffers");
  protocol_send(in, netin, netout);
}

static int gnutls_wrap(int ret, const char* msg)
{
  if (ret < 0) {
    mystring m = msg;
    m += ": ";
    m += gnutls_strerror(ret);
    protocol_fail(ERR_MSG_TEMPFAIL, m.c_str());
  }
  return ret;
}

static gnutls_session_t tls_session;

static void tls_init(const char* remote)
{
  gnutls_anon_client_credentials_t anon_cred;
  gnutls_certificate_credentials_t creds;
  gnutls_wrap(gnutls_global_init(),
	      "Error initializing TLS library");
  gnutls_wrap(gnutls_certificate_allocate_credentials(&creds),
	      "Error allocating TLS certificate");
  gnutls_wrap(gnutls_init(&tls_session, GNUTLS_CLIENT),
	      "Error creating TLS session");
  gnutls_wrap(gnutls_priority_set_direct(tls_session, "NORMAL", NULL),
	      "Error setting TLS options");
  gnutls_credentials_set(tls_session, GNUTLS_CRD_CERTIFICATE, creds);
}

static void do_tls(fdibuf& in, int fd)
{
  int r;

  gnutls_transport_set_ptr(tls_session, (gnutls_transport_ptr_t)fd);

  do {
    r = gnutls_handshake(tls_session);
    if (gnutls_error_is_fatal(r))
      gnutls_wrap(r, "Error completing TLS handshake");
  } while (r < 0);

  tlsibuf tlsin(tls_session);
  tlsobuf tlsout(tls_session);
  if (!tlsin || !tlsout)
    protocol_fail(ERR_MSG_TEMPFAIL, "Error allocating I/O buffers");

  protocol_send(in, tlsin, tlsout);
}

int cli_main(int, char* argv[])
{
  const char* remote = argv[0];
  if (port == 0)
    port = use_ssl ? default_ssl_port : default_port;
  if (port < 0)
    protocol_fail(ERR_USAGE, "Invalid value for --port");
  if (use_ssl)
    tls_init(remote);
  fdibuf in(0, true);
  protocol_prep(in);
  int fd = tcpconnect(remote, port);
  if(fd < 0)
    protocol_fail(-fd, "Connect failed");
  if (use_ssl)
    do_tls(in, fd);
  else
    do_plain(in, fd);
  return 0;
}

