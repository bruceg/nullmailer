// Copyright (C) 2018 Bruce Guenter <bruce@untroubled.org>
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

#include "tlsibuf.h"

///////////////////////////////////////////////////////////////////////////////
// Class tlsibuf
///////////////////////////////////////////////////////////////////////////////
tlsibuf::tlsibuf(gnutls_session_t s, unsigned bufsz)
  : fdibuf(-1, false, bufsz), session(s)
{
  flags &= ~flag_closed;
}

ssize_t tlsibuf::_read(char* buf, ssize_t len)
{
  int rc;
  do
  {
    rc = gnutls_record_recv(session, buf, len);
  } while (rc == GNUTLS_E_AGAIN);
  return rc;
}
