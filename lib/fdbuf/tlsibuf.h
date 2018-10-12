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

#ifndef FDBUF__TLSIBUF__H__
#define FDBUF__TLSIBUF__H__

#include "fdibuf.h"
#include <gnutls/gnutls.h>

class tlsibuf : public fdibuf
{
public:
  tlsibuf(gnutls_session_t, unsigned bufsz = FDBUF_SIZE);
protected:
  gnutls_session_t session;
  virtual ssize_t _read(char* buf, ssize_t len);
};

#endif // FDBUF__TLSIBUF__H__
