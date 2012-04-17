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
#include "errcodes.h"
#include "mystring/mystring.h"
#include "protocol.h"
#include <gnutls/gnutls.h>
#include <gnutls/abstract.h>
#include "fdbuf/tlsibuf.h"
#include "fdbuf/tlsobuf.h"

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

void tls_init(const char* remote)
{
  gnutls_certificate_credentials_t creds;
  gnutls_wrap(gnutls_global_init(),
	      "Error initializing TLS library");
  gnutls_wrap(gnutls_certificate_allocate_credentials(&creds),
	      "Error allocating TLS certificate");
  gnutls_wrap(gnutls_init(&tls_session, GNUTLS_CLIENT),
	      "Error creating TLS session");
  gnutls_wrap(gnutls_priority_set_direct(tls_session, "NORMAL", NULL),
	      "Error setting TLS options");
  gnutls_wrap(gnutls_credentials_set(tls_session, GNUTLS_CRD_CERTIFICATE, creds),
	      "Error setting TLS credentials");
  gnutls_wrap(gnutls_server_name_set(tls_session, GNUTLS_NAME_DNS, remote, strlen(remote)),
	      "Error setting TLS server name");
}

void tls_send(fdibuf& in, int fd)
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
