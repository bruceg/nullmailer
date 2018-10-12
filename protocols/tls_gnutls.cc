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

#include <config.h>
#include <unistd.h>
#include "errcodes.h"
#include "mystring/mystring.h"
#include "protocol.h"
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include "fdbuf/tlsibuf.h"
#include "fdbuf/tlsobuf.h"

int tls_insecure = false;
int tls_anon_auth = false;
const char* tls_x509certfile = NULL;
const char* tls_x509keyfile = NULL;
const char* tls_x509cafile = NULL;
const char* tls_x509crlfile = NULL;
int tls_x509derfmt = false;

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

static int cert_verify(gnutls_session_t session)
{
  if (tls_x509cafile != NULL && !tls_insecure) {
    // Verify the certificate
    unsigned int status = 0;
    gnutls_wrap(gnutls_certificate_verify_peers2(session, &status),
		"Could not verify SSL/TLS certificate");
    if (status != 0)
      protocol_fail(ERR_MSG_TEMPFAIL, "Server SSL/TLS certificate is untrusted");

    // Verify the hostname
    unsigned int cert_list_size = 0;
    const gnutls_datum_t* cert_list = gnutls_certificate_get_peers(session, &cert_list_size);
    const char* hostname = (const char*)gnutls_session_get_ptr(session);
    gnutls_x509_crt_t crt;
    gnutls_wrap(gnutls_x509_crt_init(&crt),
		"Error allocating memory");
    gnutls_wrap(gnutls_x509_crt_import(crt, &cert_list[0], GNUTLS_X509_FMT_DER),
		"Error decoding SSL/TLS certificate");
    if (gnutls_x509_crt_check_hostname(crt, hostname) == 0)
      protocol_fail(ERR_MSG_TEMPFAIL, "Server SSL/TLS certificate does not match hostname");
    gnutls_x509_crt_deinit(crt);
  }
  // All verification errors cause protocol to exit
  return 0;
}

static gnutls_session_t tls_session;

void tls_cert_auth_init(const char* remote)
{
  gnutls_certificate_credentials_t creds;
  gnutls_wrap(gnutls_global_init(),
	      "Error initializing TLS library");
  gnutls_wrap(gnutls_certificate_allocate_credentials(&creds),
	      "Error allocating TLS certificate");
  gnutls_wrap(gnutls_init(&tls_session, GNUTLS_CLIENT),
	      "Error creating TLS session");
#ifdef HAVE_GNUTLS_PRIORITY_SET_DIRECT
  gnutls_wrap(gnutls_priority_set_direct(tls_session, "NORMAL", NULL),
	      "Error setting TLS options");
#else
  gnutls_wrap(gnutls_set_default_priority(tls_session),
	      "Error setting TLS options");
#endif
  gnutls_wrap(gnutls_credentials_set(tls_session, GNUTLS_CRD_CERTIFICATE, creds),
	      "Error setting TLS credentials");
  gnutls_wrap(gnutls_server_name_set(tls_session, GNUTLS_NAME_DNS, remote, strlen(remote)),
	      "Error setting TLS server name");
  gnutls_session_set_ptr(tls_session, (void*)remote);
#ifdef HAVE_GNUTLS_SET_VERIFY_FUNCTION
  gnutls_certificate_set_verify_function(creds, cert_verify);
#endif
  gnutls_certificate_set_verify_flags(creds, 0);

  gnutls_x509_crt_fmt_t x509fmt = tls_x509derfmt ? GNUTLS_X509_FMT_DER : GNUTLS_X509_FMT_PEM;
  if (tls_x509keyfile == NULL)
    tls_x509keyfile = tls_x509certfile;
  if (tls_x509certfile != NULL)
    gnutls_wrap(gnutls_certificate_set_x509_key_file(creds, tls_x509certfile, tls_x509keyfile, x509fmt),
		"Error setting SSL/TLS X.509 client certificate");
  if (tls_x509cafile == NULL && access(DEFAULT_CA_FILE, R_OK) == 0)
    tls_x509cafile = DEFAULT_CA_FILE;
  if (tls_x509cafile != NULL)
    gnutls_wrap(gnutls_certificate_set_x509_trust_file(creds, tls_x509cafile, x509fmt),
		"Error loading SSL/TLS X.509 trust file");
  if (tls_x509crlfile != NULL)
    gnutls_wrap(gnutls_certificate_set_x509_crl_file(creds, tls_x509crlfile, x509fmt),
		"Error loading SSL/TLS X.509 CRL file");
}

void tls_anon_auth_init(const char* remote)
{
  gnutls_anon_client_credentials_t anon_creds;

  gnutls_wrap(gnutls_global_init(),
	      "Error initializing TLS library");
  gnutls_wrap(gnutls_anon_allocate_client_credentials(&anon_creds),
	      "Error allocating TLS anonymous client credentials");
  gnutls_wrap(gnutls_init(&tls_session, GNUTLS_CLIENT),
	      "Error creating TLS session");

#ifdef HAVE_GNUTLS_PRIORITY_SET_DIRECT
  gnutls_wrap(gnutls_priority_set_direct(tls_session, "NORMAL:+ANON-ECDH:+ANON-DH", NULL),
	      "Error setting TLS options");
#else
  gnutls_wrap(gnutls_set_default_priority(tls_session),
	      "Error setting TLS options");
#endif
  gnutls_wrap(gnutls_credentials_set(tls_session, GNUTLS_CRD_ANON, anon_creds),
	      "Error setting TLS credentials");
  gnutls_wrap(gnutls_server_name_set(tls_session, GNUTLS_NAME_DNS, remote, strlen(remote)),
	      "Error setting TLS server name");
  gnutls_session_set_ptr(tls_session, (void*)remote);
}

void tls_init(const char* remote)
{
  if ((tls_x509certfile || tls_x509keyfile || tls_x509cafile || tls_x509crlfile) && (tls_anon_auth)) {
    protocol_fail(ERR_MSG_TEMPFAIL, "Error: TLS certificate and TLS anonymous auth options cannot both be specified");
  }

  if (tls_anon_auth && tls_insecure) {
    tls_anon_auth_init(remote);
  } else {
    tls_cert_auth_init(remote);
  }
}

void tls_send(fdibuf& in, int fd)
{
  int r;

  gnutls_transport_set_ptr(tls_session, (gnutls_transport_ptr_t)(long)fd);

  do {
    r = gnutls_handshake(tls_session);
    if (gnutls_error_is_fatal(r))
      gnutls_wrap(r, "Error completing TLS handshake");
  } while (r < 0);
#ifndef HAVE_GNUTLS_SET_VERIFY_FUNCTION
  cert_verify(tls_session);
#endif

  tlsibuf tlsin(tls_session);
  tlsobuf tlsout(tls_session);
  if (!tlsin || !tlsout)
    protocol_fail(ERR_MSG_TEMPFAIL, "Error allocating I/O buffers");

  protocol_send(in, tlsin, tlsout);
}
